// convert.wgsl
// takes the accumulated i32 buffers from scatter pass
// converts them back to float so next stages can use them

struct Params {
    n_elements: u32,   // total entries (beams * freq bins)
    scale:      f32,   // same scale factor used during atomic adds
    _pad0:      u32,   // alignment padding (required for uniform layout)
    _pad1:      u32,
};

@group(0) @binding(0) var<uniform> params: Params;

// input buffers (fixed-point values)
@group(0) @binding(1) var<storage, read> in_re: array<i32>;
@group(0) @binding(2) var<storage, read> in_im: array<i32>;

// output buffers (converted back to float)
@group(0) @binding(3) var<storage, read_write> out_re: array<f32>;
@group(0) @binding(4) var<storage, read_write> out_im: array<f32>;

@compute @workgroup_size(256)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;

    // guard against over-dispatch
    if (i >= params.n_elements) {
        return;
    }

    // undo fixed-point scaling
    let s = params.scale;

    out_re[i] = f32(in_re[i]) / s;
    out_im[i] = f32(in_im[i]) / s;
}