import React, { useState, useEffect } from "react";

interface ConnectionState {
  connected: boolean;
  provider: string;
  sampleRate: number;
  channels: number;
}

const App: React.FC = () => {
  const [connection, setConnection] = useState<ConnectionState>({
    connected: false,
    provider: "simulator",
    sampleRate: 256,
    channels: 64,
  });
  const [latestIntent, setLatestIntent] = useState({ intent: "idle", confidence: 0 });
  const [ws, setWs] = useState<WebSocket | null>(null);

  useEffect(() => {
    return () => { ws?.close(); };
  }, [ws]);

  const handleConnect = () => {
    const socket = new WebSocket("ws://localhost:8765");
    socket.onopen = () => setConnection((prev) => ({ ...prev, connected: true }));
    socket.onclose = () => setConnection((prev) => ({ ...prev, connected: false }));
    socket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      if (data.type === "intent") {
        setLatestIntent({ intent: data.intent, confidence: data.confidence });
      }
    };
    setWs(socket);
  };

  const handleDisconnect = () => {
    ws?.close();
    setWs(null);
    setConnection((prev) => ({ ...prev, connected: false }));
  };

  return (
    <div style={{ fontFamily: "system-ui", padding: 24, maxWidth: 1200, margin: "0 auto" }}>
      <header style={{ borderBottom: "2px solid #333", paddingBottom: 16, marginBottom: 24 }}>
        <h1>eNI — Neural Interface Dashboard</h1>
        <div style={{ display: "flex", gap: 16, alignItems: "center" }}>
          <span style={{
            display: "inline-block", width: 12, height: 12, borderRadius: "50%",
            backgroundColor: connection.connected ? "#4caf50" : "#f44336",
          }} />
          <span>{connection.connected ? "Connected" : "Disconnected"}</span>
          <button onClick={connection.connected ? handleDisconnect : handleConnect}>
            {connection.connected ? "Disconnect" : "Connect"}
          </button>
          <select
            value={connection.provider}
            onChange={(e) => setConnection((prev) => ({ ...prev, provider: e.target.value }))}
          >
            <option value="simulator">Simulator</option>
            <option value="eeg">EEG</option>
            <option value="neuralink">Neuralink</option>
          </select>
        </div>
      </header>

      <main style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 24 }}>
        <section style={{ border: "1px solid #ddd", borderRadius: 8, padding: 16 }}>
          <h2>Signal Viewer</h2>
          <p style={{ color: "#666" }}>Multi-channel neural signal display</p>
          <div style={{ height: 200, background: "#f5f5f5", borderRadius: 4, display: "flex", alignItems: "center", justifyContent: "center" }}>
            {connection.connected ? "Streaming..." : "Connect to view signals"}
          </div>
        </section>

        <section style={{ border: "1px solid #ddd", borderRadius: 8, padding: 16 }}>
          <h2>Power Spectral Density</h2>
          <p style={{ color: "#666" }}>Frequency domain analysis</p>
          <div style={{ height: 200, background: "#f5f5f5", borderRadius: 4, display: "flex", alignItems: "center", justifyContent: "center" }}>
            {connection.connected ? "Computing PSD..." : "Connect to view PSD"}
          </div>
        </section>

        <section style={{ border: "1px solid #ddd", borderRadius: 8, padding: 16 }}>
          <h2>Decoder Output</h2>
          <div style={{ fontSize: 24, fontWeight: "bold", textAlign: "center", padding: 32 }}>
            <div>{latestIntent.intent.toUpperCase()}</div>
            <div style={{ fontSize: 14, color: "#666" }}>
              Confidence: {(latestIntent.confidence * 100).toFixed(1)}%
            </div>
          </div>
        </section>

        <section style={{ border: "1px solid #ddd", borderRadius: 8, padding: 16 }}>
          <h2>Feedback Control</h2>
          <div style={{ display: "flex", flexDirection: "column", gap: 12 }}>
            <label>
              Stimulation Type:
              <select style={{ marginLeft: 8 }}>
                <option value="none">None</option>
                <option value="haptic">Haptic</option>
                <option value="visual">Visual</option>
                <option value="electrical">Electrical</option>
              </select>
            </label>
            <label>
              Amplitude (mA):
              <input type="range" min="0" max="2" step="0.1" defaultValue="0.5" style={{ marginLeft: 8 }} />
            </label>
            <button disabled={!connection.connected}>Apply Feedback</button>
          </div>
        </section>
      </main>

      <footer style={{ marginTop: 32, paddingTop: 16, borderTop: "1px solid #eee", color: "#999", fontSize: 12 }}>
        eNI v0.2.0 | Provider: {connection.provider} | {connection.sampleRate}Hz | {connection.channels}ch
      </footer>
    </div>
  );
};

export default App;
