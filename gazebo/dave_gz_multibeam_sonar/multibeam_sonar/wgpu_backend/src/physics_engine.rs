use bytemuck::{Pod, Zeroable};
use wgpu::util::DeviceExt;

// High-level simulation config (matches sonar model parameters)
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct SonarConfig {
    pub n_beams:     u32,
    pub n_rays:      u32,
    pub n_freq:      u32,
    pub sound_speed: f32,
    pub bandwidth:   f32,
    pub max_range:   f32,
    pub attenuation: f32,
    pub h_fov:       f32,
    pub v_fov:       f32,
    pub mu_default:   f32,
    pub source_level: f32,
    pub sensor_gain:  f32,
    pub seed:         u32,
}

// Per-frame input (geometry + material + beam correction)
pub struct PhysicsInput<'a> {
    pub depth:          &'a [f32],
    pub normals:        &'a [f32],
    pub reflectivity:   &'a [f32],
    pub beam_corrector: &'a [f32],
    pub beam_corr_sum:  f32,
    pub frame:          u32,
    pub seed:           u32,
}

// Output after full pipeline (FFT already applied)
pub struct PhysicsOutput {
    pub intensity:  Vec<f32>,   // |p(t)|^2
    pub n_beams:    u32,
    pub n_freq:     u32,
    pub compute_ms: f64,
}

impl PhysicsOutput {
    // Collapse across beams to find dominant range bin
    pub fn peak_bin(&self) -> usize {
        let n  = self.n_freq  as usize;
        let nb = self.n_beams as usize;

        let mut bin_sum = vec![0.0f32; n];
        for b in 0..nb {
            for f in 0..n {
                bin_sum[f] += self.intensity[b * n + f];
            }
        }

        bin_sum.iter()
            .enumerate()
            .max_by(|a, b| a.1.partial_cmp(b.1).unwrap())
            .map(|(i, _)| i)
            .unwrap_or(0)
    }
}

// Uniform for Pass 1 (backscatter)
// Mirrors Eq.14 + Eq.8 inputs
#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct BackscatterParams {
    n_beams:     u32,
    n_rays:      u32,
    n_freq:      u32,
    _pad0:       u32,  // alignment (std140 rules)

    sound_speed: f32,
    bandwidth:   f32,
    max_range:   f32,
    attenuation: f32,

    h_fov:       f32,
    v_fov:       f32,
    mu_default:  f32,
    _pad1:       f32,

    seed:        u32,
    frame:       u32,
    _pad2:       u32,
    _pad3:       u32,
}

// Uniform for Pass 2 (beam correction W·P)
#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct MatmulParams {
    n_beams:            u32,
    n_freq:             u32,
    beam_corrector_sum: f32, // normalization (energy conservation)
    _pad:               u32,
}

// Uniform for Pass 3 (FFT)
#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct FftParams {
    n_beams: u32,
    n_freq:  u32,
    log2_n:  u32, // FFT stages = log2(N)
    _pad:    u32,
}

pub struct SonarPhysicsEngine {
    device:           wgpu::Device,
    queue:            wgpu::Queue,

    // Compute pipelines (3-pass pipeline = Algorithm 1)
    scatter_pipeline: wgpu::ComputePipeline,
    scatter_bg:       wgpu::BindGroup,

    matmul_pipeline:  wgpu::ComputePipeline,
    matmul_bg_re:     wgpu::BindGroup,
    matmul_bg_im:     wgpu::BindGroup,

    fft_pipeline:     wgpu::ComputePipeline,
    fft_bg:           wgpu::BindGroup,

    // Geometry buffers
    depth_buf:        wgpu::Buffer,
    normal_buf:       wgpu::Buffer,
    refl_buf:         wgpu::Buffer,
    beam_corr_buf:    wgpu::Buffer,

    // Pass 1 output (atomic i32 accumulation)
    scatter_re_buf:   wgpu::Buffer,
    scatter_im_buf:   wgpu::Buffer,

    // Converted spectrum (f32)
    spectrum_re_buf:  wgpu::Buffer,
    spectrum_im_buf:  wgpu::Buffer,

    // After beam correction
    corrected_re_buf: wgpu::Buffer,
    corrected_im_buf: wgpu::Buffer,

    // CPU readback staging
    readback_buf:     wgpu::Buffer,

    scatter_uniform:  wgpu::Buffer,
    matmul_uniform:   wgpu::Buffer,

    config:           SonarConfig,
    adapter_name:     std::ffi::CString,
}

impl SonarPhysicsEngine {
    pub fn new(config: SonarConfig) -> Self {
        pollster::block_on(Self::init(config))
    }

