// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Header-only DSP math operations for eNI
// No heap allocation, no external dependencies beyond <math.h>

#ifndef ENI_MATH_OPS_H
#define ENI_MATH_OPS_H

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ENI_MATH_PI
#define ENI_MATH_PI 3.14159265358979323846f
#endif

#ifndef ENI_MATH_TWO_PI
#define ENI_MATH_TWO_PI 6.28318530717958647692f
#endif

#ifndef ENI_MATH_EPSILON
#define ENI_MATH_EPSILON 1e-7f
#endif

/* ── Scalar utilities ──────────────────────────────────────────────── */

static inline float eni_math_clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static inline float eni_math_absf(float x) {
    return x < 0.0f ? -x : x;
}

static inline float eni_math_maxf(float a, float b) {
    return a > b ? a : b;
}

static inline float eni_math_minf(float a, float b) {
    return a < b ? a : b;
}

static inline float eni_math_lerp(float a, float b, float t) {
    return a + t * (b - a);
}

static inline int eni_math_is_power_of_two(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

static inline int eni_math_next_power_of_two(int n) {
    if (n <= 1) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

/* ── Vector operations (no heap) ───────────────────────────────────── */

static inline void eni_math_vec_add(const float *a, const float *b,
                                     float *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = a[i] + b[i];
}

static inline void eni_math_vec_sub(const float *a, const float *b,
                                     float *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = a[i] - b[i];
}

static inline void eni_math_vec_mul(const float *a, const float *b,
                                     float *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = a[i] * b[i];
}

static inline void eni_math_vec_scale(const float *a, float s,
                                       float *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = a[i] * s;
}

static inline float eni_math_vec_dot(const float *a, const float *b, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) sum += a[i] * b[i];
    return sum;
}

static inline float eni_math_vec_sum(const float *a, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) sum += a[i];
    return sum;
}

static inline float eni_math_vec_mean(const float *a, int n) {
    if (n <= 0) return 0.0f;
    return eni_math_vec_sum(a, n) / (float)n;
}

static inline float eni_math_vec_max(const float *a, int n) {
    if (n <= 0) return -FLT_MAX;
    float m = a[0];
    for (int i = 1; i < n; ++i) if (a[i] > m) m = a[i];
    return m;
}

static inline float eni_math_vec_min(const float *a, int n) {
    if (n <= 0) return FLT_MAX;
    float m = a[0];
    for (int i = 1; i < n; ++i) if (a[i] < m) m = a[i];
    return m;
}

static inline int eni_math_vec_argmax(const float *a, int n) {
    if (n <= 0) return -1;
    int idx = 0;
    for (int i = 1; i < n; ++i) if (a[i] > a[idx]) idx = i;
    return idx;
}

static inline float eni_math_vec_norm2(const float *a, int n) {
    return sqrtf(eni_math_vec_dot(a, a, n));
}

static inline float eni_math_vec_variance(const float *a, int n) {
    if (n <= 1) return 0.0f;
    float mean = eni_math_vec_mean(a, n);
    float var = 0.0f;
    for (int i = 0; i < n; ++i) {
        float d = a[i] - mean;
        var += d * d;
    }
    return var / (float)(n - 1);
}

static inline float eni_math_vec_stddev(const float *a, int n) {
    return sqrtf(eni_math_vec_variance(a, n));
}

static inline float eni_math_vec_rms(const float *a, int n) {
    if (n <= 0) return 0.0f;
    float sum_sq = 0.0f;
    for (int i = 0; i < n; ++i) sum_sq += a[i] * a[i];
    return sqrtf(sum_sq / (float)n);
}

/* ── Activation functions (used in NN inference) ──────────────────── */

static inline float eni_math_relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

static inline float eni_math_sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

static inline float eni_math_tanh_approx(float x) {
    return tanhf(x);
}

static inline void eni_math_softmax(const float *input, float *output, int n) {
    float max_val = input[0];
    for (int i = 1; i < n; ++i) if (input[i] > max_val) max_val = input[i];

    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        output[i] = expf(input[i] - max_val);
        sum += output[i];
    }
    if (sum > ENI_MATH_EPSILON) {
        for (int i = 0; i < n; ++i) output[i] /= sum;
    }
}

/* ── DSP window functions ─────────────────────────────────────────── */

static inline void eni_math_window_hann(float *w, int n) {
    for (int i = 0; i < n; ++i)
        w[i] = 0.5f * (1.0f - cosf(ENI_MATH_TWO_PI * (float)i / (float)(n - 1)));
}

static inline void eni_math_window_hamming(float *w, int n) {
    for (int i = 0; i < n; ++i)
        w[i] = 0.54f - 0.46f * cosf(ENI_MATH_TWO_PI * (float)i / (float)(n - 1));
}

static inline void eni_math_window_blackman(float *w, int n) {
    for (int i = 0; i < n; ++i) {
        float t = ENI_MATH_TWO_PI * (float)i / (float)(n - 1);
        w[i] = 0.42f - 0.5f * cosf(t) + 0.08f * cosf(2.0f * t);
    }
}

