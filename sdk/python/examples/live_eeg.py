"""eNI Python SDK — Example: Live EEG streaming."""

from eni import ENIProvider, DSPPipeline, Decoder


def main():
    provider = ENIProvider(provider_type="simulator")

    dsp = DSPPipeline(sample_rate=256, fft_size=256, channels=64)
    dsp.init()

    decoder = Decoder(decoder_type="energy")

    with provider:
        print("Streaming neural data (10 samples)...")
        for i in range(10):
            packet = provider.poll()
            features = dsp.process(packet["samples"])
            intent = decoder.decode(packet["samples"])
            print(f"  [{i}] intent={intent['intent']} confidence={intent['confidence']:.2f} "
                  f"alpha={features['band_power']['alpha']:.4f}")

    print("Done.")


if __name__ == "__main__":
    main()
