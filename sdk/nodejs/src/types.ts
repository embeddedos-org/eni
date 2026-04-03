export interface ENIPacket {
  samples: number[];
  num_channels: number;
  timestamp: number;
}

export interface ENIIntent {
  intent: string;
  confidence: number;
  class_id: number;
}

export interface ENIBandPower {
  delta: number;
  theta: number;
  alpha: number;
  beta: number;
  gamma: number;
}

export interface ENIStimParams {
  type: StimType;
  amplitude: number;
  duration_ms: number;
  frequency: number;
}

export enum StimType {
  NONE = 0,
  HAPTIC = 1,
  VISUAL = 2,
  AUDITORY = 3,
  ELECTRICAL = 4,
  THERMAL = 5,
  COMBINED = 6,
}

export enum DecoderType {
  ENERGY = "energy",
  NN = "nn",
  CNN = "cnn",
  LSTM = "lstm",
}

export enum ProviderType {
  SIMULATOR = "simulator",
  NEURALINK = "neuralink",
  EEG = "eeg",
  GENERIC = "generic",
  WIRELESS = "wireless",
}

export interface ENIConfig {
  provider: ProviderType;
  sample_rate: number;
  channels: number;
  decoder: DecoderType;
  model_path?: string;
}
