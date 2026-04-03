// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Header-only Q15/Q31 fixed-point arithmetic for eNI
// No heap allocation, no external dependencies

#ifndef ENI_FIXED_POINT_H
#define ENI_FIXED_POINT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Q15 format: 1 sign bit + 15 fractional bits ─────────────────── */
/* Range: [-1.0, 1.0 - 2^-15] ≈ [-1.0, 0.999969] */

typedef int16_t q15_t;
typedef int32_t q31_t;

#define Q15_FRAC_BITS  15
#define Q15_ONE        ((q15_t)0x7FFF)
#define Q15_MINUS_ONE  ((q15_t)0x8001)
#define Q15_ZERO       ((q15_t)0)
#define Q15_HALF       ((q15_t)0x4000)
#define Q15_MAX        ((q15_t)0x7FFF)
#define Q15_MIN        ((q15_t)0x8000)

#define Q31_FRAC_BITS  31
#define Q31_ONE        ((q31_t)0x7FFFFFFF)
#define Q31_MINUS_ONE  ((q31_t)0x80000001)
#define Q31_ZERO       ((q31_t)0)
#define Q31_MAX        ((q31_t)0x7FFFFFFF)
#define Q31_MIN        ((q31_t)0x80000000)

/* ── Q15 conversion ──────────────────────────────────────────────── */

static inline q15_t eni_float_to_q15(float f) {
    if (f >= 1.0f) return Q15_MAX;
    if (f <= -1.0f) return Q15_MIN;
    return (q15_t)(f * 32768.0f);
}

static inline float eni_q15_to_float(q15_t q) {
    return (float)q / 32768.0f;
}

static inline q15_t eni_int16_to_q15(int16_t x, int frac_bits) {
    int shift = Q15_FRAC_BITS - frac_bits;
    if (shift >= 0) return (q15_t)((int32_t)x << shift);
    return (q15_t)(x >> (-shift));
}

/* ── Q15 arithmetic ──────────────────────────────────────────────── */

static inline q15_t eni_q15_add(q15_t a, q15_t b) {
    int32_t sum = (int32_t)a + (int32_t)b;
    if (sum > Q15_MAX) return Q15_MAX;
    if (sum < Q15_MIN) return Q15_MIN;
    return (q15_t)sum;
}

static inline q15_t eni_q15_sub(q15_t a, q15_t b) {
    int32_t diff = (int32_t)a - (int32_t)b;
    if (diff > Q15_MAX) return Q15_MAX;
    if (diff < Q15_MIN) return Q15_MIN;
    return (q15_t)diff;
}

static inline q15_t eni_q15_mul(q15_t a, q15_t b) {
    int32_t product = ((int32_t)a * (int32_t)b) >> Q15_FRAC_BITS;
    if (product > Q15_MAX) return Q15_MAX;
    if (product < Q15_MIN) return Q15_MIN;
    return (q15_t)product;
}

static inline q15_t eni_q15_mul_round(q15_t a, q15_t b) {
    int32_t product = (int32_t)a * (int32_t)b;
    product += (1 << (Q15_FRAC_BITS - 1)); /* rounding */
    product >>= Q15_FRAC_BITS;
    if (product > Q15_MAX) return Q15_MAX;
    if (product < Q15_MIN) return Q15_MIN;
    return (q15_t)product;
}

static inline q15_t eni_q15_abs(q15_t x) {
    if (x == Q15_MIN) return Q15_MAX;
    return x < 0 ? -x : x;
}

static inline q15_t eni_q15_neg(q15_t x) {
    if (x == Q15_MIN) return Q15_MAX;
    return -x;
}

static inline q15_t eni_q15_shift_left(q15_t x, int shift) {
    int32_t result = (int32_t)x << shift;
    if (result > Q15_MAX) return Q15_MAX;
    if (result < Q15_MIN) return Q15_MIN;
    return (q15_t)result;
}

static inline q15_t eni_q15_shift_right(q15_t x, int shift) {
    return (q15_t)(x >> shift);
}

/* ── Q15 vector operations ───────────────────────────────────────── */

static inline void eni_q15_vec_add(const q15_t *a, const q15_t *b,
                                    q15_t *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = eni_q15_add(a[i], b[i]);
}

static inline void eni_q15_vec_sub(const q15_t *a, const q15_t *b,
                                    q15_t *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = eni_q15_sub(a[i], b[i]);
}

static inline void eni_q15_vec_mul(const q15_t *a, const q15_t *b,
                                    q15_t *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = eni_q15_mul(a[i], b[i]);
}

