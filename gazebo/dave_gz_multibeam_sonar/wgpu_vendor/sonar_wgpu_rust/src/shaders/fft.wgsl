// Cooley-Tukey FFT (power-of-2 sizes only, N≤4096), or CPU fallback for arbitrary N.

struct Params {
  n_beams: u32,
  n_freq: u32,
  log2_n: u32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read_write> p_re: array<f32>;
@group(0) @binding(2) var<storage, read_write> p_im: array<f32>;

// WGSL writes to shared memory (smem_re/im) -> fast scratchpad local to workgroup.
var<workgroup> smem_re: array<f32, 4096>;
var<workgroup> smem_im: array<f32, 4096>;

// Bit reversal for FFT permutation.
fn bit_reverse(v: u32, bits: u32) -> u32 {
  var x = v;
  var y: u32 = 0u;
  for (var i: u32 = 0u; i < bits; i = i + 1u) {
    y = (y << 1u) | (x & 1u);
    x = x >> 1u;
  }
  return y;
}

// Cooley-Tukey FFT kernel with bit-reversal, butterfly stages, and writeback.
@compute @workgroup_size(256, 1, 1)
fn main(
  @builtin(workgroup_id) wg: vec3<u32>,             // (wg) -> which beam this workgroup handles
  @builtin(local_invocation_id) lid: vec3<u32>      // (lid) -> which thread within the workgroup (0..255)
) {
  let beam = wg.x;
  if (beam >= params.n_beams || params.n_freq > 4096u) {
    return;
  }

  let n = params.n_freq;
  let base = beam * n;

  // Bit-reversal permutation.
  for (var i: u32 = lid.x; i < n; i = i + 256u) {
    let j = bit_reverse(i, params.log2_n);
    smem_re[j] = p_re[base + i];
    smem_im[j] = p_im[base + i];
  }
  workgroupBarrier();

  // Cooley-Tukey butterfly iterations.
  for (var s: u32 = 0u; s < params.log2_n; s = s + 1u) {
    let half = 1u << s;
    let stride = half << 1u;

    for (var i: u32 = lid.x; i < n / 2u; i = i + 256u) {
      let group = i / half;
      let j = i % half;
      let i0 = group * stride + j;
      let i1 = i0 + half;

      // Twiddle factor: root-of-unity exp(-j*2*pi*k/N).
      let angle = -2.0 * 3.14159265358979323846 * f32(j) / f32(stride);
      let wr = cos(angle);
      let wi = sin(angle);

      // Butterfly combine: complex multiply (u ± w*t).
      let tr = smem_re[i1] * wr - smem_im[i1] * wi;
      let ti = smem_re[i1] * wi + smem_im[i1] * wr;
      let ur = smem_re[i0];
      let ui = smem_im[i0];

      smem_re[i0] = ur + tr;
      smem_im[i0] = ui + ti;
      smem_re[i1] = ur - tr;
      smem_im[i1] = ui - ti;
    }

    workgroupBarrier();
  }

  // Writeback to global storage.
  for (var i: u32 = lid.x; i < n; i = i + 256u) {
    p_re[base + i] = smem_re[i];
    p_im[base + i] = smem_im[i];
  }
}
