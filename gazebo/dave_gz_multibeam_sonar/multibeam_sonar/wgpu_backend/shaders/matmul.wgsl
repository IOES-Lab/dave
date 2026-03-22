// matmul.wgsl
// Beam correction pass (Choi et al. 2021)
//
// Applies beam pattern matrix W to redistribute energy across beams.
// Backscatter pass assumes D(θ,φ)=1, this step corrects that using W.
//
// C[beam, f] = Σ_k W[beam, k] * P_raw[k, f] / sum(W)

struct Params {
    n_beams: u32,
    n_freq: u32,
    beam_corrector_sum: f32, // used to keep total energy consistent
    _pad: u32,
};

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read> A: array<f32>; // [n_beams x n_beams]
@group(0) @binding(2) var<storage, read> B: array<f32>; // [n_beams x n_freq]
@group(0) @binding(3) var<storage, read_write> C: array<f32>; // [n_beams x n_freq]

// shared tiles (16x16)
var<workgroup> tile_A: array<array<f32, 16>, 16>;
var<workgroup> tile_B: array<array<f32, 16>, 16>;

@compute @workgroup_size(16, 16, 1)
fn main(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_id) lid: vec3<u32>
) {
    let row = gid.y; // beam index
    let col = gid.x; // frequency index

    // avoid out-of-bounds work when grid is padded
    let in_bounds = (row < params.n_beams) && (col < params.n_freq);

    var acc: f32 = 0.0;

    // number of tiles along k (beam) dimension
    let num_tiles = (params.n_beams + 15u) / 16u;

    // loop over tiles
    for (var t = 0u; t < num_tiles; t++) {

        // load A tile: row fixed, k varies
        let k_a = t * 16u + lid.x;
        if (row < params.n_beams && k_a < params.n_beams) {
            tile_A[lid.y][lid.x] = A[row * params.n_beams + k_a];
        } else {
            tile_A[lid.y][lid.x] = 0.0;
        }

        // load B tile: col fixed, k varies
        let k_b = t * 16u + lid.y;
        if (col < params.n_freq && k_b < params.n_beams) {
            tile_B[lid.y][lid.x] = B[k_b * params.n_freq + col];
        } else {
            tile_B[lid.y][lid.x] = 0.0;
        }

        // wait until tile is fully loaded
        workgroupBarrier();

        // multiply tile rows × cols
        // each thread computes one output element
        for (var k = 0u; k < 16u; k++) {
            acc += tile_A[lid.y][k] * tile_B[k][lid.x];
        }

        // sync before next tile overwrite
        workgroupBarrier();
    }

    // normalise to avoid energy scaling due to W
    if (in_bounds) {
        let norm = max(params.beam_corrector_sum, 1e-12);
        C[row * params.n_freq + col] = acc / norm;
    }
}