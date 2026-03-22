#ifndef SONAR_WGPU_H
#define SONAR_WGPU_H

/* Generated via cbindgen — interface between C++ plugin and Rust backend */

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /* Opaque handle to Rust-side engine */
  typedef struct SonarPhysicsEngine SonarPhysicsEngine;

  /* Create GPU sonar engine.
     Returns NULL if GPU init fails (caller should fallback to CPU). */
  SonarPhysicsEngine * sonar_wgpu_create(
    uint32_t n_beams, uint32_t n_rays, uint32_t n_freq, float sound_speed, float bandwidth,
    float max_range, float attenuation, float source_level, float sensor_gain, float h_fov,
    float v_fov, uint64_t seed);

  /* Run one frame.
     Output layout: [re0, im0, re1, im1, ...] (beam-major).
     Size = n_beams * n_freq * 2.
     Caller owns memory → free with sonar_wgpu_free(). */
  float * sonar_wgpu_compute(
    SonarPhysicsEngine * engine, const float * depth, const float * normals, const float * refl,
    const float * beam_corr, uint32_t n_beams, uint32_t n_rays, uint32_t n_freq, uint64_t frame,
    float beam_corr_sum);

  /* Free buffer returned by compute */
  void sonar_wgpu_free(float * ptr, uintptr_t len);

  /* Destroy engine + release GPU resources */
  void sonar_wgpu_destroy(void * engine);

  /* --- CPU / legacy path (kept for compatibility) --- */

  SonarPhysicsEngine * sonar_physics_create(
    uint32_t n_beams, uint32_t n_rays, uint32_t n_freq, float sound_speed, float bandwidth,
    float max_range, float attenuation, float h_fov, float v_fov);

  void sonar_physics_destroy(SonarPhysicsEngine * engine);

#ifdef __cplusplus
}
#endif

#endif /* SONAR_WGPU_H */