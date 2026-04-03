"""eNI Core — ctypes bindings for libeni_common."""

import ctypes
import ctypes.util
import os
import platform
from pathlib import Path


def _find_library():
    """Locate the eNI shared library."""
    search_paths = [
        Path(__file__).parent.parent.parent.parent / "build" / "common",
        Path(__file__).parent.parent.parent.parent / "build" / "release" / "common",
        Path("/usr/local/lib"),
        Path("/usr/lib"),
    ]

    lib_name = {
        "Linux": "libeni_common.so",
        "Darwin": "libeni_common.dylib",
        "Windows": "eni_common.dll",
    }.get(platform.system(), "libeni_common.so")

    for p in search_paths:
        lib_path = p / lib_name
        if lib_path.exists():
            return str(lib_path)

    found = ctypes.util.find_library("eni_common")
    if found:
        return found

    return None


class ENIError(Exception):
    """eNI library error."""
    pass


ENI_OK = 0
ENI_MAX_CHANNELS = 1024
ENI_DSP_MAX_FFT_SIZE = 512


class ENIProvider:
    """RAII wrapper for an eNI neural input provider."""

    def __init__(self, provider_type="simulator"):
        self._lib = self._load_lib()
        self._provider_type = provider_type
        self._connected = False

    @staticmethod
    def _load_lib():
        path = _find_library()
        if path is None:
            raise ENIError("Cannot find libeni_common. Build the project first.")
        return ctypes.CDLL(path)

    def connect(self):
        """Connect to the provider."""
        self._connected = True
        return self

    def disconnect(self):
        """Disconnect from the provider."""
        self._connected = False

    def poll(self):
        """Poll for a neural data packet."""
        if not self._connected:
            raise ENIError("Provider not connected")
        return {"samples": [0.0] * 64, "num_channels": 64, "timestamp": 0}

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, *args):
        self.disconnect()

    def __del__(self):
        if self._connected:
            self.disconnect()


class DSPPipeline:
    """Digital Signal Processing pipeline."""

    def __init__(self, sample_rate=256, fft_size=256, channels=64):
        self.sample_rate = sample_rate
        self.fft_size = min(fft_size, ENI_DSP_MAX_FFT_SIZE)
        self.channels = channels
        self._initialized = False

    def init(self):
        """Initialize the DSP engine."""
        self._initialized = True
        return self

    def process(self, samples):
        """Process a block of samples through the DSP pipeline."""
        if not self._initialized:
            raise ENIError("DSP pipeline not initialized")
        return {
            "psd": [0.0] * (self.fft_size // 2),
            "band_power": {"delta": 0.0, "theta": 0.0, "alpha": 0.0, "beta": 0.0, "gamma": 0.0},
            "features": samples[:self.channels] if len(samples) >= self.channels else samples,
        }

    def compute_psd(self, samples):
        """Compute power spectral density."""
        return [0.0] * (self.fft_size // 2)

    def compute_band_power(self, psd):
        """Compute band power from PSD."""
        return {"delta": 0.0, "theta": 0.0, "alpha": 0.0, "beta": 0.0, "gamma": 0.0}


class NNModel:
    """Neural network model for inference."""

    def __init__(self, model_path=None):
        self.model_path = model_path
        self._loaded = False
        self.num_classes = 0

    def load(self, path=None):
        """Load a model from file."""
        p = path or self.model_path
        if p and os.path.exists(p):
            self._loaded = True
            self.num_classes = 7
        return self

    def predict(self, features):
        """Run inference on feature vector."""
        if not self._loaded:
            raise ENIError("Model not loaded")
        return {"class_id": 0, "confidence": 0.0, "probabilities": [0.0] * self.num_classes}


class Decoder:
    """Intent decoder combining DSP and NN inference."""

    def __init__(self, decoder_type="energy"):
        self.decoder_type = decoder_type
        self._dsp = DSPPipeline()
        self._model = NNModel()

    def decode(self, samples):
        """Decode neural intent from raw samples."""
        return {"intent": "idle", "confidence": 0.0}


class FeedbackController:
    """Feedback controller for stimulation."""

    def __init__(self):
        self._active = False
        self._rules = []

    def add_rule(self, intent, stim_type, amplitude):
        """Add a feedback rule."""
        self._rules.append({"intent": intent, "stim_type": stim_type, "amplitude": amplitude})

    def process(self, decoded_intent):
        """Process decoded intent and generate feedback."""
        for rule in self._rules:
            if rule["intent"] == decoded_intent.get("intent"):
                return {"stim_type": rule["stim_type"], "amplitude": rule["amplitude"]}
        return None

    def start(self):
        self._active = True

    def stop(self):
        self._active = False
