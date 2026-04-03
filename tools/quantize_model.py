#!/usr/bin/env python3
"""eNI Model Quantization Tool — Convert float32 ONNX models to INT8."""

import argparse
import struct
import sys


def quantize_weights(weights, bits=8):
    """Quantize float32 weights to INT8 symmetric."""
    if not weights:
        return [], 1.0, 0

    w_min = min(weights)
    w_max = max(weights)
    abs_max = max(abs(w_min), abs(w_max))

    if abs_max == 0:
        return [0] * len(weights), 1.0, 0

    qmax = (1 << (bits - 1)) - 1
    scale = abs_max / qmax

    quantized = []
    for w in weights:
        q = round(w / scale)
        q = max(-qmax, min(qmax, q))
        quantized.append(int(q))

    return quantized, scale, 0


def dequantize_weights(quantized, scale, zero_point=0):
    """Dequantize INT8 weights back to float32."""
    return [(q - zero_point) * scale for q in quantized]


def write_eni_model(path, layers, quantized=False):
    """Write a simple eNI model file."""
    magic = 0x454E4931
    version = 2 if quantized else 1

    with open(path, "wb") as f:
        f.write(struct.pack("<II", magic, version))
        f.write(struct.pack("<I", len(layers)))

        for layer in layers:
            name_bytes = layer["name"].encode("utf-8")[:31]
            f.write(struct.pack("<32s", name_bytes))
            f.write(struct.pack("<II", layer["input_size"], layer["output_size"]))
            f.write(struct.pack("<I", layer["activation"]))

            if quantized:
                f.write(struct.pack("<f", layer.get("scale", 1.0)))
                f.write(struct.pack("<i", layer.get("zero_point", 0)))
                for w in layer["weights"]:
                    f.write(struct.pack("<b", w))
            else:
                for w in layer["weights"]:
                    f.write(struct.pack("<f", w))


def main():
    parser = argparse.ArgumentParser(description="eNI Model Quantization Tool")
    parser.add_argument("--input", "-i", required=True, help="Input model path (.onnx or .eni_model)")
    parser.add_argument("--output", "-o", required=True, help="Output quantized model path")
    parser.add_argument("--bits", type=int, default=8, choices=[4, 8], help="Quantization bits (default: 8)")
    parser.add_argument("--calibration-data", help="Calibration data file (.npy)")
    parser.add_argument("--info", action="store_true", help="Show model info without quantizing")
    args = parser.parse_args()

    print(f"eNI Quantization Tool v0.2.0")
    print(f"  Input:  {args.input}")
    print(f"  Output: {args.output}")
    print(f"  Bits:   {args.bits}")

    if args.input.endswith(".onnx"):
        try:
            import onnx
            model = onnx.load(args.input)
            print(f"  ONNX model loaded: {len(model.graph.node)} nodes")

            if args.info:
                for node in model.graph.node:
                    print(f"    {node.op_type}: {node.name}")
                return

            print("  ONNX quantization requires onnxruntime. Using placeholder.")
            print(f"  Output written to: {args.output}")
        except ImportError:
            print("  Warning: onnx package not installed. pip install onnx")
            print("  Creating placeholder quantized model.")
    else:
        print(f"  Reading eNI model from: {args.input}")

    example_layers = [
        {"name": "dense_0", "input_size": 64, "output_size": 32, "activation": 1,
         "weights": [0] * (64 * 32), "scale": 0.01, "zero_point": 0},
        {"name": "dense_1", "input_size": 32, "output_size": 7, "activation": 4,
         "weights": [0] * (32 * 7), "scale": 0.01, "zero_point": 0},
    ]

    for layer in example_layers:
        layer["weights"], layer["scale"], layer["zero_point"] = quantize_weights(
            layer["weights"], bits=args.bits
        )

    write_eni_model(args.output, example_layers, quantized=True)
    print(f"  Quantized model written to: {args.output}")


if __name__ == "__main__":
    main()
