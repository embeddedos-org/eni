/**
 * eNI Node.js SDK — TypeScript bindings for the embedded Neural Interface.
 */

import { EventEmitter } from "events";
import {
  ENIPacket,
  ENIIntent,
  ENIBandPower,
  ENIConfig,
  ProviderType,
  DecoderType,
} from "./types";

export class ENIProvider extends EventEmitter {
  private config: ENIConfig;
  private connected = false;
  private pollInterval: ReturnType<typeof setInterval> | null = null;

  constructor(config?: Partial<ENIConfig>) {
    super();
    this.config = {
      provider: config?.provider ?? ProviderType.SIMULATOR,
      sample_rate: config?.sample_rate ?? 256,
      channels: config?.channels ?? 64,
      decoder: config?.decoder ?? DecoderType.ENERGY,
      model_path: config?.model_path,
    };
  }

  async connect(): Promise<void> {
    this.connected = true;
    this.emit("connected", this.config.provider);
  }

  async disconnect(): Promise<void> {
    if (this.pollInterval) {
      clearInterval(this.pollInterval);
      this.pollInterval = null;
    }
    this.connected = false;
    this.emit("disconnected");
  }

  poll(): ENIPacket {
    if (!this.connected) throw new Error("Provider not connected");
    const samples = new Array(this.config.channels).fill(0).map(() => Math.random() * 100 - 50);
    return {
      samples,
      num_channels: this.config.channels,
      timestamp: Date.now(),
    };
  }

  startStreaming(intervalMs = 10): void {
    if (!this.connected) throw new Error("Provider not connected");
    this.pollInterval = setInterval(() => {
      const packet = this.poll();
      this.emit("data", packet);
    }, intervalMs);
  }

  stopStreaming(): void {
    if (this.pollInterval) {
      clearInterval(this.pollInterval);
      this.pollInterval = null;
    }
  }

  isConnected(): boolean {
    return this.connected;
  }
}

export class DSPPipeline {
  private sampleRate: number;
  private fftSize: number;
  private channels: number;

  constructor(sampleRate = 256, fftSize = 256, channels = 64) {
    this.sampleRate = sampleRate;
    this.fftSize = Math.min(fftSize, 512);
    this.channels = channels;
  }

  process(samples: number[]): { psd: number[]; bandPower: ENIBandPower } {
    const psd = new Array(this.fftSize / 2).fill(0);
    const bandPower: ENIBandPower = { delta: 0, theta: 0, alpha: 0, beta: 0, gamma: 0 };
    return { psd, bandPower };
  }
}

export class Decoder {
  private type: DecoderType;

  constructor(type: DecoderType = DecoderType.ENERGY) {
    this.type = type;
  }

  decode(samples: number[]): ENIIntent {
    return { intent: "idle", confidence: 0.0, class_id: 0 };
  }
}

export { ENIPacket, ENIIntent, ENIBandPower, ENIConfig, ProviderType, DecoderType } from "./types";
