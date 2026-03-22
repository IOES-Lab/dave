// fft.wgsl
// Spectrum → range conversion using radix-2 FFT
// Based on beam spectrum from backscatter pass (Eq.8 → time domain)

struct Params {
    n_beams: u32,
    n_freq:  u32,
    log2_n:  u32,   // precomputed log2(n_freq)
    _pad:    u32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read_write> p_re: array<f32>;
@group(0) @binding(2) var<storage, read_write> p_im: array<f32>;

// shared memory for one beam
var<workgroup> smem_re: array<f32, 4096>;
var<workgroup> smem_im: array<f32, 4096>;

const PI: f32 = 3.141592653589793;

// reverse lowest 'bits' bits (needed for FFT input reordering)
fn bit_reverse(v: u32, bits: u32) -> u32 {
    var x = v;
    var y = 0u;

    for (var i = 0u; i < bits; i++) {
        y = (y << 1u) | (x & 1u);
        x >>= 1u;
    }
    return y;
}

// one workgroup = one beam
@compute @workgroup_size(256, 1, 1)
fn main(
    @builtin(workgroup_id) wg: vec3<u32>,
    @builtin(local_invocation_id) lid: vec3<u32>
) {
    let beam = wg.x;

    // simple guard (also enforces shared memory limit)
    if (beam >= params.n_beams || params.n_freq > 4096u) {
        return;
    }

    let n = params.n_freq;
    let base = beam * n;

    //  Step 1: load + bit-reversal 
    // FFT expects bit-reversed ordering before butterfly stages
    // each thread loads multiple elements (stride = workgroup size)
    for (var i = lid.x; i < n; i += 256u) {
        let j = bit_reverse(i, params.log2_n);

        // write directly into shared memory in reordered form
        smem_re[j] = p_re[base + i];
        smem_im[j] = p_im[base + i];
    }
    workgroupBarrier();

    //  Step 2: butterfly stages 
    // log2(n) stages, each doubling the merge size
    for (var s = 0u; s < params.log2_n; s++) {

        let half   = 1u << s;        // size of sub-FFT
        let stride = half << 1u;     // full butterfly width

        // total butterflies per stage = n/2
        for (var i = lid.x; i < n / 2u; i += 256u) {

            // map flat index → butterfly indices
            let group = i / half;    // which block
            let j     = i % half;    // position inside block

            let i0 = group * stride + j;   // top element
            let i1 = i0 + half;            // bottom element

            // twiddle factor: exp(-i * 2π * j / stride)
            // negative sign → forward FFT convention
            let angle = -2.0 * PI * f32(j) / f32(stride);
            let wr = cos(angle);
            let wi = sin(angle);

            // t = W * x[i1]
            let tr = smem_re[i1] * wr - smem_im[i1] * wi;
            let ti = smem_re[i1] * wi + smem_im[i1] * wr;

            // butterfly combine
            let ur = smem_re[i0];
            let ui = smem_im[i0];

            smem_re[i0] = ur + tr;
            smem_im[i0] = ui + ti;

            smem_re[i1] = ur - tr;
            smem_im[i1] = ui - ti;
        }

        // sync before next stage (data dependency)
        workgroupBarrier();
    }

    //  Step 3: write back 
    // output is time-domain signal per beam
    // range mapping is handled outside (t → r conversion)
    for (var i = lid.x; i < n; i += 256u) {
        p_re[base + i] = smem_re[i];
        p_im[base + i] = smem_im[i];
    }
}