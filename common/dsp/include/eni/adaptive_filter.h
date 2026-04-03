/**
 * @file adaptive_filter.h
 * @brief Adaptive filtering: LMS, NLMS, and RLS algorithms.
 *
 * Fixed-size coefficient arrays — MCU-safe, no dynamic allocation.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_ADAPTIVE_FILTER_H
#define ENI_ADAPTIVE_FILTER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */
#define ENI_FILTER_MAX_ORDER    256
#define ENI_RLS_MAX_ORDER        64  /**< Smaller for P-matrix memory. */

/* ---------------------------------------------------------------------------
 * LMS Filter
 * ------------------------------------------------------------------------ */
typedef struct {
    float    w[ENI_FILTER_MAX_ORDER];   /**< Filter coefficients. */
    float    x[ENI_FILTER_MAX_ORDER];   /**< Input delay line. */
    float    step_size;                 /**< µ: adaptation step. */
    uint32_t order;                     /**< Filter order (taps). */
    uint32_t idx;                       /**< Circular index. */
    bool     initialised;
} eni_lms_filter_t;

int   eni_lms_init(eni_lms_filter_t *f, uint32_t order, float step_size);
float eni_lms_update(eni_lms_filter_t *f, float input, float desired);
int   eni_lms_filter_block(eni_lms_filter_t *f,
                           const float *input,
                           const float *desired,
                           float *output,
                           float *error,
                           uint32_t len);

/* ---------------------------------------------------------------------------
 * NLMS Filter
 * ------------------------------------------------------------------------ */
typedef struct {
    float    w[ENI_FILTER_MAX_ORDER];
    float    x[ENI_FILTER_MAX_ORDER];
    float    step_size;
    float    eps;                       /**< Regularisation constant. */
    uint32_t order;
    uint32_t idx;
    bool     initialised;
} eni_nlms_filter_t;

int   eni_nlms_init(eni_nlms_filter_t *f, uint32_t order,
                    float step_size, float eps);
float eni_nlms_update(eni_nlms_filter_t *f, float input, float desired);

/* ---------------------------------------------------------------------------
 * RLS Filter
 * ------------------------------------------------------------------------ */
typedef struct {
    float    w[ENI_RLS_MAX_ORDER];
    float    x[ENI_RLS_MAX_ORDER];
    float    P[ENI_RLS_MAX_ORDER][ENI_RLS_MAX_ORDER]; /**< Inverse corr. */
    float    lambda;                    /**< Forgetting factor. */
    float    delta;                     /**< Initialisation constant. */
    uint32_t order;
    uint32_t idx;
    bool     initialised;
} eni_rls_filter_t;

int   eni_rls_init(eni_rls_filter_t *f, uint32_t order,
                   float lambda, float delta);
float eni_rls_update(eni_rls_filter_t *f, float input, float desired);

#ifdef __cplusplus
}
#endif

#endif /* ENI_ADAPTIVE_FILTER_H */
