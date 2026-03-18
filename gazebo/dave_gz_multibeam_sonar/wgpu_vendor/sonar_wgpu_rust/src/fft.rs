// CPU FFT using rustfft for both power-of-two and arbitrary lengths.

use crate::ComplexF;
use rayon::prelude::*;
use rustfft::{num_complex::Complex, FftPlanner};

/// Batched FFT preserving the existing public API used by lib.rs.
pub fn fft_batched(input: &[ComplexF], n_beams: usize, n_freq: usize) -> Vec<ComplexF> {
    let mut rows: Vec<Vec<ComplexF>> = (0..n_beams)
        .map(|b| input[b * n_freq..(b + 1) * n_freq].to_vec())
        .collect();

    let mut planner = FftPlanner::<f32>::new();
    let fft = planner.plan_fft_forward(n_freq);

    rows.par_iter_mut().for_each(|row| {
        let mut rustfft_row: Vec<Complex<f32>> = row
            .iter()
            .map(|x| Complex::<f32> { re: x.re, im: x.im })
            .collect();

        fft.process(&mut rustfft_row);

        for (dst, src) in row.iter_mut().zip(rustfft_row.iter()) {
            dst.re = src.re;
            dst.im = src.im;
        }
    });

    rows.into_iter().flatten().collect()
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ComplexF;

    /// Reference O(N²) DFT used only in tests for correctness comparison.
    fn naive_dft(data: &mut [ComplexF]) {
        let n = data.len();
        let input: Vec<ComplexF> = data.to_vec();
        for k in 0..n {
            let mut re_sum = 0.0f32;
            let mut im_sum = 0.0f32;
            for j in 0..n {
                let angle = -2.0 * std::f32::consts::PI * (k as f32) * (j as f32) / (n as f32);
                re_sum += input[j].re * angle.cos() - input[j].im * angle.sin();
                im_sum += input[j].re * angle.sin() + input[j].im * angle.cos();
            }
            data[k] = ComplexF { re: re_sum, im: im_sum };
        }
    }

    fn nearly_eq(a: f32, b: f32) -> bool {
        (a - b).abs() < 1e-3
    }

    fn nearly_eq_rel(a: f32, b: f32) -> bool {
        let mag = b.abs().max(1e-6);
        (a - b).abs() / mag < 1e-2 // 1% relative tolerance for f32 accumulation over N=399
    }

    #[test]
    fn rustfft_matches_naive_dft_n3() {
        let x = vec![
            ComplexF { re: 1.0, im: 0.0 },
            ComplexF { re: 2.0, im: -1.0 },
            ComplexF { re: 0.5, im: 3.0 },
        ];
        let x_b = fft_batched(&x, 1, 3);
        let mut x_d = x.clone();
        naive_dft(&mut x_d);
        for (b, d) in x_b.iter().zip(x_d.iter()) {
            assert!(nearly_eq(b.re, d.re), "re: {} vs {}", b.re, d.re);
            assert!(nearly_eq(b.im, d.im), "im: {} vs {}", b.im, d.im);
        }
    }

    #[test]
    fn rustfft_matches_naive_dft_n399() {
        use std::f32::consts::PI;
        let n = 399usize;
        // Construct a chirp-like test signal
        let input: Vec<ComplexF> = (0..n)
            .map(|k| ComplexF {
                re: ((k as f32) * PI / 50.0).cos(),
                im: ((k as f32) * PI / 75.0).sin(),
            })
            .collect();
        let x_b = fft_batched(&input, 1, n);
        let mut x_d = input.clone();
        naive_dft(&mut x_d);
        // Compare first 5 and last 5 bins
        for &i in &[0, 1, 2, 3, 4, 394, 395, 396, 397, 398] {
            assert!(nearly_eq_rel(x_b[i].re, x_d[i].re),
                "bin {i} re: {} vs {}", x_b[i].re, x_d[i].re);
            assert!(nearly_eq_rel(x_b[i].im, x_d[i].im),
                "bin {i} im: {} vs {}", x_b[i].im, x_d[i].im);
        }
    }

    #[test]
    fn rustfft_impulse_gives_constant_spectrum() {
        // DFT of [1, 0, 0, ..., 0] = [1, 1, 1, ..., 1]
        let n = 399usize;
        let mut x = vec![ComplexF::default(); n];
        x[0].re = 1.0;
        let x = fft_batched(&x, 1, n);
        for (i, v) in x.iter().enumerate() {
            assert!(nearly_eq(v.re, 1.0), "bin {i} re = {} (expected 1)", v.re);
            assert!(nearly_eq(v.im, 0.0), "bin {i} im = {} (expected 0)", v.im);
        }
    }
}
