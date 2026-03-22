// backscatter.wgsl
// Based on Choi et al. (2021)
// Uses Eq.14 for per-ray amplitude and Eq.8 for frequency accumulation

struct Params {
    n_beams: u32,
    n_rays: u32,
    n_freq: u32,
    _pad0: u32,

    sound_speed: f32,
    bandwidth: f32,
    max_range: f32,
    attenuation: f32,

    h_fov: f32,
    v_fov: f32,
    mu_default: f32,
    _pad1: f32,

    seed: u32,
    frame: u32,
    _pad2: u32,
    _pad3: u32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read> depth: array<f32>;     // per-ray distance
@group(0) @binding(2) var<storage, read> normals: array<f32>;   // packed xyz normals
@group(0) @binding(3) var<storage, read> refl: array<f32>;      // reflectivity per ray
@group(0) @binding(4) var<storage, read_write> out_re: array<atomic<i32>>;
@group(0) @binding(5) var<storage, read_write> out_im: array<atomic<i32>>;

const PI: f32 = 3.141592653589793;
const SCALE: f32 = 1048576.0; // fixed-point scale for atomics

//  RNG (Philox-style counter RNG, deterministic per (frame, beam, ray)) ---
fn mulhilo32(a: u32, b: u32) -> vec2<u32> {
    // manual 32-bit multiply split (WGSL doesn't expose 64-bit directly)
    let a_lo = a & 0xFFFFu;
    let a_hi = a >> 16u;
    let b_lo = b & 0xFFFFu;
    let b_hi = b >> 16u;

    let p0 = a_lo * b_lo;
    let p1 = a_hi * b_lo;
    let p2 = a_lo * b_hi;
    let p3 = a_hi * b_hi;

    let mid = (p0 >> 16u) + (p1 & 0xFFFFu) + (p2 & 0xFFFFu);
    let hi = p3 + (p1 >> 16u) + (p2 >> 16u) + (mid >> 16u);

    return vec2<u32>(hi, a * b);
}

fn philox_round(c: vec4<u32>, k: vec2<u32>) -> vec4<u32> {
    // one Feistel round
    let r0 = mulhilo32(0xD2511F53u, c.x);
    let r1 = mulhilo32(0xCD9E8D57u, c.z);
    return vec4<u32>(r1.x ^ c.y ^ k.x, r1.y, r0.x ^ c.w ^ k.y, r0.y);
}

fn philox4x32_10(ctr: vec4<u32>, key: vec2<u32>) -> vec4<u32> {
    // 10 rounds → good statistical quality, still fast on GPU
    var c = ctr;
    var k = key;

    for (var i = 0; i < 9; i++) {
        c = philox_round(c, k);
        k.x += 0x9E3779B9u; // Weyl sequence increment
        k.y += 0xBB67AE85u;
    }
    c = philox_round(c, k);
    return c;
}

fn u32_to_unit(x: u32) -> f32 {
    // map to (0,1] to avoid log(0)
    return f32(x) * (1.0 / 4294967296.0) + (0.5 / 4294967296.0);
}

fn philox_normal4(seed: u32, frame: u32, subseq: u32) -> vec4<f32> {
    // Box-Muller: convert uniform → Gaussian (needed for speckle)
    let raw = philox4x32_10(
        vec4<u32>(frame, 0u, subseq, 0u),
        vec2<u32>(seed, 0u)
    );

    let u1 = u32_to_unit(raw.x);
    let u2 = u32_to_unit(raw.y);
    let u3 = u32_to_unit(raw.z);
    let u4 = u32_to_unit(raw.w);

    let r1 = sqrt(-2.0 * log(u1));
    let r2 = sqrt(-2.0 * log(u3));

    return vec4<f32>(
        r1 * sin(2.0 * PI * u2),
        r1 * cos(2.0 * PI * u2),
        r2 * sin(2.0 * PI * u4),
        r2 * cos(2.0 * PI * u4)
    );
}

@compute @workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {

    let beam = gid.x;
    let ray  = gid.y;

    // guard against over-dispatch
    if (beam >= params.n_beams || ray >= params.n_rays) {
        return;
    }

    let idx = beam * params.n_rays + ray;
    let r = depth[idx];

    // skip invalid or out-of-range rays early (cheap reject)
    if (r <= 0.001 || r > params.max_range) {
        return;
    }

    // fetch surface info
    let nx = normals[idx * 3u + 0u];
    let ny = normals[idx * 3u + 1u];
    let nz = normals[idx * 3u + 2u];
    let mu = max(refl[idx], 0.0); // clamp just in case input is noisy

    // convert pixel position → direction (sensor frame)
    let beam_ang = params.h_fov * (f32(beam) / max(f32(params.n_beams) - 1.0, 1.0) - 0.5);
    let ray_ang  = params.v_fov * (f32(ray)  / max(f32(params.n_rays)  - 1.0, 1.0) - 0.5);

    let rd_x = sin(beam_ang) * cos(ray_ang);
    let rd_y = sin(ray_ang);
    let rd_z = cos(beam_ang) * cos(ray_ang);

    // incidence term |dot(ray, normal)| → energy drop at grazing angles
    let cos_inc = abs(rd_x * nx + rd_y * ny + rd_z * nz);

    // differential area (solid angle scaled by range²)
    let d_theta_h = params.h_fov / max(f32(params.n_beams) - 1.0, 1.0);
    let d_theta_v = params.v_fov / max(f32(params.n_rays)  - 1.0, 1.0);
    let dA = r * r * d_theta_h * d_theta_v;

    // attenuation + spreading (simple model)
    let TL = exp(-params.attenuation * r) / r;

    // Eq.14 deterministic amplitude term
    let A_det = sqrt(mu) * cos_inc * sqrt(dA) * TL;

    // stochastic component → produces speckle when summed
    let subseq = beam * params.n_rays + ray;
    let xi = philox_normal4(params.seed, params.frame, subseq);

    let amp_re = A_det * (xi.x / sqrt(2.0));
    let amp_im = A_det * (xi.y / sqrt(2.0));

    let delta_f  = params.bandwidth / f32(params.n_freq);
    let n_freq_f = f32(params.n_freq);
    let is_even  = (params.n_freq & 1u) == 0u;

    for (var f = 0u; f < params.n_freq; f++) {

        // match fft-style frequency layout (centered around 0)
        var freq: f32;
        if (is_even) {
            freq = delta_f * (-n_freq_f + 2.0 * (f32(f) + 1.0)) / 2.0;
        } else {
            freq = delta_f * (-(n_freq_f - 1.0) + 2.0 * (f32(f) + 1.0)) / 2.0;
        }

        // Eq.8 phase term (two-way travel)
        let k   = 2.0 * PI * freq / params.sound_speed;
        let phi = 2.0 * r * k;

        let c = cos(phi);
        let s = sin(phi);

        // complex multiply (manual since no complex type)
        let contrib_re = amp_re * c - amp_im * s;
        let contrib_im = amp_re * s + amp_im * c;

        let out_idx = beam * params.n_freq + f;

        // clamp before converting → avoid overflow in atomics
        let re_i32 = i32(clamp(contrib_re * SCALE, -2147483520.0, 2147483520.0));
        let im_i32 = i32(clamp(contrib_im * SCALE, -2147483520.0, 2147483520.0));

        // accumulate across rays (parallel safe)
        atomicAdd(&out_re[out_idx], re_i32);
        atomicAdd(&out_im[out_idx], im_i32);
    }
}