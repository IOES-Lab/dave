/// Adapters, device, queue setups + BUFFERS SETUP + PIPELINE SETUPS + PIPELINE COMPILE + GPU CONTEXT INIT all in pipeline.rs
/// + Shaders compiled

use std::sync::OnceLock;

/// GPU buffer lifetime: persistent allocation across frames via queue.write_buffer.
/// Buffers reallocated only when sonar dimensions change.
pub struct SonarBuffers {
    pub n_beams: u32,
    pub n_rays: u32,
    pub n_freq: u32,
    // Input data (written via queue.write_buffer each frame)
    pub depth_buf: wgpu::Buffer,
    pub normal_buf: wgpu::Buffer,
    pub refl_buf: wgpu::Buffer,
    pub window_buf: wgpu::Buffer,
    pub bc_buf: wgpu::Buffer,
    // Backscatter atomic i32 accumulators (zeroed via write_buffer each frame)
    pub out_re_i32: wgpu::Buffer,
    pub out_im_i32: wgpu::Buffer,
    // Convert output / matmul input (f32)
    pub mm_re_in: wgpu::Buffer,
    pub mm_im_in: wgpu::Buffer,
    // Matmul output / FFT input
    pub mm_re_out: wgpu::Buffer,
    pub mm_im_out: wgpu::Buffer,
    // FFT in-place buffers
    pub p_re_buf: wgpu::Buffer,
    pub p_im_buf: wgpu::Buffer,
    // Host-visible staging buffers for final readback
    pub stg_re: wgpu::Buffer,
    pub stg_im: wgpu::Buffer,
}

impl SonarBuffers {
    /// Buffer allocation for 4 pipeline stages: backscatter, convert, matmul, FFT.
    /// Chain: backscatter→convert→matmul→FFT→readback.
    pub fn new(device: &wgpu::Device, n_beams: u32, n_rays: u32, n_freq: u32) -> Self {
        let depth_ray = (n_beams as usize) * (n_rays as usize);
        let spectrum = (n_beams as usize) * (n_freq as usize);
        let bc = (n_beams as usize) * (n_beams as usize);
        let f32_bytes = |n: usize| (n * std::mem::size_of::<f32>()) as u64;
        let i32_bytes = |n: usize| (n * std::mem::size_of::<i32>()) as u64;

        SonarBuffers {
            n_beams, n_rays, n_freq,
            depth_buf: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_depth"),
                size: f32_bytes(depth_ray),
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            normal_buf: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_normal"),
                size: f32_bytes(depth_ray * 3),
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            refl_buf: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_refl"),
                size: f32_bytes(depth_ray),
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            window_buf: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_window"),
                size: f32_bytes(n_freq as usize),
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            bc_buf: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_bc"),
                size: f32_bytes(bc),
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            out_re_i32: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_re_i32"),
                size: i32_bytes(spectrum),
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            out_im_i32: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_im_i32"),
                size: i32_bytes(spectrum),
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            mm_re_in: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_mm_re_in"),
                size: f32_bytes(spectrum),
                usage: wgpu::BufferUsages::STORAGE,
                mapped_at_creation: false,
            }),
            mm_im_in: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_mm_im_in"),
                size: f32_bytes(spectrum),
                usage: wgpu::BufferUsages::STORAGE,
                mapped_at_creation: false,
            }),
            mm_re_out: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_mm_re_out"),
                size: f32_bytes(spectrum),
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC,
                mapped_at_creation: false,
            }),
            mm_im_out: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_mm_im_out"),
                size: f32_bytes(spectrum),
                usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC,
                mapped_at_creation: false,
            }),
            p_re_buf: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_p_re"),
                size: f32_bytes(spectrum),
                usage: wgpu::BufferUsages::STORAGE
                    | wgpu::BufferUsages::COPY_SRC
                    | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            p_im_buf: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_p_im"),
                size: f32_bytes(spectrum),
                usage: wgpu::BufferUsages::STORAGE
                    | wgpu::BufferUsages::COPY_SRC
                    | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            stg_re: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_stg_re"),
                size: f32_bytes(spectrum),
                usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
            stg_im: device.create_buffer(&wgpu::BufferDescriptor {
                label: Some("pb_stg_im"),
                size: f32_bytes(spectrum),
                usage: wgpu::BufferUsages::MAP_READ | wgpu::BufferUsages::COPY_DST,
                mapped_at_creation: false,
            }),
        }
    }
}

