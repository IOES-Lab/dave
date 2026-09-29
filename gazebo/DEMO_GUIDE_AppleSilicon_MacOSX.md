# Multibeam Sonar Demo Guide
Build and run the GPU multibeam sonar demo. The plugin supports wgpu (Vulkan), and CPU backends—pick one at launch without recompiling.

![Multibeam Sonar Demo](https://github.com/user-attachments/assets/8fbe8a3f-e917-42af-a51d-c49f133da882)

## Setup

### Native Installation of ROS2 Jazzy and Gazebo Harmonic
- Follow the instructions in this guide to set up ROS2 Jazzy and Gazebo Harmonic natively on Apple Silicon: https://github.com/IOES-Lab/ROS2_Jazzy_MacOS_Native_AppleSilicon

### ROS_GZ for Mac
- Install ROS_GZ following the instructions here: https://github.com/IOES-Lab/ROS_GZ_MacOS_Native_AppleSilicon

### RUST and Cargo
- Install Rust and Cargo using the official installer: https://www.rust-lang.org/tools/install
  ```bash
  curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
  ```

### Vulkan SDK
- Install with brew:
    ```bash
    brew install vulkan-tools
    ```

## Build

Build in two steps on a fresh clone (the Rust library must be ready before the plugin can link to it):

```bash
source /opt/ros/jazzy/setup.bash

# Step 1: build and install the Rust library
colcon build --packages-select wgpu_vendor
source install/setup.bash

# Step 2: build the plugin and everything else
colcon build --packages-select dave_demos dave_worlds dave_interfaces multibeam_sonar multibeam_sonar_system dave_multibeam_sonar_demo dave_sensor_models
source install/setup.bash
```

On subsequent builds, you can run all packages in one command.

## Run

```bash
ros2 launch dave_multibeam_sonar_demo multibeam_sonar_demo.launch.py compute_backend:=wgpu
```

Use `compute_backend:=cuda` for CUDA, `compute_backend:=cpu` for CPU, or `compute_backend:=auto` to pick the best available (tries wgpu -> cuda -> cpu).

You'll see RViz2 launch with a point cloud display. The sonar fan should update in real time. Check the terminal for initialization messages and per-frame timing.

## Troubleshooting

**`Could not find wgpu_vendorConfig.cmake`** -> You skipped Step 1. Build `wgpu_vendor` first, source install, then build the rest.

**`Another world of the same name is running`** -> Kill stale Gazebo with `pkill -9 -f gz` and try again.

**CUDA backend won't initialize** -> Run `nvidia-smi` to check if the driver is loaded. If not, reinstall or reload with `sudo modprobe nvidia`.


## How It Works

Each frame, Gazebo renders depth and surface normals. The sonar plugin reads these and runs acoustic physics on your selected backend. The wgpu backend dispatches four compute shaders: backscatter (acoustic return per ray), convert (fixed-point i32 -> f32), matmul (beam correction), and FFT (range compression). Output goes to ROS 2 topics as a point cloud and sonar image via ros_gz_bridge. The Rust library compiles to a static library linked into the C++ Gazebo plugin via C FFI.

## Data Flow

**Pipeline stages:**

1. **Input Buffers** - CPU writes depth, normal maps, reflectivity, window function, beam correction matrix
2. **backscatter.wgsl** - Computes acoustic return per ray using Lambert model, outputs to atomic accumulators
3. **convert.wgsl** - Converts fixed-point i32 results to f32
4. **matmul.wgsl** - Applies beam correction matrix to each beam
5. **fft.wgsl** - Performs in-place FFT with zero-padding to power-of-2 for range compression
6. **Readback** - CPU reads first n_freq bins from staging buffers
7. **Output** - Results published to ROS 2 as point cloud and sonar image

**Buffer details:**

| Buffer | Dimensions | Type | Usage |
|--------|-----------|------|-------|
| depth_buf, normal_buf, refl_buf | n_beams × n_rays | f32 | Input from Gazebo |
| out_re_i32, out_im_i32 | n_beams × n_freq | i32 | Atomic accumulators (zeroed each frame) |
| mm_re_in, mm_im_in | n_beams × n_freq | f32 | After convert pass |
| mm_re_out, mm_im_out | n_beams × n_freq | f32 | After beam correction |
| p_re_buf, p_im_buf | n_beams × fft_len | f32 | FFT input/output (zero-padded) |
| stg_re, stg_im | n_beams × fft_len | f32 | Staging for CPU readback |