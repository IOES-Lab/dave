// Cooley-Tukey FFT with zero-padding to the next power of 2.
// Supports any n_freq <= 4096. The input is padded to padded_n (next power of 2 >= n_freq)
// in shared memory; only the first n_freq output bins are written back per beam.

struct Params {
  n_beams:  u32,
  n_freq:   u32,   // actual number of frequency bins (may not be power-of-2)
  padded_n: u32,   // next power of 2 >= n_freq (actual FFT size, <= 4096)
  log2_n:   u32,   // log2(padded_n)
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read_write> p_re: array<f32>;
@group(0) @binding(2) var<storage, read_write> p_im: array<f32>;

// Shared scratchpad for in-place FFT: sized for the maximum supported padded_n.
var<workgroup> smem_re: array<f32, 4096>;
var<workgroup> smem_im: array<f32, 4096>;

// Bit-reversal permutation used by the Cooley-Tukey butterfly.
fn bit_reverse(v: u32, bits: u32) -> u32 {
  var x = v;
  var y: u32 = 0u;
  for (var i: u32 = 0u; i < bits; i = i + 1u) {
    y = (y << 1u) | (x & 1u);
    x = x >> 1u;
  }
  return y;
}

// One workgroup per beam. Threads: 256 per workgroup.
@compute @workgroup_size(256, 1, 1)
fn main(
  @builtin(workgroup_id)       wg:  vec3<u32>,  // wg.x = beam index
  @builtin(local_invocation_id) lid: vec3<u32>  // lid.x = thread 0..255
) {
  let beam = wg.x;
  if (beam >= params.n_beams || params.padded_n > 4096u) {
    return;
  }

  let n      = params.padded_n;   // FFT size (power of 2)
  let n_real = params.n_freq;     // actual data count stored in the buffer
  let base   = beam * n_real;     // flat offset into the (non-padded) p_re/p_im buffers

  // Load with zero-padding into bit-reversed positions:
  //   indices [0, n_real) come from the buffer; [n_real, n) are zero-padded.
  for (var i: u32 = lid.x; i < n; i = i + 256u) {
    let j = bit_reverse(i, params.log2_n);
    if (i < n_real) {
      smem_re[j] = p_re[base + i];
      smem_im[j] = p_im[base + i];
    } else {
      smem_re[j] = 0.0;
      smem_im[j] = 0.0;
    }
  }
  workgroupBarrier();

  // Cooley-Tukey butterfly iterations over log2(padded_n) stages.
  for (var s: u32 = 0u; s < params.log2_n; s = s + 1u) {
    let half   = 1u << s;
    let stride = half << 1u;

    for (var i: u32 = lid.x; i < n / 2u; i = i + 256u) {
      let group = i / half;
      let j     = i % half;
      let i0    = group * stride + j;
      let i1    = i0 + half;

      // Twiddle factor: exp(-j * 2*pi * k / stride)
      let angle = -2.0 * 3.14159265358979323846 * f32(j) / f32(stride);
      let wr = cos(angle);
      let wi = sin(angle);

      // Butterfly: u ± w*t
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

  // Writeback: only the first n_real bins per beam; buffer layout stays n_beams * n_freq.
  for (var i: u32 = lid.x; i < n_real; i = i + 256u) {
    p_re[base + i] = smem_re[i];
    p_im[base + i] = smem_im[i];
  }
}
