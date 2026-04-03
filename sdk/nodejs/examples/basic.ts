import { ENIProvider, DSPPipeline, Decoder, ProviderType, DecoderType } from "../src/index";

async function main() {
  const provider = new ENIProvider({
    provider: ProviderType.SIMULATOR,
    sample_rate: 256,
    channels: 64,
    decoder: DecoderType.ENERGY,
  });

  const dsp = new DSPPipeline(256, 256, 64);
  const decoder = new Decoder(DecoderType.ENERGY);

  await provider.connect();

  provider.on("data", (packet) => {
    const { bandPower } = dsp.process(packet.samples);
    const intent = decoder.decode(packet.samples);
    console.log(`Intent: ${intent.intent} (${intent.confidence.toFixed(2)}) | Alpha: ${bandPower.alpha.toFixed(4)}`);
  });

  provider.startStreaming(100);

  setTimeout(async () => {
    provider.stopStreaming();
    await provider.disconnect();
    console.log("Done.");
  }, 5000);
}

main().catch(console.error);