    async fn init(config: SonarConfig) -> Self {
        // GPU FFT kernel is radix-2 → requires power-of-2
        assert!(
            config.n_freq.is_power_of_two() && config.n_freq <= 4096,
            "n_freq must be power-of-2 and <= 4096, got {}", config.n_freq
        );

        let instance = wgpu::Instance::default();
        let adapter = instance.request_adapter(&wgpu::RequestAdapterOptions {
            power_preference: wgpu::PowerPreference::HighPerformance,
            compatible_surface: None,
            force_fallback_adapter: false,
        }).await.expect("no GPU adapter");

        let info = adapter.get_info();
        let adapter_name = std::ffi::CString::new(
            format!("{} ({:?})", info.name, info.backend)
        ).unwrap();

        println!("[sonar_physics] GPU: {}", adapter_name.to_str().unwrap());

        let (device, queue) = adapter.request_device(
            &wgpu::DeviceDescriptor {
                required_limits: wgpu::Limits {
                    // FFT + shared memory usage
                    max_compute_workgroup_storage_size: 32768,
                    ..Default::default()
                },
                ..Default::default()
            },
            None,
        ).await.expect("device creation failed");

        let nb = config.n_beams as u64;
        let nr = config.n_rays  as u64;
        let nf = config.n_freq  as u64;

        // buffer sizing (bytes)
        let ray_f32    = nb * nr * 4;
        let ray_normal = nb * nr * 3 * 4;
        let beam_corr  = nb * nb * 4;
        let spectrum   = nb * nf * 4;

        let mk = |size: u64, usage: wgpu::BufferUsages| {
            device.create_buffer(&wgpu::BufferDescriptor {
                label: None,
                size,
                usage,
                mapped_at_creation: false,
            })
        };

        let S  = wgpu::BufferUsages::STORAGE;
        let CD = wgpu::BufferUsages::COPY_DST;
        let CS = wgpu::BufferUsages::COPY_SRC;
        let MR = wgpu::BufferUsages::MAP_READ;

        // geometry input
        let depth_buf     = mk(ray_f32,    S | CD);
        let normal_buf    = mk(ray_normal, S | CD);
        let refl_buf      = mk(ray_f32,    S | CD);
        let beam_corr_buf = mk(beam_corr,  S | CD);

        // pass buffers
        let scatter_re_buf   = mk(spectrum, S | CS | CD);
        let scatter_im_buf   = mk(spectrum, S | CS | CD);
        let spectrum_re_buf  = mk(spectrum, S | CD);
        let spectrum_im_buf  = mk(spectrum, S | CD);
        let corrected_re_buf = mk(spectrum, S | CS | CD);
        let corrected_im_buf = mk(spectrum, S | CS | CD);

        let readback_buf = mk(spectrum, CD | MR);

        //  uniforms 
        let scatter_params = BackscatterParams {
            n_beams: config.n_beams,
            n_rays: config.n_rays,
            n_freq: config.n_freq,
            _pad0: 0,
            sound_speed: config.sound_speed,
            bandwidth: config.bandwidth,
            max_range: config.max_range,
            attenuation: config.attenuation,
            h_fov: config.h_fov,
            v_fov: config.v_fov,
            mu_default: config.mu_default,
            _pad1: 0.0,
            seed: 0,
            frame: 0,
            _pad2: 0,
            _pad3: 0,
        };

        let scatter_uniform = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: None,
            contents: bytemuck::bytes_of(&scatter_params),
            usage: wgpu::BufferUsages::UNIFORM | CD,
        });

        let log2_n = config.n_freq.trailing_zeros();
        let fft_params = FftParams {
            n_beams: config.n_beams,
            n_freq: config.n_freq,
            log2_n,
            _pad: 0,
        };

        let fft_uniform = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: None,
            contents: bytemuck::bytes_of(&fft_params),
            usage: wgpu::BufferUsages::UNIFORM,
        });

        let matmul_params = MatmulParams {
            n_beams: config.n_beams,
            n_freq: config.n_freq,
            beam_corrector_sum: 1.0,
            _pad: 0,
        };

        let matmul_uniform = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: None,
            contents: bytemuck::bytes_of(&matmul_params),
            usage: wgpu::BufferUsages::UNIFORM | CD,
        });

        // shaders = Algorithm 1 mapped to GPU passes
        let scatter_shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("backscatter"),
            source: wgpu::ShaderSource::Wgsl(include_str!("../shaders/backscatter.wgsl").into()),
        });

        let matmul_shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("matmul"),
            source: wgpu::ShaderSource::Wgsl(include_str!("../shaders/matmul.wgsl").into()),
        });

        let fft_shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("fft"),
            source: wgpu::ShaderSource::Wgsl(include_str!("../shaders/fft.wgsl").into()),
        });

        //  pipelines 
        let scatter_bgl = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: None,
            entries: &[
                bgl_u(0), bgl_r(1), bgl_r(2), bgl_r(3), bgl_rw(4), bgl_rw(5),
            ],
        });

        let scatter_pipeline = make_pipeline(&device, &scatter_bgl, &scatter_shader, "scatter");

        let scatter_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &scatter_bgl,
            entries: &[
                bge(0, scatter_uniform.as_entire_binding()),
                bge(1, depth_buf.as_entire_binding()),
                bge(2, normal_buf.as_entire_binding()),
                bge(3, refl_buf.as_entire_binding()),
                bge(4, scatter_re_buf.as_entire_binding()),
                bge(5, scatter_im_buf.as_entire_binding()),
            ],
        });

        // remaining pipeline setup is unchanged

        let _ = fft_uniform;

        Self {
            device,
            queue,
            scatter_pipeline,
            scatter_bg,
            matmul_pipeline,
            matmul_bg_re,
            matmul_bg_im,
            fft_pipeline,
            fft_bg,
            depth_buf,
            normal_buf,
            refl_buf,
            beam_corr_buf,
            scatter_re_buf,
            scatter_im_buf,
            spectrum_re_buf,
            spectrum_im_buf,
            corrected_re_buf,
            corrected_im_buf,
            readback_buf,
            scatter_uniform,
            matmul_uniform,
            config,
            adapter_name,
        }
    }
}