/**
 * @file adaptive_filter.c
 * @brief Adaptive filter implementations: LMS, NLMS, RLS.
 *
 * All filters use fixed-size arrays — no heap allocation.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/adaptive_filter.h"
#include <string.h>
#include <math.h>

/* ========================================================================= */
/*  LMS                                                                      */
/* ========================================================================= */

int eni_lms_init(eni_lms_filter_t *f, uint32_t order, float step_size)
{
    if (!f) return -1;
    if (order == 0 || order > ENI_FILTER_MAX_ORDER) return -2;
    if (step_size <= 0.0f) return -2;

    memset(f, 0, sizeof(*f));
    f->order     = order;
    f->step_size = step_size;
    f->initialised = true;
    return 0;
}

float eni_lms_update(eni_lms_filter_t *f, float input, float desired)
{
    if (!f || !f->initialised) return 0.0f;

    uint32_t N = f->order;

    /* Shift new sample into delay line */
    f->x[f->idx] = input;

    /* Compute filter output y = w^T * x */
    float y = 0.0f;
    for (uint32_t i = 0; i < N; i++) {
        uint32_t j = (f->idx + ENI_FILTER_MAX_ORDER - i) % ENI_FILTER_MAX_ORDER;
        y += f->w[i] * f->x[j];
    }

    /* Error */
    float e = desired - y;

    /* Update coefficients: w += mu * e * x */
    float mu = f->step_size;
    for (uint32_t i = 0; i < N; i++) {
        uint32_t j = (f->idx + ENI_FILTER_MAX_ORDER - i) % ENI_FILTER_MAX_ORDER;
        f->w[i] += mu * e * f->x[j];
    }

    f->idx = (f->idx + 1) % ENI_FILTER_MAX_ORDER;
    return e;
}

int eni_lms_filter_block(eni_lms_filter_t *f,
                         const float *input,
                         const float *desired,
                         float *output,
                         float *error,
                         uint32_t len)
{
    if (!f || !input || !desired) return -1;
    if (!f->initialised) return -3;

    for (uint32_t n = 0; n < len; n++) {
        float e = eni_lms_update(f, input[n], desired[n]);
        if (error)  error[n]  = e;
        if (output) output[n] = desired[n] - e;
    }
    return 0;
}

/* ========================================================================= */
/*  NLMS                                                                     */
/* ========================================================================= */

int eni_nlms_init(eni_nlms_filter_t *f, uint32_t order,
                  float step_size, float eps)
{
    if (!f) return -1;
    if (order == 0 || order > ENI_FILTER_MAX_ORDER) return -2;
    if (step_size <= 0.0f) return -2;

    memset(f, 0, sizeof(*f));
    f->order     = order;
    f->step_size = step_size;
    f->eps       = (eps > 0.0f) ? eps : 1e-6f;
    f->initialised = true;
    return 0;
}

float eni_nlms_update(eni_nlms_filter_t *f, float input, float desired)
{
    if (!f || !f->initialised) return 0.0f;

    uint32_t N = f->order;
    f->x[f->idx] = input;

    /* Output */
    float y = 0.0f;
    float x_norm = 0.0f;
    for (uint32_t i = 0; i < N; i++) {
        uint32_t j = (f->idx + ENI_FILTER_MAX_ORDER - i) % ENI_FILTER_MAX_ORDER;
        float xi = f->x[j];
        y += f->w[i] * xi;
        x_norm += xi * xi;
    }

    float e = desired - y;

    /* Normalised step */
    float mu = f->step_size / (x_norm + f->eps);

    for (uint32_t i = 0; i < N; i++) {
        uint32_t j = (f->idx + ENI_FILTER_MAX_ORDER - i) % ENI_FILTER_MAX_ORDER;
        f->w[i] += mu * e * f->x[j];
    }

    f->idx = (f->idx + 1) % ENI_FILTER_MAX_ORDER;
    return e;
}

/* ========================================================================= */
/*  RLS                                                                      */
/* ========================================================================= */

int eni_rls_init(eni_rls_filter_t *f, uint32_t order,
                 float lambda, float delta)
{
    if (!f) return -1;
    if (order == 0 || order > ENI_RLS_MAX_ORDER) return -2;
    if (lambda <= 0.0f || lambda > 1.0f) return -2;
    if (delta <= 0.0f) return -2;

    memset(f, 0, sizeof(*f));
    f->order  = order;
    f->lambda = lambda;
    f->delta  = delta;

    /* Initialise P = (1/delta) * I */
    float inv_delta = 1.0f / delta;
    for (uint32_t i = 0; i < order; i++) {
        f->P[i][i] = inv_delta;
    }

    f->initialised = true;
    return 0;
}

float eni_rls_update(eni_rls_filter_t *f, float input, float desired)
{
    if (!f || !f->initialised) return 0.0f;

    uint32_t N = f->order;
    float inv_lambda = 1.0f / f->lambda;

    /* Shift input into delay line */
    f->x[f->idx] = input;

    /* Build x vector (most recent N samples) */
    float xv[ENI_RLS_MAX_ORDER];
    for (uint32_t i = 0; i < N; i++) {
        uint32_t j = (f->idx + ENI_RLS_MAX_ORDER - i) % ENI_RLS_MAX_ORDER;
        xv[i] = f->x[j];
    }

    /* k = P * x */
    float Px[ENI_RLS_MAX_ORDER];
    for (uint32_t i = 0; i < N; i++) {
        Px[i] = 0.0f;
        for (uint32_t j = 0; j < N; j++) {
            Px[i] += f->P[i][j] * xv[j];
        }
    }

    /* denominator = lambda + x^T * P * x */
    float denom = f->lambda;
    for (uint32_t i = 0; i < N; i++) {
        denom += xv[i] * Px[i];
    }

    /* Gain vector k = Px / denom */
    float k[ENI_RLS_MAX_ORDER];
    float inv_denom = 1.0f / (denom + 1e-12f);
    for (uint32_t i = 0; i < N; i++) {
        k[i] = Px[i] * inv_denom;
    }

    /* A priori error */
    float y = 0.0f;
    for (uint32_t i = 0; i < N; i++) {
        y += f->w[i] * xv[i];
    }
    float e = desired - y;

    /* Update weights: w += k * e */
    for (uint32_t i = 0; i < N; i++) {
        f->w[i] += k[i] * e;
    }

    /* Update P = (1/lambda) * (P - k * x^T * P) */
    /* Temp = k * Px^T  (outer product k * (x^T * P) = k * Px^T) */
    for (uint32_t i = 0; i < N; i++) {
        for (uint32_t j = 0; j < N; j++) {
            f->P[i][j] = inv_lambda * (f->P[i][j] - k[i] * Px[j]);
        }
    }

    f->idx = (f->idx + 1) % ENI_RLS_MAX_ORDER;
    return e;
}
