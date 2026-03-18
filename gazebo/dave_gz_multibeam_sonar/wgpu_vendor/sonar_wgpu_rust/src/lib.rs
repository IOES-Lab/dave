/// C-FFI interface, Buffer initialization and write ups and reads + Bind groups created + encoder setup + memory free
/// + Compute passes for each shader + gpu/cpu path selection

mod fft;          // CPU FFT + Bluestein
mod pipeline;     // GPU context + buffer management

use bytemuck::{Pod, Zeroable};

#[derive(Clone, Copy, Default)]
pub struct ComplexF {
    pub re: f32,
    pub im: f32,
}
use std::sync::atomic::{AtomicU64, Ordering};
use wgpu::util::DeviceExt;

fn is_power_of_two(x: usize) -> bool {
    x != 0 && (x & (x - 1)) == 0
}

#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct BackscatterParams {
    n_beams: u32,
    n_rays: u32,
    n_freq: u32,
    ray_skips: u32,
    sound_speed: f32,
    max_distance: f32,
    source_level: f32,
    attenuation: f32,
    sensor_gain: f32,
    bandwidth: f32,
    seed_lo: u32,
    seed_hi: u32,
    frame_lo: u32,          // WGSL has no native u64 type.
    frame_hi: u32,          // Split frame index into two u32 - frame_lo and frame_hi
    area_scaler: f32,
    h_fov: f32,
    v_fov: f32,
    _pad0: u32,
    _pad1: u32,
    _pad2: u32,
}

#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct MatmulParams {
    n_beams: u32,
    n_freq: u32,
    beam_corrector_sum: f32,
}

#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct FftParams {
    n_beams: u32,
    n_freq: u32,
    log2_n: u32,
}