/// GPU compute context: device handle + compiled shader pipelines.
/// Four pipelines compiled once at init: backscatter, convert, matmul, FFT.
pub struct GpuContext {
    pub device: wgpu::Device,   // the GPU -> used to create everything
    pub queue: wgpu::Queue,     // the command queue -> used to submit work

    // Backscatter kernel: ray-surface interactions with Lambert scatter.
    pub bs_pipeline: wgpu::ComputePipeline,         // compiled backscatter shader
    pub bs_bgl: wgpu::BindGroupLayout,              // describes bs shader's buffer slots

    // i32→f32 conversion: fixed-point atomic backscatter output to float spectrum.
    pub convert_pipeline: wgpu::ComputePipeline,
    pub convert_bgl: wgpu::BindGroupLayout,

    // Beam correction GEMM: tiled matrix multiply for spectrum.
    pub mm_pipeline: wgpu::ComputePipeline,
    pub mm_bgl: wgpu::BindGroupLayout,

    // FFT kernel: power-of-2 only, N≤4096 (4096 is the shared memory limit). CPU fallback via Bluestein for arbitrary N.
    pub fft_pipeline: wgpu::ComputePipeline,
    pub fft_bgl: wgpu::BindGroupLayout,

    // SonarBuffers are None until first compute call when dimensions are known.
    // Reallocated automatically if dimensions change between frames.
    // Persistent GPU buffers across frames: CUDA cudaMalloc equivalent (allocated once, reused across ≥1 frames with same dimensions).
    pub buffers: std::sync::Mutex<Option<SonarBuffers>>,
}

static GPU_CONTEXT: OnceLock<Option<GpuContext>> = OnceLock::new();

/// Bind group layout entry: storage buffer descriptor.
fn make_storage_entry(binding: u32, read_only: bool) -> wgpu::BindGroupLayoutEntry {
    wgpu::BindGroupLayoutEntry {
        binding,
        visibility: wgpu::ShaderStages::COMPUTE,
        ty: wgpu::BindingType::Buffer {
            ty: wgpu::BufferBindingType::Storage { read_only },
            has_dynamic_offset: false,
            min_binding_size: None,
        },
        count: None,
    }
}

/// Uniform entry: shader parameter buffer.
fn make_uniform_entry(binding: u32) -> wgpu::BindGroupLayoutEntry {
    wgpu::BindGroupLayoutEntry {
        binding,
        visibility: wgpu::ShaderStages::COMPUTE,
        ty: wgpu::BindingType::Buffer {
            ty: wgpu::BufferBindingType::Uniform,
            has_dynamic_offset: false,
            min_binding_size: None,
        },
        count: None,
    }
}

