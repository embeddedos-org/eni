"""eNI Stream — Real-time data streaming utilities."""

import asyncio
import time


class NeuralStream:
    """Async generator for real-time neural data streaming."""

    def __init__(self, provider, sample_rate=256):
        self._provider = provider
        self._sample_rate = sample_rate
        self._running = False

    async def start(self):
        """Start streaming neural data."""
        self._running = True
        interval = 1.0 / self._sample_rate
        while self._running:
            packet = self._provider.poll()
            yield packet
            await asyncio.sleep(interval)

    def stop(self):
        """Stop the stream."""
        self._running = False

    def to_numpy(self, packets, num_channels=64):
        """Convert a list of packets to a NumPy array."""
        try:
            import numpy as np
            data = [p["samples"][:num_channels] for p in packets]
            return np.array(data, dtype=np.float32)
        except ImportError:
            raise ImportError("NumPy is required: pip install numpy")

    def to_dataframe(self, packets, num_channels=64, channel_names=None):
        """Convert packets to a Pandas DataFrame."""
        try:
            import pandas as pd
            import numpy as np
            data = self.to_numpy(packets, num_channels)
            if channel_names is None:
                channel_names = [f"ch_{i}" for i in range(num_channels)]
            df = pd.DataFrame(data, columns=channel_names[:num_channels])
            df["timestamp"] = [p.get("timestamp", i) for i, p in enumerate(packets)]
            return df
        except ImportError:
            raise ImportError("Pandas and NumPy required: pip install pandas numpy")


class BufferedStream:
    """Synchronous buffered stream for batch processing."""

    def __init__(self, provider, buffer_size=256):
        self._provider = provider
        self._buffer_size = buffer_size
        self._buffer = []

    def collect(self, num_samples=None):
        """Collect samples into a buffer."""
        n = num_samples or self._buffer_size
        samples = []
        for _ in range(n):
            packet = self._provider.poll()
            samples.append(packet)
        return samples

    def epoch(self, duration_ms, sample_rate=256):
        """Collect an epoch of fixed duration."""
        num_samples = int(duration_ms * sample_rate / 1000)
        return self.collect(num_samples)
