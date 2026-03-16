// GPU GEMM: WGPU compute shader for beam correction via matrix multiply.
// Tiled approach: C = A*B normalized, with shared memory for cache optimization.

struct Params {
  n_beams: u32,
  n_freq: u32,
  beam_corrector_sum: f32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read> a: array<f32>;        // beam_corrector [nBeams*nBeams]
@group(0) @binding(2) var<storage, read> b: array<f32>;        // spectrum input [nBeams*nFreq]
@group(0) @binding(3) var<storage, read_write> c: array<f32>;  // output [nBeams*nFreq]

// Shared memory tiles for GEMM optimization (tile size 16x16).
var<workgroup> tile_a: array<array<f32, 16>, 16>;
var<workgroup> tile_b: array<array<f32, 16>, 16>;

// Tiled GEMM kernel: 16x16 workgroup, each thread computes one C[row,col] element.
@compute @workgroup_size(16, 16, 1)
fn main(
  @builtin(global_invocation_id) gid: vec3<u32>,
  @builtin(local_invocation_id) lid: vec3<u32>
) {
  let row = gid.y;
  let col = gid.x;
  if (row >= params.n_beams || col >= params.n_freq) {
    return;
  }

  // Accumulator: accumulates partial products across tiles.
  var acc = 0.0;
  let tiles = (params.n_beams + 15u) / 16u;

  // Loop over tiles of size 16: classic blocked GEMM.
  // (k = beam index in the inner product)
  for (var t: u32 = 0u; t < tiles; t = t + 1u) {
    let k_a = t * 16u + lid.x;
    let k_b = t * 16u + lid.y;

    // LOAD: load tile data into shared memory.
    // Load A tile: each thread loads one A[row, k_a].
    if (k_a < params.n_beams) {
      tile_a[lid.y][lid.x] = a[row * params.n_beams + k_a];
    } else {
      tile_a[lid.y][lid.x] = 0.0;
    }

    // Load B tile: each thread loads one B[k_b, col].
    if (k_b < params.n_beams) {
      tile_b[lid.y][lid.x] = b[k_b * params.n_freq + col];
    } else {
      tile_b[lid.y][lid.x] = 0.0;
    }

    workgroupBarrier();

    // COMPUTE: partial dot product.
    // Accumulate tile product: inner loop over tile dimension.
    for (var k: u32 = 0u; k < 16u; k = k + 1u) {
      acc = acc + tile_a[lid.y][k] * tile_b[k][lid.x];
    }

    workgroupBarrier();
  }

  // WRITE: final result to global memory.
  // Normalize output: divide by beam_corrector_sum.
  let norm = max(params.beam_corrector_sum, 1e-12);
  c[row * params.n_freq + col] = acc / norm;
}