/// Bind group layout builder: defines buffer bindings for each shader.
fn build_bgls(device: &wgpu::Device) -> (
    wgpu::BindGroupLayout,
    wgpu::BindGroupLayout,
    wgpu::BindGroupLayout,
    wgpu::BindGroupLayout,
) {
    // Backscatter kernel params: uniform params, depth, normals, reflectivity, output accumulators, window.
    let bs_bgl = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("bs_bgl"),
        entries: &[
            make_uniform_entry(0),          // (params)
            make_storage_entry(1, true),    // (depth)          true = read_only
            make_storage_entry(2, true),    // (normal)
            make_storage_entry(3, true),    // (reflectivity)
            make_storage_entry(4, false),   // (out_re_i32)     false = read_write
            make_storage_entry(5, false),   // (out_im_i32)
            make_storage_entry(6, true),    // (window)
        ],
    });

    // Matmul (GEMM) kernel params: uniform params, beam_corrector, spectrum_in, spectrum_out.
    let mm_bgl = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("mm_bgl"),
        entries: &[
            make_uniform_entry(0),
            make_storage_entry(1, true),
            make_storage_entry(2, true),
            make_storage_entry(3, false),
        ],
    });

    // FFT kernel params: uniform params, spectrum_re, spectrum_im (in-place).
    let fft_bgl = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("fft_bgl"),
        entries: &[
            make_uniform_entry(0),
            make_storage_entry(1, false),
            make_storage_entry(2, false),
        ],
    });

    // Convert kernel params: int32 input (re/im), float32 output (re/im).
    let convert_bgl = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("convert_bgl"),
        entries: &[
            make_storage_entry(0, true),
            make_storage_entry(1, true),
            make_storage_entry(2, false),
            make_storage_entry(3, false),
        ],
    });

    (bs_bgl, mm_bgl, fft_bgl, convert_bgl)
}

/// Pipeline compilation: shader module compilation + pipeline layout binding (one-time init).
fn compile_pipeline(
    device: &wgpu::Device,
    label: &str,
    wgsl: &str,
    bgl: &wgpu::BindGroupLayout,
) -> wgpu::ComputePipeline {
    let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
        label: Some(label),
        source: wgpu::ShaderSource::Wgsl(wgsl.into()),
    });
    let layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
        label: Some(label),
        bind_group_layouts: &[bgl],
        push_constant_ranges: &[],
    });
    device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
        label: Some(label),
        layout: Some(&layout),
        module: &shader,
        entry_point: "main",
        compilation_options: wgpu::PipelineCompilationOptions::default(),
    })
}

/// GPU context initialization: Vulkan adapter enumeration, device request.
fn init_gpu_context() -> Option<GpuContext> {
    // TODO: These environment variable overrides are NVIDIA/Vulkan-specific (Linux only).
    // Replace with wgpu::Backends::all() so the adapter selection works automatically
    // on any platform (Metal on macOS, DX12 on Windows, Vulkan on Linux) without
    // needing manual ICD path configuration.
    if std::env::var("VK_ICD_FILENAMES").is_err() {
        unsafe {
            std::env::set_var("VK_ICD_FILENAMES", "/usr/share/vulkan/icd.d/nvidia_icd.json");
        }
    }
    if std::env::var("VK_LAYER_PATH").is_err() {
        unsafe {
            std::env::set_var("DISABLE_LAYER_NV_optimus", "1");
        }
    }

    // Catch panic to prevent GPU init failure from crashing Gazebo.
    let result = std::panic::catch_unwind(|| {
        // TODO: Replace wgpu::Backends::VULKAN with wgpu::Backends::all() to support
        // Metal (macOS), DX12 (Windows), and other backends automatically.
        // The explicit VULKAN flag means this will fail silently on non-Vulkan platforms.
        let instance = wgpu::Instance::new(wgpu::InstanceDescriptor {
            backends: wgpu::Backends::VULKAN,
            ..Default::default()
        });

        // TODO: Same as above — enumerate_adapters(wgpu::Backends::all()) would pick up
        // the best available backend on any platform instead of Vulkan-only.
        let adapters: Vec<_> = instance
            .enumerate_adapters(wgpu::Backends::VULKAN)
            .into_iter()
            .collect();

        eprintln!("[sonar_wgpu] Vulkan adapters found: {}", adapters.len());
        for a in &adapters {
            let info = a.get_info();
            eprintln!("[sonar_wgpu]   {:?} -> {}", info.device_type, info.name);
        }

        let adapter = adapters
            .iter()
            .find(|a| a.get_info().device_type == wgpu::DeviceType::DiscreteGpu)
            .or_else(|| adapters.first())?;

        eprintln!("[sonar_wgpu] Selected adapter: {}", adapter.get_info().name);
        let (device, queue) = pollster::block_on(adapter.request_device(
            &wgpu::DeviceDescriptor {
                label: Some("sonar_wgpu_device"),
                required_features: wgpu::Features::empty(),
                required_limits: wgpu::Limits::default(),
            },
            None,
        ))
        .map_err(|e| {
            eprintln!("[sonar_wgpu] request_device failed: {e}");
            e
        })
        .ok()?;

        // One-time shader compilation.
        let t0 = std::time::Instant::now();
        let (bs_bgl, mm_bgl, fft_bgl, convert_bgl) = build_bgls(&device);
        let bs_pipeline = compile_pipeline(
            &device, "bs_pipeline",
            include_str!("shaders/backscatter.wgsl"), &bs_bgl,
        );
        let convert_pipeline = compile_pipeline(
            &device, "convert_pipeline",
            include_str!("shaders/convert.wgsl"), &convert_bgl,
        );
        let mm_pipeline = compile_pipeline(
            &device, "mm_pipeline",
            include_str!("shaders/matmul.wgsl"), &mm_bgl,
        );
        let fft_pipeline = compile_pipeline(
            &device, "fft_pipeline",
            include_str!("shaders/fft.wgsl"), &fft_bgl,
        );
        eprintln!("[sonar_wgpu] GPU pipelines compiled in {:.0} ms -> ready.", t0.elapsed().as_millis());

        Some(GpuContext { device, queue, bs_pipeline, bs_bgl, convert_pipeline, convert_bgl, mm_pipeline, mm_bgl, fft_pipeline, fft_bgl, buffers: std::sync::Mutex::new(None) })
    });

    match result {
        Ok(ctx) => ctx,
        Err(_) => {
            eprintln!("[sonar_wgpu] GPU init panicked; falling back to CPU path");
            None
        }
    }
}

