/// The physics kernel.

struct Params {
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
  frame_lo: u32,
  frame_hi: u32,
  area_scaler: f32,
  h_fov: f32,
  v_fov: f32,
  _pad0: u32,
  _pad1: u32,
  _pad2: u32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read> depth: array<f32>;
@group(0) @binding(2) var<storage, read> normals: array<f32>;
@group(0) @binding(3) var<storage, read> reflectivity: array<f32>;
@group(0) @binding(4) var<storage, read_write> out_re: array<atomic<i32>>;
@group(0) @binding(5) var<storage, read_write> out_im: array<atomic<i32>>;
@group(0) @binding(6) var<storage, read> window_vals: array<f32>;

const PI: f32 = 3.14159265358979323846;
const SCALE: f32 = 1024.0;
const LIMIT: i32 = 2147483520i;  // near i32 max

// Philox4x32-10 RNG: WGPU/WGSL port of CUDA curand implementation.
// Based on Salmon et al., 2011, SC'11: "Random123: A Library of Counter-Based Random Number Generators".
// Based on https://github.com/DEShawResearch/random123/blob/main/include/Random123/philox.h
fn mulhilo32(a: u32, b: u32) -> vec2<u32> {
  let a_lo = a & 0xFFFFu;
  let a_hi = a >> 16u;
  let b_lo = b & 0xFFFFu;
  let b_hi = b >> 16u;
  let lo = a * b;
  let p0 = a_lo * b_lo;
  let p1 = a_hi * b_lo;
  let p2 = a_lo * b_hi;
  let p3 = a_hi * b_hi;
  let mid = (p0 >> 16u) + (p1 & 0xFFFFu) + (p2 & 0xFFFFu);
  let hi = p3 + (p1 >> 16u) + (p2 >> 16u) + (mid >> 16u);
  return vec2<u32>(hi, lo);
}

// Single Philox4x32 round.
fn philox_round(c: vec4<u32>, k: vec2<u32>) -> vec4<u32> {
  let r0 = mulhilo32(0xD2511F53u, c.x);
  let r1 = mulhilo32(0xCD9E8D57u, c.z);
  return vec4<u32>(r1.x ^ c.y ^ k.x, r1.y, r0.x ^ c.w ^ k.y, r0.y);
}

// Full 10-round Philox4x32.
fn philox4x32_10(ctr: vec4<u32>, key: vec2<u32>) -> vec4<u32> {
  var c = ctr;
  var k = key;
  c = philox_round(c, k); k.x += 0x9E3779B9u; k.y += 0xBB67AE85u;
  c = philox_round(c, k); k.x += 0x9E3779B9u; k.y += 0xBB67AE85u;
  c = philox_round(c, k); k.x += 0x9E3779B9u; k.y += 0xBB67AE85u;
  c = philox_round(c, k); k.x += 0x9E3779B9u; k.y += 0xBB67AE85u;
  c = philox_round(c, k); k.x += 0x9E3779B9u; k.y += 0xBB67AE85u;
  c = philox_round(c, k); k.x += 0x9E3779B9u; k.y += 0xBB67AE85u;
  c = philox_round(c, k); k.x += 0x9E3779B9u; k.y += 0xBB67AE85u;
  c = philox_round(c, k); k.x += 0x9E3779B9u; k.y += 0xBB67AE85u;
  c = philox_round(c, k); k.x += 0x9E3779B9u; k.y += 0xBB67AE85u;
  c = philox_round(c, k);
  return c;
}

// Convert raw u32 to uniform float (0,1].
fn philox_uniform(x: u32) -> f32 {
  return f32(x) * (1.0 / 4294967296.0) + (0.5 / 4294967296.0);
}

// Generate 4 normal-distributed floats using Box-Muller transform.
// Counter: [frame_lo, frame_hi, subsequence, 0], Key: [seed_lo, seed_hi]
// Mirrors CUDA curand_init(seed, subsequence, offset=0)
fn philox_normal4(seed_lo: u32, seed_hi: u32, frame_lo: u32, frame_hi: u32, subsequence: u32) -> vec4<f32> {
  let raw = philox4x32_10(vec4<u32>(frame_lo, frame_hi, subsequence, 0u), vec2<u32>(seed_lo, seed_hi));
  let u1 = philox_uniform(raw.x);
  let u2 = philox_uniform(raw.y);
  let u3 = philox_uniform(raw.z);
  let u4 = philox_uniform(raw.w);
  let r1 = sqrt(-2.0 * log(u1));
  let t1 = 2.0 * PI * u2;
  let r2 = sqrt(-2.0 * log(u3));
  let t2 = 2.0 * PI * u4;
  return vec4<f32>(r1 * sin(t1), r1 * cos(t1), r2 * sin(t2), r2 * cos(t2));
}

@compute @workgroup_size(8, 8, 1)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
  let beam = gid.x;
  let reduced_ray = gid.y;
  if (beam >= params.n_beams) {
    return;
  }

