"""eNI Visualize — Matplotlib/Plotly helpers for neural signal visualization."""


def plot_signal(samples, sample_rate=256, channels=None, title="Neural Signal"):
    """Plot multi-channel neural signal data."""
    try:
        import matplotlib.pyplot as plt
        import numpy as np

        data = np.array(samples, dtype=np.float32)
        if data.ndim == 1:
            data = data.reshape(1, -1)

        n_channels = data.shape[0] if channels is None else min(len(channels), data.shape[0])
        t = np.arange(data.shape[1]) / sample_rate

        fig, axes = plt.subplots(n_channels, 1, figsize=(12, 2 * n_channels), sharex=True)
        if n_channels == 1:
            axes = [axes]

        for i in range(n_channels):
            label = channels[i] if channels else f"Ch {i}"
            axes[i].plot(t, data[i], linewidth=0.5)
            axes[i].set_ylabel(label)
            axes[i].grid(True, alpha=0.3)

        axes[-1].set_xlabel("Time (s)")
        fig.suptitle(title)
        plt.tight_layout()
        return fig
    except ImportError:
        raise ImportError("Matplotlib required: pip install matplotlib")


def plot_psd(psd, sample_rate=256, fft_size=256, title="Power Spectral Density"):
    """Plot power spectral density."""
    try:
        import matplotlib.pyplot as plt
        import numpy as np

        freqs = np.linspace(0, sample_rate / 2, len(psd))
        fig, ax = plt.subplots(figsize=(10, 4))
        ax.semilogy(freqs, psd)
        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Power (µV²/Hz)")
        ax.set_title(title)
        ax.grid(True, alpha=0.3)

        bands = {"δ": (0.5, 4), "θ": (4, 8), "α": (8, 13), "β": (13, 30), "γ": (30, 100)}
        colors = ["blue", "green", "yellow", "orange", "red"]
        for (name, (lo, hi)), color in zip(bands.items(), colors):
            mask = (freqs >= lo) & (freqs <= hi)
            ax.fill_between(freqs[mask], psd[mask], alpha=0.2, color=color, label=name)
        ax.legend()
        plt.tight_layout()
        return fig
    except ImportError:
        raise ImportError("Matplotlib required: pip install matplotlib")


def plot_spectrogram(samples, sample_rate=256, title="Spectrogram"):
    """Plot spectrogram of a single channel."""
    try:
        import matplotlib.pyplot as plt
        import numpy as np

        data = np.array(samples, dtype=np.float32)
        fig, ax = plt.subplots(figsize=(10, 4))
        ax.specgram(data, NFFT=256, Fs=sample_rate, noverlap=128, cmap="viridis")
        ax.set_xlabel("Time (s)")
        ax.set_ylabel("Frequency (Hz)")
        ax.set_title(title)
        plt.colorbar(ax.images[0], ax=ax, label="Power (dB)")
        plt.tight_layout()
        return fig
    except ImportError:
        raise ImportError("Matplotlib required: pip install matplotlib")


def plot_topographic(channel_data, montage="10-20", title="Topographic Map"):
    """Plot topographic map (placeholder for MNE-style topomap)."""
    try:
        import matplotlib.pyplot as plt
        import numpy as np

        positions_10_20 = {
            "Fp1": (-0.3, 0.9), "Fp2": (0.3, 0.9), "F7": (-0.7, 0.6),
            "F3": (-0.35, 0.6), "Fz": (0.0, 0.6), "F4": (0.35, 0.6),
            "F8": (0.7, 0.6), "T3": (-0.9, 0.0), "C3": (-0.45, 0.0),
            "Cz": (0.0, 0.0), "C4": (0.45, 0.0), "T4": (0.9, 0.0),
            "T5": (-0.7, -0.6), "P3": (-0.35, -0.6), "Pz": (0.0, -0.6),
            "P4": (0.35, -0.6), "T6": (0.7, -0.6), "O1": (-0.3, -0.9),
            "O2": (0.3, -0.9),
        }

        fig, ax = plt.subplots(figsize=(6, 6))
        n = min(len(channel_data), len(positions_10_20))
        names = list(positions_10_20.keys())[:n]
        values = channel_data[:n]

        xs = [positions_10_20[name][0] for name in names]
        ys = [positions_10_20[name][1] for name in names]

        scatter = ax.scatter(xs, ys, c=values, cmap="RdBu_r", s=200, edgecolors="black")
        for name, x, y in zip(names, xs, ys):
            ax.annotate(name, (x, y), ha="center", va="center", fontsize=7)

        circle = plt.Circle((0, 0), 1.0, fill=False, linewidth=2)
        ax.add_patch(circle)
        ax.set_xlim(-1.3, 1.3)
        ax.set_ylim(-1.3, 1.3)
        ax.set_aspect("equal")
        ax.set_title(title)
        ax.axis("off")
        plt.colorbar(scatter, ax=ax, label="Amplitude (µV)")
        plt.tight_layout()
        return fig
    except ImportError:
        raise ImportError("Matplotlib required: pip install matplotlib")