/// Global singleton: lazily initializes GPU context on first access.
/// Returns None if GPU unavailable; falls back to CPU implementation.
pub fn get_or_init() -> Option<&'static GpuContext> {
    GPU_CONTEXT.get_or_init(init_gpu_context).as_ref()
}


/*
Data flow with buffer names:

    CPU INPUT DATA
        │
        ▼
    depth_buf    [n_beams × n_rays]      f32   COPY_DST | STORAGE
    normal_buf   [n_beams × n_rays × 3]  f32   COPY_DST | STORAGE
    refl_buf     [n_beams × n_rays]      f32   COPY_DST | STORAGE
    window_buf   [n_freq]                f32   COPY_DST | STORAGE
    bc_buf       [n_beams × n_beams]     f32   COPY_DST | STORAGE
        │
        ▼ backscatter.wgsl (reads above, writes below)
        │
    out_re_i32   [n_beams × n_freq]      i32   COPY_DST | STORAGE  ← atomic accumulators
    out_im_i32   [n_beams × n_freq]      i32   COPY_DST | STORAGE  ← zeroed each frame
        │
        ▼ convert.wgsl (i32 → f32)
        │
    mm_re_in     [n_beams × n_freq]      f32   STORAGE
    mm_im_in     [n_beams × n_freq]      f32   STORAGE
        │
        ▼ matmul.wgsl (beam correction)
        │
    mm_re_out    [n_beams × n_freq]      f32   STORAGE | COPY_SRC
    mm_im_out    [n_beams × n_freq]      f32   STORAGE | COPY_SRC
        │
        ▼ copied into FFT buffers
        │
    p_re_buf     [n_beams × n_freq]      f32   STORAGE | COPY_SRC | COPY_DST
    p_im_buf     [n_beams × n_freq]      f32   STORAGE | COPY_SRC | COPY_DST
        │
        ▼ fft.wgsl (in-place FFT)
        │
        ▼ copied to staging
        │
    stg_re       [n_beams × n_freq]      f32   MAP_READ | COPY_DST
    stg_im       [n_beams × n_freq]      f32   MAP_READ | COPY_DST
        │
        ▼ CPU reads back final result
    */