/// The function receives flat arrays from C++ and returns a heap-allocated array.
#[no_mangle]
pub extern "C" fn sonar_wgpu_compute(
    depth_flat: *const f32,
    normal_flat: *const f32,
    reflectivity_flat: *const f32,
    beam_corrector_flat: *const f32,
    window_flat: *const f32,
    n_beams: u32,
    n_rays: u32,
    n_freq: u32,
    ray_skips: u32,
    sound_speed: f32,
    max_distance: f32,
    source_level: f32,
    attenuation: f32,
    sensor_gain: f32,
    bandwidth: f32,
    beam_corrector_sum: f32,
    area_scaler: f32,
    h_fov: f32,
    v_fov: f32,
    frame_index: u64,
    seed: u64,
) -> *mut f32 {
    if depth_flat.is_null()
        || normal_flat.is_null()
        || reflectivity_flat.is_null()
        || beam_corrector_flat.is_null()
        || window_flat.is_null()
    {
        return std::ptr::null_mut();
    }

    let n_beams_us = n_beams as usize;
    let n_rays_us = n_rays as usize;
    let n_freq_us = n_freq as usize;

    if n_beams_us == 0 || n_rays_us == 0 || n_freq_us == 0 {
        return std::ptr::null_mut();
    }

    use std::sync::Once;

    let depth_len = n_beams_us * n_rays_us;
    let normal_len = depth_len * 3;
    let refl_len = depth_len;
    let bc_len = n_beams_us * n_beams_us;
    let spectrum_len = n_beams_us * n_freq_us;

    let depth = unsafe { std::slice::from_raw_parts(depth_flat, depth_len) };
    let normals = unsafe { std::slice::from_raw_parts(normal_flat, normal_len) };
    let reflectivity = unsafe { std::slice::from_raw_parts(reflectivity_flat, refl_len) };
    let beam_corrector = unsafe { std::slice::from_raw_parts(beam_corrector_flat, bc_len) };

    let ctx = match pipeline::get_or_init() {
        Some(c) => {
            static GPU_LOG: Once = Once::new();
            GPU_LOG.call_once(|| eprintln!("[sonar_wgpu] GPU device acquired -> running on GPU."));
            c
        }
        None => {
            static FALLBACK_LOG: Once = Once::new();
            FALLBACK_LOG.call_once(|| eprintln!("[sonar_wgpu] GPU init failed -> returning null (C++ will use CPU backend)."));
            return std::ptr::null_mut();
        }
    };
    let device = &ctx.device;
    let queue = &ctx.queue;

    static GPU_FRAME_COUNT: AtomicU64 = AtomicU64::new(0);
    let gpu_frame = GPU_FRAME_COUNT.fetch_add(1, Ordering::Relaxed);
    let gpu_t0 = std::time::Instant::now();

    let window_vals = unsafe { std::slice::from_raw_parts(window_flat, n_freq_us) };

    // ---- Per-frame uniform buffers (tiny, always recreated) ----
    let backscatter_params = BackscatterParams {
        n_beams,
        n_rays,
        n_freq,
        ray_skips,
        sound_speed,
        max_distance,
        source_level,
        attenuation,
        sensor_gain,
        bandwidth,
        seed_lo: (seed & 0xFFFF_FFFF) as u32,
        seed_hi: (seed >> 32) as u32,
        frame_lo: (frame_index & 0xFFFF_FFFF) as u32,
        frame_hi: (frame_index >> 32) as u32,
        area_scaler,
        h_fov,
        v_fov,
        _pad0: 0,
        _pad1: 0,
        _pad2: 0,
    };
    let bs_param_buf = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: Some("bs_param"),
        contents: bytemuck::bytes_of(&backscatter_params),
        usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
    });
    let mm_params = MatmulParams { n_beams, n_freq, beam_corrector_sum };
    let mm_param_buf = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: Some("mm_param"),
        contents: bytemuck::bytes_of(&mm_params),
        usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
    });

    // REMOVE THIS, UNNCESSARY
    // ---- Persistent data buffers: allocate once per dimension set, reuse every frame ----
    let mut buf_guard = ctx.buffers.lock().unwrap();
    {
        let needs_alloc = buf_guard.as_ref().map_or(true, |b| {
            b.n_beams != n_beams || b.n_rays != n_rays || b.n_freq != n_freq
        });
        if needs_alloc {
            *buf_guard = Some(pipeline::SonarBuffers::new(device, n_beams, n_rays, n_freq));
            eprintln!("[sonar_wgpu] Persistent GPU buffers allocated for {}×{}×{}",
                n_beams, n_rays, n_freq);
        }
    }
    let buf = buf_guard.as_ref().unwrap();

    // Upload frame data into persistent buffers (DMA, no GPU alloc)
    queue.write_buffer(&buf.depth_buf,  0, bytemuck::cast_slice(depth));
    queue.write_buffer(&buf.normal_buf, 0, bytemuck::cast_slice(normals));
    queue.write_buffer(&buf.refl_buf,   0, bytemuck::cast_slice(reflectivity));
    queue.write_buffer(&buf.window_buf, 0, bytemuck::cast_slice(window_vals));
    queue.write_buffer(&buf.bc_buf,     0, bytemuck::cast_slice(beam_corrector));
    // Zero atomic accumulators (reused each frame)
    let zeros_i32 = vec![0i32; spectrum_len];
    queue.write_buffer(&buf.out_re_i32, 0, bytemuck::cast_slice(&zeros_i32));
    queue.write_buffer(&buf.out_im_i32, 0, bytemuck::cast_slice(&zeros_i32));

    // ---- Bind groups (metadata only -> no GPU allocation) ----
    let bs_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("bs_bg"),
        layout: &ctx.bs_bgl,
        entries: &[
            wgpu::BindGroupEntry { binding: 0, resource: bs_param_buf.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 1, resource: buf.depth_buf.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 2, resource: buf.normal_buf.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 3, resource: buf.refl_buf.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 4, resource: buf.out_re_i32.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 5, resource: buf.out_im_i32.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 6, resource: buf.window_buf.as_entire_binding() },
        ],
    });
    let conv_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("conv_bg"),
        layout: &ctx.convert_bgl,
        entries: &[
            wgpu::BindGroupEntry { binding: 0, resource: buf.out_re_i32.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 1, resource: buf.out_im_i32.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 2, resource: buf.mm_re_in.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 3, resource: buf.mm_im_in.as_entire_binding() },
        ],
    });
    let mm_re_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("mm_re_bg"),
        layout: &ctx.mm_bgl,
        entries: &[
            wgpu::BindGroupEntry { binding: 0, resource: mm_param_buf.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 1, resource: buf.bc_buf.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 2, resource: buf.mm_re_in.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 3, resource: buf.mm_re_out.as_entire_binding() },
        ],
    });
    let mm_im_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("mm_im_bg"),
        layout: &ctx.mm_bgl,
        entries: &[
            wgpu::BindGroupEntry { binding: 0, resource: mm_param_buf.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 1, resource: buf.bc_buf.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 2, resource: buf.mm_im_in.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 3, resource: buf.mm_im_out.as_entire_binding() },
        ],
    });

    let spectrum_bytes = (spectrum_len * std::mem::size_of::<f32>()) as u64;
    let gpu_fft = n_freq_us <= 4096 && is_power_of_two(n_freq_us);

    // ---- Single command encoder: all passes + staging copy in one GPU submission ----
    let mut enc = device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
        label: Some("sonar_enc"),
    });

    // Backscatter dispatch: depth/normals/refl → i32 fixed-point spectrum
    {
        let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
            label: Some("bs_pass"),
            timestamp_writes: None,
        });
        pass.set_pipeline(&ctx.bs_pipeline);
        pass.set_bind_group(0, &bs_bg, &[]);
        let gx = (n_beams + 7) / 8;
        let ray_step = std::cmp::max(1, ray_skips);
        let reduced_rays = (n_rays + ray_step - 1) / ray_step;
        let gy = (reduced_rays + 7) / 8;
        pass.dispatch_workgroups(gx, gy, 1);
    }

    // Convert dispatch: i32 fixed-point → f32 (GPU-side, eliminates CPU readback round-trip)
    {
        let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
            label: Some("conv_pass"),
            timestamp_writes: None,
        });
        pass.set_pipeline(&ctx.convert_pipeline);
        pass.set_bind_group(0, &conv_bg, &[]);
        let gx = (spectrum_len as u32 + 63) / 64;
        pass.dispatch_workgroups(gx, 1, 1);
    }

    // Matmul dispatch: beam-corrector matrix multiply, re and im in same encoder
    {
        let gx = (n_freq + 15) / 16;
        let gy = (n_beams + 15) / 16;
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("mm_re_pass"),
                timestamp_writes: None,
            });
            pass.set_pipeline(&ctx.mm_pipeline);
            pass.set_bind_group(0, &mm_re_bg, &[]);
            pass.dispatch_workgroups(gx, gy, 1);
        }
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("mm_im_pass"),
                timestamp_writes: None,
            });
            pass.set_pipeline(&ctx.mm_pipeline);
            pass.set_bind_group(0, &mm_im_bg, &[]);
            pass.dispatch_workgroups(gx, gy, 1);
        }
    }

    // FFT Dispatch (optional, only if n_freq is power-of-two and <= 4096); re/im in same encoder
    if gpu_fft {
        // copy MM output → FFT buffers → dispatch fft shader → copy to staging
        enc.copy_buffer_to_buffer(&buf.mm_re_out, 0, &buf.p_re_buf, 0, spectrum_bytes);
        enc.copy_buffer_to_buffer(&buf.mm_im_out, 0, &buf.p_im_buf, 0, spectrum_bytes);

        let fft_params = FftParams { n_beams, n_freq, log2_n: n_freq_us.ilog2() };
        let fft_param_buf = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("fft_param"),
            contents: bytemuck::bytes_of(&fft_params),
            usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
        });
        let fft_bg = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: Some("fft_bg"),
            layout: &ctx.fft_bgl,
            entries: &[
                wgpu::BindGroupEntry { binding: 0, resource: fft_param_buf.as_entire_binding() },
                wgpu::BindGroupEntry { binding: 1, resource: buf.p_re_buf.as_entire_binding() },
                wgpu::BindGroupEntry { binding: 2, resource: buf.p_im_buf.as_entire_binding() },
            ],
        });
        {
            let mut pass = enc.begin_compute_pass(&wgpu::ComputePassDescriptor {
                label: Some("fft_pass"),
                timestamp_writes: None,
            });
            pass.set_pipeline(&ctx.fft_pipeline);
            pass.set_bind_group(0, &fft_bg, &[]);
            pass.dispatch_workgroups(n_beams, 1, 1);
        }
        // ... fft dispatch ... then copy FFT output to staging for readback, all in same encoder
        enc.copy_buffer_to_buffer(&buf.p_re_buf, 0, &buf.stg_re, 0, spectrum_bytes);
        enc.copy_buffer_to_buffer(&buf.p_im_buf, 0, &buf.stg_im, 0, spectrum_bytes);
    } else {
        // CPU FFT path: copy MM output directly to staging
        enc.copy_buffer_to_buffer(&buf.mm_re_out, 0, &buf.stg_re, 0, spectrum_bytes);
        enc.copy_buffer_to_buffer(&buf.mm_im_out, 0, &buf.stg_im, 0, spectrum_bytes);
    }

    // Submit the entire frame in a single GPU command -> POLL 1
    queue.submit(std::iter::once(enc.finish()));
    device.poll(wgpu::Maintain::Wait);

    // map_async is asynchronous -> it requests CPU access to the buffer but doesn't block.
    // Map staging buffers (GPU work already done above) -> POLL 2
    let re_ok = std::sync::Arc::new(std::sync::atomic::AtomicBool::new(false));
    let im_ok = std::sync::Arc::new(std::sync::atomic::AtomicBool::new(false));
    let re_ok_cb = std::sync::Arc::clone(&re_ok);
    let im_ok_cb = std::sync::Arc::clone(&im_ok);

    buf.stg_re
        .slice(..)
        .map_async(wgpu::MapMode::Read, move |r| {
            re_ok_cb.store(r.is_ok(), std::sync::atomic::Ordering::Release);
        });
    buf.stg_im
        .slice(..)
        .map_async(wgpu::MapMode::Read, move |r| {
            im_ok_cb.store(r.is_ok(), std::sync::atomic::Ordering::Release);
        });
    device.poll(wgpu::Maintain::Wait);

    let re_mapped = re_ok.load(std::sync::atomic::Ordering::Acquire);
    let im_mapped = im_ok.load(std::sync::atomic::Ordering::Acquire);
    if !re_mapped || !im_mapped {
        eprintln!(
            "[sonar_wgpu] map_async failed: stg_re_ok={}, stg_im_ok={}",
            re_mapped, im_mapped
        );
        buf.stg_re.unmap();
        buf.stg_im.unmap();
        return std::ptr::null_mut();
    }

    let (mut p_re, mut p_im) = {
        let re_data = buf.stg_re.slice(..).get_mapped_range();
        let im_data = buf.stg_im.slice(..).get_mapped_range();
        let re = bytemuck::cast_slice::<u8, f32>(&re_data).to_vec();
        let im = bytemuck::cast_slice::<u8, f32>(&im_data).to_vec();
        (re, im)
    };
    buf.stg_re.unmap();
    buf.stg_im.unmap();

    // CPU FFT fallback (n_freq not power-of-two; uses Bluestein O(N log N))
    if !gpu_fft {
        let fft_t0 = std::time::Instant::now();
        let mut complex = vec![ComplexF::default(); spectrum_len];
        for i in 0..spectrum_len {
            complex[i].re = p_re[i];
            complex[i].im = p_im[i];
        }
        let fallback = fft::fft_batched(&complex, n_beams_us, n_freq_us);
        for i in 0..spectrum_len {
            p_re[i] = fallback[i].re;
            p_im[i] = fallback[i].im;
        }
        if gpu_frame == 0 || gpu_frame % 50 == 49 {
            eprintln!("[sonar_wgpu] CPU FFT:  {:6.1} ms ({} beams x {} freq)",
                fft_t0.elapsed().as_secs_f64() * 1000.0, n_beams, n_freq);
        }
    }

    // Matches CUDA's post-FFT * delta_f scaling
    let delta_f = bandwidth / (n_freq as f32);
    for i in 0..spectrum_len {
        p_re[i] *= delta_f;
        p_im[i] *= delta_f;
    }

    // Pack into interleaved [re0, im0, re1, im1, ...] for C++ caller
    let mut out = vec![0.0f32; spectrum_len * 2];
    for b in 0..n_beams_us {
        for f in 0..n_freq_us {
            let idx = b * n_freq_us + f;
            out[idx * 2] = p_re[idx];
            out[idx * 2 + 1] = p_im[idx];
        }
    }

    let elapsed_ms = gpu_t0.elapsed().as_secs_f64() * 1000.0;
    // Log on the very first frame and then every 50 frames.
    if gpu_frame == 0 || gpu_frame % 50 == 49 {
        let active_rays = (n_rays + std::cmp::max(1, ray_skips) - 1) / std::cmp::max(1, ray_skips);
        eprintln!(
            "[sonar_wgpu] GPU #{:<5} | {:6.1} ms | {} beams × {} rays × {} freq",
            gpu_frame + 1, elapsed_ms, n_beams, active_rays, n_freq
        );
    }

    let mut boxed = out.into_boxed_slice();
    let ptr = boxed.as_mut_ptr();
    std::mem::forget(boxed);
    ptr
}

#[no_mangle]
pub extern "C" fn sonar_wgpu_free(ptr: *mut f32, len: usize) {
    if ptr.is_null() || len == 0 {
        return;
    }
    unsafe {
        let _ = Vec::from_raw_parts(ptr, len, len);
    }
}
