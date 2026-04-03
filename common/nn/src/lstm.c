/**
 * @file lstm.c
 * @brief LSTM layer implementation with gate computations.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/lstm.h"
#include <string.h>
#include <math.h>

/* ---- Activation functions ----------------------------------------------- */

static inline float sigmoid_f(float x)
{
    if (x > 10.0f)  return 1.0f;
    if (x < -10.0f) return 0.0f;
    return 1.0f / (1.0f + expf(-x));
}

static inline float tanh_f(float x)
{
    if (x > 10.0f)  return 1.0f;
    if (x < -10.0f) return -1.0f;
    return tanhf(x);
}

/* ---- Matrix-vector multiply: y += A * x -------------------------------- */

static void matvec_add(float *y,
                       const float *A,
                       const float *x,
                       uint32_t rows,
                       uint32_t cols)
{
    for (uint32_t r = 0; r < rows; r++) {
        float sum = 0.0f;
        const float *row = A + r * cols;
        for (uint32_t c = 0; c < cols; c++) {
            sum += row[c] * x[c];
        }
        y[r] += sum;
    }
}

/* ---- Public API --------------------------------------------------------- */

int eni_lstm_init(eni_lstm_t *lstm, uint32_t input_size, uint32_t hidden_size)
{
    if (!lstm) return -1;
    if (input_size == 0 || input_size > ENI_LSTM_MAX_INPUT) return -2;
    if (hidden_size == 0 || hidden_size > ENI_LSTM_MAX_HIDDEN) return -2;

    memset(lstm, 0, sizeof(*lstm));
    lstm->input_size  = input_size;
    lstm->hidden_size = hidden_size;
    lstm->initialised = true;
    return 0;
}

int eni_lstm_forward(eni_lstm_t *lstm,
                     const float *x,
                     float *h_out)
{
    if (!lstm || !x) return -1;
    if (!lstm->initialised) return -3;

    uint32_t H = lstm->hidden_size;
    uint32_t I = lstm->input_size;

    /* Temporary gate pre-activations */
    float fg[ENI_LSTM_MAX_HIDDEN]; /* forget gate */
    float ig[ENI_LSTM_MAX_HIDDEN]; /* input gate */
    float og[ENI_LSTM_MAX_HIDDEN]; /* output gate */
    float cg[ENI_LSTM_MAX_HIDDEN]; /* cell candidate */

    /* Initialise with biases */
    memcpy(fg, lstm->bf, H * sizeof(float));
    memcpy(ig, lstm->bi, H * sizeof(float));
    memcpy(og, lstm->bo, H * sizeof(float));
    memcpy(cg, lstm->bc, H * sizeof(float));

    /* Add W * x */
    matvec_add(fg, lstm->Wf, x, H, I);
    matvec_add(ig, lstm->Wi, x, H, I);
    matvec_add(og, lstm->Wo, x, H, I);
    matvec_add(cg, lstm->Wc, x, H, I);

    /* Add U * h_{t-1} */
    matvec_add(fg, lstm->Uf, lstm->h, H, H);
    matvec_add(ig, lstm->Ui, lstm->h, H, H);
    matvec_add(og, lstm->Uo, lstm->h, H, H);
    matvec_add(cg, lstm->Uc, lstm->h, H, H);

    /* Apply activations and update state */
    for (uint32_t j = 0; j < H; j++) {
        float f = sigmoid_f(fg[j]);
        float i = sigmoid_f(ig[j]);
        float o = sigmoid_f(og[j]);
        float c_hat = tanh_f(cg[j]);

        /* Cell state update: c_t = f * c_{t-1} + i * c_hat */
        lstm->c[j] = f * lstm->c[j] + i * c_hat;

        /* Hidden state: h_t = o * tanh(c_t) */
        lstm->h[j] = o * tanh_f(lstm->c[j]);
    }

    /* Copy output */
    if (h_out) {
        memcpy(h_out, lstm->h, H * sizeof(float));
    }

    return 0;
}

int eni_lstm_reset_state(eni_lstm_t *lstm)
{
    if (!lstm) return -1;
    if (!lstm->initialised) return -3;

    memset(lstm->h, 0, sizeof(lstm->h));
    memset(lstm->c, 0, sizeof(lstm->c));
    return 0;
}

int eni_lstm_set_weights(eni_lstm_t *lstm,
                         const float *Wf, const float *Wi,
                         const float *Wo, const float *Wc,
                         const float *Uf, const float *Ui,
                         const float *Uo, const float *Uc,
                         const float *bf, const float *bi,
                         const float *bo, const float *bc)
{
    if (!lstm) return -1;
    if (!lstm->initialised) return -3;

    uint32_t H = lstm->hidden_size;
    uint32_t I = lstm->input_size;
    uint32_t wi_size = H * I * sizeof(float);
    uint32_t uh_size = H * H * sizeof(float);
    uint32_t b_size  = H * sizeof(float);

    if (Wf) memcpy(lstm->Wf, Wf, wi_size);
    if (Wi) memcpy(lstm->Wi, Wi, wi_size);
    if (Wo) memcpy(lstm->Wo, Wo, wi_size);
    if (Wc) memcpy(lstm->Wc, Wc, wi_size);

    if (Uf) memcpy(lstm->Uf, Uf, uh_size);
    if (Ui) memcpy(lstm->Ui, Ui, uh_size);
    if (Uo) memcpy(lstm->Uo, Uo, uh_size);
    if (Uc) memcpy(lstm->Uc, Uc, uh_size);

    if (bf) memcpy(lstm->bf, bf, b_size);
    if (bi) memcpy(lstm->bi, bi, b_size);
    if (bo) memcpy(lstm->bo, bo, b_size);
    if (bc) memcpy(lstm->bc, bc, b_size);

    return 0;
}
