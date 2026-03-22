// lib.rs
// C-facing entry points for the wgpu sonar backend.
// called from C++ via extern "C"

mod physics_engine;
use physics_engine::{PhysicsInput, SonarConfig, SonarPhysicsEngine};

// create
#[no_mangle]
pub extern "C" fn sonar_wgpu_create(
    n_beams:      u32,
    n_rays:       u32,
    n_freq:       u32,
    sound_speed:  f32,
    bandwidth:    f32,
    max_range:    f32,
    attenuation:  f32,
    source_level: f32,
    sensor_gain:  f32,
    h_fov:        f32,
    v_fov:        f32,
    seed:         u64,
) -> *mut SonarPhysicsEngine {
    // FFT shader needs power-of-2
    let n_freq_gpu = if n_freq.is_power_of_two() {
        n_freq
    } else {
        n_freq.next_power_of_two().min(4096)
    };

    let config = SonarConfig {
        n_beams,
        n_rays,
        n_freq: n_freq_gpu,
        sound_speed,
        bandwidth,
        max_range,
        attenuation,
        source_level,
        sensor_gain,
        h_fov,
        v_fov,
        mu_default: 0.5,
        seed: seed as u32,
    };

    // wgpu init can panic if no device / driver
    match std::panic::catch_unwind(|| SonarPhysicsEngine::new(config)) {
        Ok(engine) => Box::into_raw(Box::new(engine)),
        Err(e) => {
            eprintln!(
                "[sonar_wgpu] create failed: {:?}",
                e.downcast_ref::<&str>().unwrap_or(&"<unknown>")
            );
            std::ptr::null_mut()
        }
    }
}

// compute
#[no_mangle]
pub unsafe extern "C" fn sonar_wgpu_compute(
    engine:        *mut SonarPhysicsEngine,
    depth:         *const f32,
    normals:       *const f32,
    refl:          *const f32,
    beam_corr:     *const f32,
    n_beams:       u32,
    n_rays:        u32,
    n_freq:        u32,
    frame:         u64,
    beam_corr_sum: f32,
) -> *mut f32 {
    // basic sanity checks (avoid UB)
    if engine.is_null()
        || depth.is_null()
        || normals.is_null()
        || refl.is_null()
        || beam_corr.is_null()
    {
        return std::ptr::null_mut();
    }

    let eng = &mut *engine;

    let ray_total  = (n_beams * n_rays) as usize;
    let corr_total = (n_beams * n_beams) as usize;

    // wrap raw pointers into slices
    let input = PhysicsInput {
        depth:          std::slice::from_raw_parts(depth,     ray_total),
        normals:        std::slice::from_raw_parts(normals,   ray_total * 3),
        reflectivity:   std::slice::from_raw_parts(refl,      ray_total),
        beam_corrector: std::slice::from_raw_parts(beam_corr, corr_total),
        beam_corr_sum:  if beam_corr_sum > 0.0 { beam_corr_sum } else { n_beams as f32 },
        frame:          frame as u32,
        seed:           42,
    };

    // run GPU pipeline (catch device loss etc.)
    let result = std::panic::catch_unwind(
        std::panic::AssertUnwindSafe(|| eng.run(&input))
    );

    match result {
        Ok(output) => {
            let out_len = (n_beams * n_freq) as usize * 2;
            let mut buf = Vec::with_capacity(out_len);

            // flatten to [re, im, re, im, ...]
            for b in 0..(n_beams as usize) {
                for f in 0..(n_freq as usize) {
                    let idx = b * (n_freq as usize) + f;

                    // intensity = |p|^2, approximate amplitude
                    let amp = output.intensity.get(idx).copied().unwrap_or(0.0);
                    let re  = amp.sqrt();

                    buf.push(re);
                    buf.push(0.0); // imag part unused here
                }
            }

            let ptr = buf.as_mut_ptr();
            std::mem::forget(buf); // hand ownership to caller
            ptr
        }
        Err(e) => {
            eprintln!(
                "[sonar_wgpu] compute failed: {:?}",
                e.downcast_ref::<&str>().unwrap_or(&"<unknown>")
            );
            std::ptr::null_mut()
        }
    }
}

// free
#[no_mangle]
pub unsafe extern "C" fn sonar_wgpu_free(ptr: *mut f32, len: usize) {
    if !ptr.is_null() && len > 0 {
        // reconstruct vec and drop it
        drop(Vec::from_raw_parts(ptr, len, len));
    }
}

// destroy
#[no_mangle]
pub unsafe extern "C" fn sonar_wgpu_destroy(engine: *mut std::ffi::c_void) {
    if !engine.is_null() {
        // pointer came from Box::into_raw
        drop(Box::from_raw(engine as *mut SonarPhysicsEngine));
    }
}