  let ray_step = max(1u, params.ray_skips);
  let actual_ray = reduced_ray * ray_step;
  if (actual_ray >= params.n_rays) {
    return;
  }

  let dr = beam * params.n_rays + actual_ray;
  let d = depth[dr];
  if (d <= 0.0 || d > params.max_distance) {
    return;
  }

  let nx = normals[dr * 3u + 0u];
  let ny = normals[dr * 3u + 1u];
  let nz = normals[dr * 3u + 2u];
  let refl = reflectivity[dr];

  // Compute sensor-frame ray unit direction from (beam, actual_ray) pixel indices.
  // Direction formula (sin(θ_b)cos(θ_r), sin(θ_r), cos(θ_b)cos(θ_r)) is unit-length by construction.
  let beam_ang = params.h_fov * (f32(beam)       / max(f32(params.n_beams) - 1.0, 1.0) - 0.5);
  let ray_ang  = params.v_fov * (f32(actual_ray) / max(f32(params.n_rays)  - 1.0, 1.0) - 0.5);
  let rd_x = sin(beam_ang) * cos(ray_ang);
  let rd_y = sin(ray_ang);
  let rd_z = cos(beam_ang) * cos(ray_ang);
  // let cos_inc = abs(rd_x * nx + rd_y * ny + rd_z * nz);
  let cos_inc = abs(nz);
  let lambert_sqrt = sqrt(max(refl, 0.0)) * cos_inc;

  let source_term = sqrt(pow(10.0, params.source_level / 10.0)) * 1e-6;
  let propagation = exp(-2.0 * params.attenuation * d) / (d * d);
  let target_area_sqrt = sqrt(d * params.area_scaler);
  // let amp_scale = source_term * propagation * lambert_sqrt * target_area_sqrt * params.sensor_gain;
  let amp_scale = source_term * propagation * lambert_sqrt * target_area_sqrt;

  let subsequence = beam * params.n_rays + actual_ray;
  let xi = philox_normal4(params.seed_lo, params.seed_hi, params.frame_lo, params.frame_hi, subsequence);

  let amp_re = (xi.x / sqrt(2.0)) * amp_scale;
  let amp_im = (xi.y / sqrt(2.0)) * amp_scale;

  let delta_f = params.bandwidth / f32(params.n_freq);
  let n_freq_f = f32(params.n_freq);
  let is_even = (params.n_freq % 2u) == 0u;

  // Keep window buffer bound for interface stability.
  _ = window_vals[0u];

  // Per-frequency phase sweep
  for (var f: u32 = 0u; f < params.n_freq; f = f + 1u) {
    var freq: f32;
    if (is_even) {
      freq = delta_f * (-n_freq_f + 2.0 * (f32(f) + 1.0)) / 2.0;
    } else {
      freq = delta_f * (-(n_freq_f - 1.0) + 2.0 * (f32(f) + 1.0)) / 2.0;
    }

    let kw = 2.0 * PI * freq / params.sound_speed;
    let phase = 2.0 * d * kw;
    let c = cos(phase);
    let s = sin(phase);

    // let re_val = clamp((amp_re * c - amp_im * s) * SCALE, -LIMIT, LIMIT);
    // let im_val = clamp((amp_re * s + amp_im * c) * SCALE, -LIMIT, LIMIT);
    let re_val = clamp(i32(round((amp_re * c - amp_im * s) * SCALE)), -LIMIT, LIMIT);
    let im_val = clamp(i32(round((amp_re * s + amp_im * c) * SCALE)), -LIMIT, LIMIT);

    let idx = beam * params.n_freq + f;
    atomicAdd(&out_re[idx], i32(re_val));
    atomicAdd(&out_im[idx], i32(im_val));
  }
}
