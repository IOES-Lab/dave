/*
 * Copyright 2020 Naval Postgraduate School
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "sonar_calculation_cuda.cuh"

// #include <math.h>
#include <assert.h>

// For complex numbers
#include <cuComplex.h>
#include <thrust/complex.h>

// For rand() function
#include <curand.h>
#include <curand_kernel.h>
#include <unistd.h>

// For FFT
#include <cufft.h>
#include <cufftw.h>
#include <thrust/device_vector.h>
#include <list>

#include <cublas_v2.h>
#include <cuda_runtime.h>
#include <math_constants.h>
#include <chrono>
#include <ctime>

// FOR DEBUG -- DEV VERSION
#include <fstream>

std::ofstream debugLog("debug_timings.txt");

#define BLOCK_SIZE 32

// Existing SAFE_CALL (for cudaError_t)
#define SAFE_CALL(call, msg) _safe_cuda_call((call), msg, __FILE__, __LINE__)

// New SAFE_CUBLAS_CALL (for cublasStatus_t)
#define SAFE_CUBLAS_CALL(call, msg)                                                             \
  {                                                                                             \
    cublasStatus_t err = (call);                                                                \
    if (err != CUBLAS_STATUS_SUCCESS)                                                           \
    {                                                                                           \
      fprintf(stderr, "CUBLAS Error: %s in %s at line %d: %d\n", msg, __FILE__, __LINE__, err); \
      exit(EXIT_FAILURE);                                                                       \
    }                                                                                           \
  }

#define SAFE_CUFFT_CALL(call, msg)                                                             \
  {                                                                                            \
    cufftResult err = (call);                                                                  \
    if (err != CUFFT_SUCCESS)                                                                  \
    {                                                                                          \
      fprintf(stderr, "CUFFT Error: %s in %s at line %d: %d\n", msg, __FILE__, __LINE__, err); \
      exit(EXIT_FAILURE);                                                                      \
    }                                                                                          \
  }

static inline void _safe_cuda_call(
  cudaError err, const char * msg, const char * file_name, const int line_number)
{
  if (err != cudaSuccess)
  {
    fprintf(
      stderr, "%s\n\nFile: %s\n\nLine Number: %d\n\nReason: %s\n", msg, file_name, line_number,
      cudaGetErrorString(err));
    std::cin.get();
    exit(EXIT_FAILURE);
  }
}

// Persistent GPU memory pointers (device)
static float * d_depth_image = nullptr;
static float * d_normal_image = nullptr;
static float * d_reflectivity_image = nullptr;
static float * d_ray_elevationAngles = nullptr;
static thrust::complex<float> * d_P_Beams = nullptr;
static thrust::complex<float> * P_Beams = nullptr;
static bool memory_initialized = false;
float * ray_elevationAngles = nullptr;

float *P_Beams_Cor_real_h = nullptr, *P_Beams_Cor_imag_h = nullptr;
float *d_P_Beams_Cor_real = nullptr, *d_P_Beams_Cor_imag = nullptr;
float *d_P_Beams_Cor_F_real = nullptr, *d_P_Beams_Cor_F_imag = nullptr;
float *beamCorrector_lin_h = nullptr, *d_beamCorrector_lin = nullptr;

int P_Beams_Cor_N;
int P_Beams_Cor_Bytes;
int beamCorrector_lin_N;
int beamCorrector_lin_Bytes;

// FFT
cufftComplex * deviceInputData;
cufftComplex * deviceOutputData;

///////////////////////////////////////////////////////////////////////////
// Incident Angle Calculation Function
// incidence angle is target's normal angle accounting for the ray's azimuth
// and elevation
__device__ float compute_incidence(float azimuth, float elevation, float * normal)
{
  // ray normal from camera azimuth and elevation
  float camera_x = cosf(-azimuth) * cosf(elevation);
  float camera_y = sinf(-azimuth) * cosf(elevation);
  float camera_z = sinf(elevation);
  float ray_normal[3] = {camera_x, camera_y, camera_z};

  // target normal with axes compensated to camera axes
  float target_normal[3] = {normal[2], -normal[0], -normal[1]};

  // dot product
  float dot_product = ray_normal[0] * target_normal[0] + ray_normal[1] * target_normal[1] +
                      ray_normal[2] * target_normal[2];

  return M_PI - acosf(dot_product);
}

///////////////////////////////////////////////////////////////////////////
__device__ __host__ float unnormalized_sinc(float t)
{
  if (abs(t) < 1E-8)
  {
    return 1.0;
  }
  else
  {
    return sin(t) / t;
  }
}

__global__ void reduce_beams_kernel(
  const thrust::complex<float> * __restrict__ d_P_Beams, float * d_P_Beams_Cor_real,
  float * d_P_Beams_Cor_imag, int nBeams, int nFreq, int nRaysSkipped)
{
  const int beam = blockIdx.y;
  const int f = blockIdx.x;

  if (beam >= nBeams || f >= nFreq)
  {
    return;
  }

  __shared__ float shared_real[BLOCK_SIZE];
  __shared__ float shared_imag[BLOCK_SIZE];

  float sum_real = 0.0f;
  float sum_imag = 0.0f;

  for (int ray = threadIdx.x; ray < nRaysSkipped; ray += blockDim.x)
  {
    int idx = beam * nFreq * nRaysSkipped + ray * nFreq + f;

    thrust::complex<float> val = d_P_Beams[idx];
    sum_real += val.real();
    sum_imag += val.imag();
  }

  shared_real[threadIdx.x] = sum_real;
  shared_imag[threadIdx.x] = sum_imag;

  __syncthreads();

  for (int s = blockDim.x / 2; s > 0; s >>= 1)
  {
    if (threadIdx.x < s)
    {
      shared_real[threadIdx.x] += shared_real[threadIdx.x + s];
      shared_imag[threadIdx.x] += shared_imag[threadIdx.x + s];
    }
    __syncthreads();
  }

  if (threadIdx.x == 0)
  {
    int output_idx = f * nBeams + beam;
    d_P_Beams_Cor_real[output_idx] = shared_real[0];
    d_P_Beams_Cor_imag[output_idx] = shared_imag[0];
  }
}

///////////////////////////////////////////////////////////////////////////
// Sonar Claculation Function
__global__ void sonar_calculation(
  thrust::complex<float> * P_Beams, float * depth_image, float * normal_image, int width,
  int height, int depth_image_step, int normal_image_step, unsigned long long seed,
  float * reflectivity_image, int reflectivity_image_step, float hPixelSize, float vPixelSize,
  float hFOV, float vFOV, float beam_azimuthAngleWidth, float beam_elevationAngleWidth,
  float ray_azimuthAngleWidth, float * ray_elevationAngles, float ray_elevationAngleWidth,
  float soundSpeed, float sourceTerm, int nBeams, int nRays, int raySkips, float sonarFreq,
  float delta_f, int nFreq, float bandwidth, float maxDistance, float attenuation,
  float area_scaler)
{
  // 2D Index of current thread
  const int beam = blockIdx.x * blockDim.x + threadIdx.x;
  const int ray = blockIdx.y * blockDim.y + threadIdx.y;

  // Only valid threads perform memory I/O
  if ((beam < width) && (ray < height) && (ray % raySkips == 0))
  {
    // Location of the image pixel
    const int depth_index = ray * depth_image_step / sizeof(float) + beam;
    const int normal_index = ray * normal_image_step / sizeof(float) + (3 * beam);
    const int reflectivity_index = ray * reflectivity_image_step / sizeof(float) + beam;

    // Input parameters for ray processing
    float distance = depth_image[depth_index] * 1.0f;
    float normal[3] = {
      normal_image[normal_index], normal_image[normal_index + 1], normal_image[normal_index + 2]};

    // Beam pattern
    // only one column of rays for each beam at beam center, interference calculated later
    float azimuthBeamPattern = 1.0;
    float elevationBeamPattern = 1.0;
    // float elevationBeamPattern = abs(unnormalized_sinc(M_PI * 0.884
    //                              / (beam_elevationAngleWidth) *
    //    sin(ray_elevationAngles[ray])));

    // printf("angles %f", ray_elevationAngles[ray]);

    // incidence angle (taking that of normal_image)
    float incidence =
      acos(normal[2]);  // compute_incidence(ray_azimuthAngle, ray_elevationAngle, normal);

    // ----- Point scattering model ------ //
    curandStatePhilox4_32_10_t state;
    curand_init(seed, beam * height + ray, 0, &state);  // seed, unique id, offset, state

    float4 xi = curand_normal4(&state);  // standard 4 normal random values

    float xi_z = xi.x;
    float xi_y = xi.y;

    // Calculate amplitude
    thrust::complex<float> randomAmps = thrust::complex<float>(xi_z / sqrt(2.0), xi_y / sqrt(2.0));
    thrust::complex<float> lambert_sqrt =
      thrust::complex<float>(sqrt(reflectivity_image[reflectivity_index]) * cos(incidence), 0.0);
    thrust::complex<float> beamPattern =
      thrust::complex<float>(azimuthBeamPattern * elevationBeamPattern, 0.0);
    thrust::complex<float> targetArea_sqrt =
      thrust::complex<float>(sqrt(distance * area_scaler), 0.0);
    thrust::complex<float> propagationTerm =
      thrust::complex<float>(1.0 / pow(distance, 2.0) * exp(-2.0 * attenuation * distance), 0.0);
    thrust::complex<float> amplitude = randomAmps * thrust::complex<float>(sourceTerm, 0.0) *
                                       propagationTerm * beamPattern * lambert_sqrt *
                                       targetArea_sqrt;

    // Max distance cut-off
    if (distance > maxDistance)
    {
      amplitude = thrust::complex<float>(0.0, 0.0);
    }

    // Summation of Echo returned from a signal (frequency domain)
    for (size_t f = 0; f < nFreq; f++)
    {
      float freq;
      if (nFreq % 2 == 0)
      {
        freq = __fdividef(delta_f * (-nFreq + 2.0f * (f + 1.0f)), 2.0f);
      }
      else
      {
        freq = __fdividef(delta_f * (-(nFreq - 1) + 2.0f * (f + 1.0f)), 2.0f);
      }

      float kw = __fdividef(2.0f * M_PI * freq, soundSpeed);  // wave vector

      float phase = 2.0f * distance * kw;
      float s, c;
      __sincosf(phase, &s, &c);  // intrinsic sine and cosine

      thrust::complex<float> kernel(c, s);
      kernel *= amplitude;

      int ray_index = ray / raySkips;  // Map ray to reduced ray index

      P_Beams[beam * nFreq * (nRays / raySkips) + ray_index * nFreq + f] = kernel;
    }
  }
}

///////////////////////////////////////////////////////////////////////////
namespace NpsGazeboSonar
{

// CUDA Device Checker Wrapper
void check_cuda_init_wrapper(void)
{
  // Check CUDA device
  cudaDeviceSynchronize();
  cudaError_t error = cudaGetLastError();
  if (error != cudaSuccess)
  {
    fprintf(stderr, "ERROR: %s\n", cudaGetErrorString(error));
    exit(-1);
  }
}

void free_cuda_memory()
{
  SAFE_CALL(cudaFree(d_depth_image), "cudaFree failed for d_depth_image");
  SAFE_CALL(cudaFree(d_normal_image), "cudaFree failed for d_normal_image");
  SAFE_CALL(cudaFree(d_reflectivity_image), "cudaFree failed for d_reflectivity_image");
  SAFE_CALL(cudaFree(d_ray_elevationAngles), "cudaFree failed for d_ray_elevationAngles");
  SAFE_CALL(cudaFree(d_P_Beams), "cudaFree failed for d_P_Beams");
  SAFE_CALL(cudaFree(deviceOutputData), "cudaFree failed for OutputData");
  SAFE_CALL(cudaFree(deviceInputData), "cudaFree failed for InputData");
  SAFE_CALL(cudaFreeHost(P_Beams), "cudaFreeHost failed for P_Beams");
  SAFE_CALL(cudaFree(d_P_Beams_Cor_imag), "cudaFree failed for d_P_Beams_Cor_imag");
  SAFE_CALL(cudaFree(d_P_Beams_Cor_real), "cudaFree failed for d_P_Beams_Cor_real");
  SAFE_CALL(cudaFree(d_P_Beams_Cor_F_imag), "cudaFree failed for d_P_Beams_Cor_F_imag");
  SAFE_CALL(cudaFree(d_P_Beams_Cor_F_real), "cudaFree failed for d_P_Beams_Cor_F_real");
  SAFE_CALL(cudaFree(d_beamCorrector_lin), "cudaFree failed for d_beamCorrector_lin");
  SAFE_CALL(cudaFreeHost(P_Beams_Cor_real_h), "cudaFreeHost failed for P_Beams_Cor_real_h");
  SAFE_CALL(cudaFreeHost(P_Beams_Cor_imag_h), "cudaFreeHost failed for P_Beams_Cor_imag_h");
  SAFE_CALL(cudaFreeHost(beamCorrector_lin_h), "cudaFreeHost failed for beamCorrector_lin_h");
  memory_initialized = false;
}

// Sonar Claculation Function Wrapper
CArray2D sonar_calculation_wrapper(
  const cv::Mat & depth_image, const cv::Mat & normal_image, double _hPixelSize, double _vPixelSize,
  double _hFOV, double _vFOV, double _beam_azimuthAngleWidth, double _beam_elevationAngleWidth,
  double _ray_azimuthAngleWidth, float * _ray_elevationAngles, double _ray_elevationAngleWidth,
  double _soundSpeed, double _maxDistance, double _sourceLevel, int _nBeams, int _nRays,
  int _raySkips, double _sonarFreq, double _bandwidth, int _nFreq,
  const cv::Mat & reflectivity_image, double _attenuation, float * window, float ** beamCorrector,
  float beamCorrectorSum, bool debugFlag, bool blazingFlag)
{
  // Start the timer for the entire function
  auto total_start_time = std::chrono::high_resolution_clock::now();

  auto start = std::chrono::high_resolution_clock::now();
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  if (debugFlag)
  {
    start = std::chrono::high_resolution_clock::now();
  }

  // ----  Allocation of properties parameters  ---- //
  const float hPixelSize = (float)_hPixelSize;
  const float vPixelSize = (float)_vPixelSize;
  const float hFOV = (float)_hFOV;
  const float vFOV = (float)_vFOV;
  const float beam_elevationAngleWidth = (float)_beam_elevationAngleWidth;
  const float beam_azimuthAngleWidth = (float)_beam_azimuthAngleWidth;
  const float ray_elevationAngleWidth = (float)_ray_elevationAngleWidth;
  const float ray_azimuthAngleWidth = (float)_ray_azimuthAngleWidth;
  const float soundSpeed = (float)_soundSpeed;
  const float maxDistance = (float)_maxDistance;
  const float sonarFreq = (float)_sonarFreq;
  const float bandwidth = (float)_bandwidth;
  const float attenuation = (float)_attenuation;
  const int nBeams = _nBeams;
  const int nRays = _nRays;
  const int nFreq = _nFreq;
  const int raySkips = _raySkips;

  // #######################################################//
  // ###############    Sonar Calculation   ################//
  // #######################################################//
  //  ---------   Calculation parameters   --------- //
  const float max_distance = maxDistance;
  // Signal
  const float delta_f = bandwidth / nFreq;
  // Precalculation
  const float area_scaler = ray_azimuthAngleWidth * ray_elevationAngleWidth;
  const float sourceLevel = (float)_sourceLevel;                      // db re 1 muPa;
  const float pref = 1e-6;                                            // 1 micro pascal (muPa);
  const float sourceTerm = sqrt(pow(10, (sourceLevel / 10))) * pref;  // source term

  // ---------   Allocate GPU memory for image   --------- //
  // Calculate total number of bytes of input and output image
  const int depth_image_Bytes = depth_image.step * depth_image.rows;
  const int normal_image_Bytes = normal_image.step * normal_image.rows;
  const int reflectivity_image_Bytes = reflectivity_image.step * reflectivity_image.rows;
  const int ray_elevationAngles_Bytes = sizeof(float) * nRays;
  const int P_Beams_N = nBeams * (int)(nRays / raySkips) * (nFreq + 1);

  P_Beams_Cor_N = nBeams * nFreq;
  P_Beams_Cor_Bytes = sizeof(float) * P_Beams_Cor_N;
  beamCorrector_lin_N = nBeams * nBeams;
  beamCorrector_lin_Bytes = sizeof(float) * beamCorrector_lin_N;

  // FFT Parameters
  const int DATASIZE = nFreq;
  const int BATCH = nBeams;

  if (!memory_initialized)
  {
    SAFE_CALL(
      cudaMalloc((void **)&d_depth_image, depth_image.step * depth_image.rows), "depth malloc");
    SAFE_CALL(
      cudaMallocHost((void **)&ray_elevationAngles, sizeof(float) * nRays),
      "MallocHost ray_elevationAngles");
    SAFE_CALL(
      cudaMalloc((void **)&d_normal_image, normal_image.step * normal_image.rows), "normal malloc");
    SAFE_CALL(
      cudaMalloc((void **)&d_reflectivity_image, reflectivity_image.step * reflectivity_image.rows),
      "reflectivity malloc");
    SAFE_CALL(
      cudaMalloc((void **)&d_ray_elevationAngles, sizeof(float) * nRays), "ray elevation malloc");
    SAFE_CALL(
      cudaMallocHost((void **)&P_Beams, sizeof(thrust::complex<float>) * P_Beams_N),
      "P_Beams malloc host");
    SAFE_CALL(
      cudaMalloc((void **)&d_P_Beams, sizeof(thrust::complex<float>) * P_Beams_N),
      "P_Beams malloc device");
    SAFE_CALL(
      cudaMallocHost((void **)&P_Beams_Cor_real_h, P_Beams_Cor_Bytes),
      "CUDA MallocHost Failed for P_Beams_Cor_real_h");
    SAFE_CALL(
      cudaMallocHost((void **)&P_Beams_Cor_imag_h, P_Beams_Cor_Bytes),
      "CUDA MallocHost Failed for P_Beams_Cor_imag_h");
    SAFE_CALL(
      cudaMallocHost((void **)&beamCorrector_lin_h, beamCorrector_lin_Bytes),
      "CUDA MallocHost Failed for beamCorrector_lin_h");
    SAFE_CALL(
      cudaMalloc((void **)&d_P_Beams_Cor_real, P_Beams_Cor_Bytes),
      "CUDA Malloc Failed for d_P_Beams_Cor_real");
    SAFE_CALL(
      cudaMalloc((void **)&d_P_Beams_Cor_imag, P_Beams_Cor_Bytes),
      "CUDA Malloc Failed for d_P_Beams_Cor_imag");
    SAFE_CALL(
      cudaMalloc((void **)&d_P_Beams_Cor_F_real, P_Beams_Cor_Bytes),
      "CUDA Malloc Failed for d_P_Beams_Cor_F_real");
    SAFE_CALL(
      cudaMalloc((void **)&d_P_Beams_Cor_F_imag, P_Beams_Cor_Bytes),
      "CUDA Malloc Failed for d_P_Beams_Cor_F_imag");
    SAFE_CALL(
      cudaMalloc((void **)&d_beamCorrector_lin, beamCorrector_lin_Bytes),
      "CUDA Malloc Failed for d_beamCorrector_lin");
    SAFE_CALL(
      cudaMalloc((void **)&deviceInputData, DATASIZE * BATCH * sizeof(cufftComplex)),
      "FFT CUDA Malloc Failed");
    SAFE_CALL(
      cudaMalloc((void **)&deviceOutputData, DATASIZE * BATCH * sizeof(cufftComplex)),
      "FFT CUDA Malloc Failed");

    memory_initialized = true;
  }

  for (size_t ray = 0; ray < nRays; ray++)
  {
    ray_elevationAngles[ray] = _ray_elevationAngles[ray];
  }

  // Perform your cudaMemcpy calls
  SAFE_CALL(
    cudaMemcpy(d_depth_image, depth_image.ptr(), depth_image_Bytes, cudaMemcpyHostToDevice),
    "CUDA Memcpy Failed");
  SAFE_CALL(
    cudaMemcpy(d_normal_image, normal_image.ptr(), normal_image_Bytes, cudaMemcpyHostToDevice),
    "CUDA Memcpy Failed");
  SAFE_CALL(
    cudaMemcpy(
      d_reflectivity_image, reflectivity_image.ptr(), reflectivity_image_Bytes,
      cudaMemcpyHostToDevice),
    "CUDA Memcpy Failed");
  SAFE_CALL(
    cudaMemcpy(
      d_ray_elevationAngles, ray_elevationAngles, ray_elevationAngles_Bytes,
      cudaMemcpyHostToDevice),
    "CUDA Memcpy Failed");

  // Specify a reasonable block size
  const dim3 block(BLOCK_SIZE, BLOCK_SIZE);

  // Calculate grid size to cover the whole image
  const dim3 grid(
    (depth_image.cols + block.x - 1) / block.x, (depth_image.rows + block.y - 1) / block.y);

  // Random seed for blazing sonar image
  unsigned long long seed;
  if (blazingFlag)
  {
    seed = static_cast<unsigned long long>(time(NULL));
  }
  else
  {
    seed = 1234;
  }

  // Launch the beamor conversion kernel
  sonar_calculation<<<grid, block>>>(
    d_P_Beams, d_depth_image, d_normal_image, normal_image.cols, normal_image.rows,
    depth_image.step, normal_image.step, seed, d_reflectivity_image, reflectivity_image.step,
    hPixelSize, vPixelSize, hFOV, vFOV, beam_azimuthAngleWidth, beam_elevationAngleWidth,
    ray_azimuthAngleWidth, d_ray_elevationAngles, ray_elevationAngleWidth, soundSpeed, sourceTerm,
    nBeams, nRays, raySkips, sonarFreq, delta_f, nFreq, bandwidth, max_distance, attenuation,
    area_scaler);

  // Synchronize to check for any kernel launch errors
  SAFE_CALL(cudaDeviceSynchronize(), "Kernel Launch Failed");

  if (debugFlag)
  {
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    long long dcount = duration.count();
    float ms = static_cast<float>(dcount) / 1000.0f;

    printf("GPU Sonar Computation Time %lld/100 [s]\n", dcount / 10000);
    printf("GPU Sonar Computation Time: %.3f ms\n", ms);

    debugLog << "GPU Sonar Computation Time " << dcount / 10000 << "/100 [s]\n";
    debugLog << "GPU Sonar Computation Time: " << ms << " ms\n";

    start = std::chrono::high_resolution_clock::now();
  }

  // ########################################################//
  // #########   Summation, Culling and windowing   #########//
  // ########################################################//

  dim3 blockDim(BLOCK_SIZE);
  dim3 gridDim(nFreq, nBeams);  // one thread block per beam+freq pair

  reduce_beams_kernel<<<gridDim, blockDim>>>(
    d_P_Beams, d_P_Beams_Cor_real, d_P_Beams_Cor_imag, nBeams, nFreq, nRays / raySkips);
  SAFE_CALL(cudaGetLastError(), "reduce_beams_kernel launch failed");
  SAFE_CALL(cudaDeviceSynchronize(), "reduce_beams_kernel execution failed");

  if (debugFlag)
  {
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    long long dcount = duration.count();

    printf("Sonar Ray Summation %lld/100 [s]\n", dcount / 10000);
    printf("Sonar Ray Summation Time: %.3f ms\n", static_cast<float>(dcount) / 1000.0f);

    debugLog << "Sonar Ray Summation " << dcount / 10000 << "/100 [s]\n";
    debugLog << "Sonar Ray Summation Time: " << static_cast<float>(dcount) / 1000.0f << " ms\n";

    start = std::chrono::high_resolution_clock::now();
  }

  // --- Beam culling correction ---
  // beamCorrector and beamCorrectorSum is precalculated at parent cpp
  CArray2D P_Beams_F(CArray(nFreq), nBeams);
  cublasHandle_t cublas_handle;

  cublasStatus_t stat = cublasCreate(&cublas_handle);
  if (stat != CUBLAS_STATUS_SUCCESS)
  {
    std::cerr << "cuBLAS create failed with error: " << stat << std::endl;
    return P_Beams_F;
  }

  for (size_t beam = 0; beam < nBeams; beam++)
  {
    for (size_t beam_other = 0; beam_other < nBeams; beam_other++)
    {
      beamCorrector_lin_h[beam_other * nBeams + beam] = beamCorrector[beam][beam_other];
    }
  }

  SAFE_CALL(
    cudaMemcpy(
      d_beamCorrector_lin, beamCorrector_lin_h, beamCorrector_lin_Bytes, cudaMemcpyHostToDevice),
    "CUDA Memcpy Failed (d_beamCorrector_lin)");

  // Define matrix dimensions and GEMM parameters
  // M_gemm = rows of A (P_Beams_Cor_real/imag) = nFreq
  // N_gemm = cols of B (beamCorrector_lin) and cols of C (result) = nBeams
  // K_gemm = cols of A and rows of B = nBeams
  const int M_gemm = nFreq;
  const int N_gemm = nBeams;
  const int K_gemm = nBeams;

  // Alpha and Beta for sgemm (float type)
  const float alpha = 1.0f;
  const float beta = 0.0f;  // Setting beta to 0.0f to overwrite C

  // Real part
  SAFE_CUBLAS_CALL(
    cublasSgemm(
      cublas_handle,
      CUBLAS_OP_N,  // op(A)
      CUBLAS_OP_N,  // op(B)
      N_gemm, M_gemm, K_gemm, &alpha,
      d_beamCorrector_lin,  // Pointer to B_original (now A in cublasSgemm)
      N_gemm,               // lda: leading dimension of B_original
      d_P_Beams_Cor_real,   // Pointer to A_original (now B in cublasSgemm)
      K_gemm,               // ldb: leading dimension of A_original
      &beta,
      d_P_Beams_Cor_F_real,  // Pointer to C_original
      N_gemm),               // ldc: leading dimension of C_original
    "cublasSgemm Failed for real part");

  // Imag part
  SAFE_CUBLAS_CALL(
    cublasSgemm(
      cublas_handle, CUBLAS_OP_N, CUBLAS_OP_N, N_gemm, M_gemm, K_gemm, &alpha, d_beamCorrector_lin,
      N_gemm, d_P_Beams_Cor_imag, K_gemm, &beta, d_P_Beams_Cor_F_imag, N_gemm),
    "cublasSgemm Failed for imag part");

  SAFE_CUBLAS_CALL(cublasDestroy_v2(cublas_handle), "CUBLAS_STATUS_ALLOC_FAILED");

  SAFE_CALL(
    cudaMemcpy(P_Beams_Cor_real_h, d_P_Beams_Cor_F_real, P_Beams_Cor_Bytes, cudaMemcpyDeviceToHost),
    "CUDA Memcpy Failed for P_Beams_Cor_real_h");
  SAFE_CALL(
    cudaMemcpy(P_Beams_Cor_imag_h, d_P_Beams_Cor_F_imag, P_Beams_Cor_Bytes, cudaMemcpyDeviceToHost),
    "CUDA Memcpy Failed for P_Beams_Cor_imag_h");

  if (debugFlag)
  {
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    long long dcount = duration.count();

    printf("GPU Window & Correction %lld/100 [s]\n", dcount / 10000);
    printf("GPU Window & Correction: %.3f ms\n", static_cast<float>(dcount) / 1000.0f);

    // Write to file
    debugLog << "GPU Window & Correction " << dcount / 10000 << "/100 [s]\n";
    debugLog << "GPU Window & Correction: " << static_cast<float>(dcount) / 1000.0f << " ms\n";

    start = std::chrono::high_resolution_clock::now();
  }

  // #################################################//
  // ###################   FFT   #####################//
  // #################################################//
  SAFE_CALL(cudaDeviceSynchronize(), "Kernel Launch Failed");

  // --- Host side input data allocation and initialization
  cufftComplex * hostInputData = (cufftComplex *)malloc(DATASIZE * BATCH * sizeof(cufftComplex));
  start = std::chrono::high_resolution_clock::now();

#pragma omp parallel for collapse(2)
  for (int beam = 0; beam < BATCH; ++beam)
  {
    for (int f = 0; f < DATASIZE; ++f)
    {
      int idx = beam * DATASIZE + f;
      const std::complex<float> & val = P_Beams_F[beam][f];
      hostInputData[idx] = make_cuComplex(
        P_Beams_Cor_real_h[f * nBeams + beam] / beamCorrectorSum,
        P_Beams_Cor_imag_h[f * nBeams + beam] / beamCorrectorSum);
    }
  }

  // --- Device side input data allocation and initialization
  SAFE_CALL(
    cudaMemcpy(
      deviceInputData, hostInputData, DATASIZE * BATCH * sizeof(cufftComplex),
      cudaMemcpyHostToDevice),
    "FFT CUDA Memcopy Failed");

  // --- Host side output data allocation
  cufftComplex * hostOutputData = (cufftComplex *)malloc(DATASIZE * BATCH * sizeof(cufftComplex));

  // --- Batched 1D FFTs
  cufftHandle handle;
  int rank = 1;          // --- 1D FFTs
  int n[] = {DATASIZE};  // --- Size of the Fourier transform
  // --- Distance between two successive input/output elements
  int istride = 1, ostride = 1;
  int idist = DATASIZE, odist = DATASIZE;  // --- Distance between batches
  // --- Input/Output size with pitch (ignored for 1D transforms)
  int inembed[] = {0};
  int onembed[] = {0};
  int batch = BATCH;  // --- Number of batched executions
  cufftPlanMany(
    &handle, rank, n, inembed, istride, idist, onembed, ostride, odist, CUFFT_C2C, batch);

  cufftExecC2C(handle, deviceInputData, deviceOutputData, CUFFT_FORWARD);

  // --- Device->Host copy of the results
  SAFE_CALL(
    cudaMemcpy(
      hostOutputData, deviceOutputData, DATASIZE * BATCH * sizeof(cufftComplex),
      cudaMemcpyDeviceToHost),
    "FFT CUDA Memcopy Failed");

  for (int beam = 0; beam < BATCH; beam++)
  {
    for (int f = 0; f < nFreq; f++)
    {
      P_Beams_F[beam][f] = Complex(
        hostOutputData[beam * DATASIZE + f].x * delta_f,
        hostOutputData[beam * DATASIZE + f].y * delta_f);
    }
  }

  cufftDestroy(handle);
  free(hostInputData);
  free(hostOutputData);

  if (debugFlag)
  {
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    long long dcount = duration.count();
    float ms = static_cast<float>(dcount) / 1000.0f;

    printf("GPU FFT Calc Time: %.3f ms\n", ms);
    printf("GPU FFT Calc Time %lld/100 [s]\n", dcount / 10000);

    // Write to file
    debugLog << "GPU FFT Calc Time: " << ms << " ms\n";
    debugLog << "GPU FFT Calc Time " << dcount / 10000 << "/100 [s]\n";

    start = std::chrono::high_resolution_clock::now();
  }

  auto total_stop_time = std::chrono::high_resolution_clock::now();
  auto total_duration =
    std::chrono::duration_cast<std::chrono::microseconds>(total_stop_time - total_start_time);

  if (debugFlag)
  {
    float ms = static_cast<float>(total_duration.count()) / 1000.0f;

    printf("Total Sonar Calculation Wrapper Time: %.3f ms\n", ms);

    debugLog << "Total Sonar Calculation Wrapper Time: " << ms << " ms\n";
  }

  return P_Beams_F;
}
}  // namespace NpsGazeboSonar