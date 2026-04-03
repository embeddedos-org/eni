"""eNI GUI Backend — WebSocket server bridging eNI to web clients."""

import asyncio
import json
import time
import math
import random

try:
    from fastapi import FastAPI, WebSocket, WebSocketDisconnect
    import uvicorn
    HAS_FASTAPI = True
except ImportError:
    HAS_FASTAPI = False


class ENIBridge:
    """Bridge between eNI C library and WebSocket clients."""

    def __init__(self, provider_type="simulator", sample_rate=256, channels=64):
        self.provider_type = provider_type
        self.sample_rate = sample_rate
        self.channels = channels
        self.running = False
        self._tick = 0

    def generate_sample(self):
        """Generate a simulated neural data sample."""
        self._tick += 1
        t = self._tick / self.sample_rate
        samples = []
        for ch in range(self.channels):
            alpha = 10.0 * math.sin(2 * math.pi * 10.0 * t + ch * 0.1)
            beta = 5.0 * math.sin(2 * math.pi * 20.0 * t + ch * 0.2)
            noise = random.gauss(0, 2.0)
            samples.append(alpha + beta + noise)

        intents = ["idle", "move_left", "move_right", "select", "attention", "motor_intent", "scroll_up"]
        intent_idx = int(t * 0.5) % len(intents)
        confidence = 0.5 + 0.3 * math.sin(t * 0.8)

        return {
            "type": "neural_data",
            "samples": samples[:8],
            "num_channels": self.channels,
            "timestamp": time.time(),
            "intent": {
                "type": "intent",
                "intent": intents[intent_idx],
                "confidence": max(0.0, min(1.0, confidence)),
                "class_id": intent_idx,
            },
            "band_power": {
                "delta": abs(random.gauss(5, 2)),
                "theta": abs(random.gauss(3, 1)),
                "alpha": abs(10 + 3 * math.sin(t)),
                "beta": abs(5 + 2 * math.cos(t)),
                "gamma": abs(random.gauss(1, 0.5)),
            },
        }


def create_app():
    if not HAS_FASTAPI:
        print("FastAPI not installed. Install with: pip install fastapi uvicorn")
        return None

    app = FastAPI(title="eNI GUI Backend", version="0.2.0")
    bridge = ENIBridge()

    @app.websocket("/ws")
    async def websocket_endpoint(websocket: WebSocket):
        await websocket.accept()
        bridge.running = True
        try:
            while bridge.running:
                data = bridge.generate_sample()
                await websocket.send_json(data)
                await asyncio.sleep(1.0 / 30)  # 30 fps
        except WebSocketDisconnect:
            bridge.running = False

    @app.get("/api/status")
    async def status():
        return {
            "version": "0.2.0",
            "provider": bridge.provider_type,
            "sample_rate": bridge.sample_rate,
            "channels": bridge.channels,
            "running": bridge.running,
        }

    return app


if __name__ == "__main__":
    app = create_app()
    if app:
        uvicorn.run(app, host="0.0.0.0", port=8765)
    else:
        print("Cannot start server. Install dependencies: pip install fastapi uvicorn")
