/**
 * @file lstm.h
 * @brief LSTM (Long Short-Term Memory) layer for embedded inference.
 *
 * Single-layer LSTM with forget, input, output, and cell gates.
 * Fixed-size arrays — MCU-safe, no dynamic allocation.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_LSTM_H
#define ENI_LSTM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */
#define ENI_LSTM_MAX_HIDDEN     128
#define ENI_LSTM_MAX_INPUT       64

/* ---------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------ */

/**
 * LSTM cell.
 *
 * Gate weight matrices: W_* are [hidden x input], U_* are [hidden x hidden],
 * b_* are [hidden]. Stored as flat arrays in row-major order.
 */
typedef struct {
    /* Gate weights: input (x) path */
    float Wf[ENI_LSTM_MAX_HIDDEN * ENI_LSTM_MAX_INPUT];  /**< Forget gate W */
    float Wi[ENI_LSTM_MAX_HIDDEN * ENI_LSTM_MAX_INPUT];  /**< Input gate W  */
    float Wo[ENI_LSTM_MAX_HIDDEN * ENI_LSTM_MAX_INPUT];  /**< Output gate W */
    float Wc[ENI_LSTM_MAX_HIDDEN * ENI_LSTM_MAX_INPUT];  /**< Cell gate W   */

    /* Gate weights: hidden (h) path */
    float Uf[ENI_LSTM_MAX_HIDDEN * ENI_LSTM_MAX_HIDDEN]; /**< Forget gate U */
    float Ui[ENI_LSTM_MAX_HIDDEN * ENI_LSTM_MAX_HIDDEN]; /**< Input gate U  */
    float Uo[ENI_LSTM_MAX_HIDDEN * ENI_LSTM_MAX_HIDDEN]; /**< Output gate U */
    float Uc[ENI_LSTM_MAX_HIDDEN * ENI_LSTM_MAX_HIDDEN]; /**< Cell gate U   */

    /* Biases */
    float bf[ENI_LSTM_MAX_HIDDEN];
    float bi[ENI_LSTM_MAX_HIDDEN];
    float bo[ENI_LSTM_MAX_HIDDEN];
    float bc[ENI_LSTM_MAX_HIDDEN];

    /* State */
    float h[ENI_LSTM_MAX_HIDDEN];    /**< Hidden state. */
    float c[ENI_LSTM_MAX_HIDDEN];    /**< Cell state. */

    /* Dimensions */
    uint32_t input_size;
    uint32_t hidden_size;

    bool initialised;
} eni_lstm_t;

/* ---------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------ */

/**
 * Initialise an LSTM cell.
 * Weights are zero-initialised; call eni_lstm_set_weights to load.
 */
int eni_lstm_init(eni_lstm_t *lstm, uint32_t input_size, uint32_t hidden_size);

/**
 * Forward pass for a single time-step.
 * @param x      Input vector [input_size].
 * @param h_out  Output hidden state [hidden_size] (can alias lstm->h).
 * @return 0 on success.
 */
int eni_lstm_forward(eni_lstm_t *lstm,
                     const float *x,
                     float *h_out);

/**
 * Reset hidden and cell states to zero.
 */
int eni_lstm_reset_state(eni_lstm_t *lstm);

/**
 * Load all gate weights and biases from flat arrays.
 * Each W array must be [hidden_size * input_size], each U array
 * [hidden_size * hidden_size], each bias [hidden_size].
 */
int eni_lstm_set_weights(eni_lstm_t *lstm,
                         const float *Wf, const float *Wi,
                         const float *Wo, const float *Wc,
                         const float *Uf, const float *Ui,
                         const float *Uo, const float *Uc,
                         const float *bf, const float *bi,
                         const float *bo, const float *bc);

#ifdef __cplusplus
}
#endif

#endif /* ENI_LSTM_H */