static inline void eni_math_window_apply(float *signal, const float *window, int n) {
    for (int i = 0; i < n; ++i) signal[i] *= window[i];
}

/* ── Matrix-vector multiply (for dense NN layers) ─────────────────── */

static inline void eni_math_matvec(const float *mat, const float *vec,
                                    float *out, int rows, int cols) {
    for (int r = 0; r < rows; ++r) {
        float sum = 0.0f;
        for (int c = 0; c < cols; ++c)
            sum += mat[r * cols + c] * vec[c];
        out[r] = sum;
    }
}

static inline void eni_math_vec_add_bias(float *vec, const float *bias, int n) {
    for (int i = 0; i < n; ++i) vec[i] += bias[i];
}

/* ── 1D Convolution (for Conv1D NN layers) ────────────────────────── */

static inline void eni_math_conv1d(const float *input, int input_len,
                                    const float *kernel, int kernel_len,
                                    float *output, int output_len) {
    for (int i = 0; i < output_len; ++i) {
        float sum = 0.0f;
        for (int k = 0; k < kernel_len; ++k) {
            int idx = i + k;
            if (idx < input_len) sum += input[idx] * kernel[k];
        }
        output[i] = sum;
    }
}

/* ── Normalization ────────────────────────────────────────────────── */

static inline void eni_math_normalize_minmax(const float *in, float *out, int n) {
    float mn = eni_math_vec_min(in, n);
    float mx = eni_math_vec_max(in, n);
    float range = mx - mn;
    if (range < ENI_MATH_EPSILON) {
        for (int i = 0; i < n; ++i) out[i] = 0.0f;
        return;
    }
    for (int i = 0; i < n; ++i) out[i] = (in[i] - mn) / range;
}

static inline void eni_math_normalize_zscore(const float *in, float *out, int n) {
    float mean = eni_math_vec_mean(in, n);
    float sd = eni_math_vec_stddev(in, n);
    if (sd < ENI_MATH_EPSILON) {
        for (int i = 0; i < n; ++i) out[i] = 0.0f;
        return;
    }
    for (int i = 0; i < n; ++i) out[i] = (in[i] - mean) / sd;
}

/* ── Spectral entropy ─────────────────────────────────────────────── */

static inline float eni_math_spectral_entropy(const float *psd_bins, int n) {
    float total = 0.0f;
    for (int i = 0; i < n; ++i) total += psd_bins[i];
    if (total < ENI_MATH_EPSILON) return 0.0f;

    float entropy = 0.0f;
    for (int i = 0; i < n; ++i) {
        float p = psd_bins[i] / total;
        if (p > ENI_MATH_EPSILON) entropy -= p * log2f(p);
    }
    return entropy;
}

/* ── Hjorth parameters ────────────────────────────────────────────── */

static inline float eni_math_hjorth_activity(const float *signal, int n) {
    return eni_math_vec_variance(signal, n);
}

static inline float eni_math_hjorth_mobility(const float *signal, int n) {
    if (n <= 2) return 0.0f;
    float var_signal = eni_math_vec_variance(signal, n);
    if (var_signal < ENI_MATH_EPSILON) return 0.0f;

    float diff[512]; /* stack-allocated, bounded by ENI_DSP_MAX_FFT_SIZE */
    int dn = (n - 1 > 512) ? 512 : n - 1;
    for (int i = 0; i < dn; ++i) diff[i] = signal[i + 1] - signal[i];

    float var_diff = eni_math_vec_variance(diff, dn);
    return sqrtf(var_diff / var_signal);
}

static inline float eni_math_hjorth_complexity(const float *signal, int n) {
    if (n <= 3) return 0.0f;
    float mob_signal = eni_math_hjorth_mobility(signal, n);
    if (mob_signal < ENI_MATH_EPSILON) return 0.0f;

    float diff[512];
    int dn = (n - 1 > 512) ? 512 : n - 1;
    for (int i = 0; i < dn; ++i) diff[i] = signal[i + 1] - signal[i];

    float mob_diff = eni_math_hjorth_mobility(diff, dn);
    return mob_diff / mob_signal;
}

/* ── Decibel conversion ───────────────────────────────────────────── */

static inline float eni_math_power_to_db(float power) {
    if (power < ENI_MATH_EPSILON) return -120.0f;
    return 10.0f * log10f(power);
}

static inline float eni_math_db_to_power(float db) {
    return powf(10.0f, db / 10.0f);
}

/* ── Moving average (circular buffer style) ───────────────────────── */

static inline float eni_math_moving_average(const float *buf, int len, int start, int count) {
    if (count <= 0) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < count; ++i) {
        sum += buf[(start + i) % len];
    }
    return sum / (float)count;
}

/* ── Cross-correlation (lag 0..max_lag) ───────────────────────────── */

static inline float eni_math_cross_correlation(const float *x, const float *y,
                                                int n, int lag) {
    if (lag < 0 || lag >= n) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < n - lag; ++i) {
        sum += x[i] * y[i + lag];
    }
    return sum / (float)(n - lag);
}

#ifdef __cplusplus
}
#endif

#endif /* ENI_MATH_OPS_H */
