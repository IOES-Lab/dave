/// Fixed-point atomic accumulation in backscatter.wgsl (SCALE=1024) allows us
/// to avoid floating-point atomics, which are not widely supported on GPUs. This shader
/// converts the accumulated i32 values back to f32 by dividing by SCALE=1024, precision 0.001.
@group(0) @binding(0) var<storage, read> in_re: array<i32>;
@group(0) @binding(1) var<storage, read> in_im: array<i32>;
@group(0) @binding(2) var<storage, read_write> out_re: array<f32>;
@group(0) @binding(3) var<storage, read_write> out_im: array<f32>;

@compute @workgroup_size(64)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&in_re)) { return; }
    out_re[i] = f32(in_re[i]) / 1024.0;
    out_im[i] = f32(in_im[i]) / 1024.0;
}
