# eNI Models Directory

Pre-trained and example models for eNI neural decoders.

## Supported Formats

| Format | Extension | Description |
|--------|-----------|-------------|
| eNI Native | `.eni_model` | Binary format with magic `0x454E4931`, contains layer definitions + weights |
| ONNX | `.onnx` | Open Neural Network Exchange format (subset supported) |
| TFLite | `.tflite` | TensorFlow Lite flatbuffer format (requires `ENI_BUILD_TFLITE=ON`) |

## ONNX Supported Operators

The eNI ONNX loader supports a minimal subset of operators:

- `Gemm` (Dense/Linear)
- `Conv` (1D convolution)
- `Relu`, `Sigmoid`, `Tanh`
- `Softmax`
- `BatchNormalization`
- `Flatten`, `Reshape`
- `Add` (element-wise)

## Quantization

Use `tools/quantize_model.py` to convert a float32 ONNX model to INT8:

```bash
python tools/quantize_model.py --input model.onnx --output model_int8.onnx --calibration-data data.npy
```

## Model Size Constraints

For MCU targets (ARM Cortex-M4, RISC-V), models must fit within:

- **Max weights**: 16384 floats (64KB) — see `ENI_NN_MAX_WEIGHTS`
- **Max layers**: 16 — see `ENI_NN_MAX_LAYERS`
- **INT8 quantized**: 16KB effective weight storage

## Example Models

- `example_intent_classifier.onnx` — Tiny 3-layer MLP for 7-class intent classification (planned)
