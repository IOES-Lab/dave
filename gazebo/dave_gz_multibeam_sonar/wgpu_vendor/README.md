
Data flow with buffer names:

    CPU INPUT DATA
        │
        ▼
    depth_buf    [n_beams × n_rays]      f32   COPY_DST | STORAGE
    normal_buf   [n_beams × n_rays × 3]  f32   COPY_DST | STORAGE
    refl_buf     [n_beams × n_rays]      f32   COPY_DST | STORAGE
    window_buf   [n_freq]                f32   COPY_DST | STORAGE
    bc_buf       [n_beams × n_beams]     f32   COPY_DST | STORAGE
        │
        ▼ backscatter.wgsl (reads above, writes below)
        │
    out_re_i32   [n_beams × n_freq]      i32   COPY_DST | STORAGE  ← atomic accumulators
    out_im_i32   [n_beams × n_freq]      i32   COPY_DST | STORAGE  ← zeroed each frame
        │
        ▼ convert.wgsl (i32 → f32)
        │
    mm_re_in     [n_beams × n_freq]      f32   STORAGE
    mm_im_in     [n_beams × n_freq]      f32   STORAGE
        │
        ▼ matmul.wgsl (beam correction)
        │
    mm_re_out    [n_beams × n_freq]      f32   STORAGE | COPY_SRC
    mm_im_out    [n_beams × n_freq]      f32   STORAGE | COPY_SRC
        │
        ▼ copied into FFT buffers
        │
    p_re_buf     [n_beams × fft_len]     f32   STORAGE | COPY_SRC | COPY_DST
    p_im_buf     [n_beams × fft_len]     f32   STORAGE | COPY_SRC | COPY_DST
        │
        ▼ fft.wgsl (in-place FFT with zero padding to fft_len)
        │
        ▼ copied to staging
        │
    stg_re       [n_beams × fft_len]     f32   MAP_READ | COPY_DST
    stg_im       [n_beams × fft_len]     f32   MAP_READ | COPY_DST
        │
        ▼ CPU reads back first n_freq bins of final result