static inline void eni_q15_vec_scale(const q15_t *a, q15_t s,
                                      q15_t *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = eni_q15_mul(a[i], s);
}

static inline q31_t eni_q15_dot(const q15_t *a, const q15_t *b, int n) {
    int64_t acc = 0;
    for (int i = 0; i < n; ++i) acc += (int32_t)a[i] * (int32_t)b[i];
    acc >>= Q15_FRAC_BITS;
    if (acc > Q31_MAX) return Q31_MAX;
    if (acc < Q31_MIN) return Q31_MIN;
    return (q31_t)acc;
}

static inline q15_t eni_q15_vec_max(const q15_t *a, int n) {
    if (n <= 0) return Q15_MIN;
    q15_t m = a[0];
    for (int i = 1; i < n; ++i) if (a[i] > m) m = a[i];
    return m;
}

static inline q15_t eni_q15_vec_min(const q15_t *a, int n) {
    if (n <= 0) return Q15_MAX;
    q15_t m = a[0];
    for (int i = 1; i < n; ++i) if (a[i] < m) m = a[i];
    return m;
}

/* ── Q31 conversion ──────────────────────────────────────────────── */

static inline q31_t eni_float_to_q31(float f) {
    if (f >= 1.0f) return Q31_MAX;
    if (f <= -1.0f) return Q31_MIN;
    return (q31_t)(f * 2147483648.0f);
}

static inline float eni_q31_to_float(q31_t q) {
    return (float)q / 2147483648.0f;
}

static inline q15_t eni_q31_to_q15(q31_t q) {
    int32_t shifted = q >> (Q31_FRAC_BITS - Q15_FRAC_BITS);
    if (shifted > Q15_MAX) return Q15_MAX;
    if (shifted < Q15_MIN) return Q15_MIN;
    return (q15_t)shifted;
}

static inline q31_t eni_q15_to_q31(q15_t q) {
    return (q31_t)q << (Q31_FRAC_BITS - Q15_FRAC_BITS);
}

/* ── Q31 arithmetic ──────────────────────────────────────────────── */

static inline q31_t eni_q31_add(q31_t a, q31_t b) {
    int64_t sum = (int64_t)a + (int64_t)b;
    if (sum > Q31_MAX) return Q31_MAX;
    if (sum < Q31_MIN) return Q31_MIN;
    return (q31_t)sum;
}

static inline q31_t eni_q31_sub(q31_t a, q31_t b) {
    int64_t diff = (int64_t)a - (int64_t)b;
    if (diff > Q31_MAX) return Q31_MAX;
    if (diff < Q31_MIN) return Q31_MIN;
    return (q31_t)diff;
}

static inline q31_t eni_q31_mul(q31_t a, q31_t b) {
    int64_t product = ((int64_t)a * (int64_t)b) >> Q31_FRAC_BITS;
    if (product > Q31_MAX) return Q31_MAX;
    if (product < Q31_MIN) return Q31_MIN;
    return (q31_t)product;
}

static inline q31_t eni_q31_abs(q31_t x) {
    if (x == Q31_MIN) return Q31_MAX;
    return x < 0 ? -x : x;
}

/* ── Q31 vector operations ───────────────────────────────────────── */

static inline void eni_q31_vec_add(const q31_t *a, const q31_t *b,
                                    q31_t *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = eni_q31_add(a[i], b[i]);
}

static inline void eni_q31_vec_mul(const q31_t *a, const q31_t *b,
                                    q31_t *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = eni_q31_mul(a[i], b[i]);
}

/* ── Q15 MAC (multiply-accumulate for FIR filters) ────────────────── */

static inline q31_t eni_q15_mac(const q15_t *input, const q15_t *coeffs, int n) {
    int64_t acc = 0;
    for (int i = 0; i < n; ++i) {
        acc += (int32_t)input[i] * (int32_t)coeffs[i];
    }
    acc >>= Q15_FRAC_BITS;
    if (acc > Q31_MAX) return Q31_MAX;
    if (acc < Q31_MIN) return Q31_MIN;
    return (q31_t)acc;
}

/* ── Bulk float↔Q15 conversion ────────────────────────────────────── */

static inline void eni_float_array_to_q15(const float *in, q15_t *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = eni_float_to_q15(in[i]);
}

static inline void eni_q15_array_to_float(const q15_t *in, float *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = eni_q15_to_float(in[i]);
}

#ifdef __cplusplus
}
#endif

#endif /* ENI_FIXED_POINT_H */
