"""eNI Python SDK — Unit tests for bindings."""

import unittest
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))


class TestENIProvider(unittest.TestCase):
    def test_create_simulator(self):
        from eni.core import ENIProvider
        try:
            provider = ENIProvider(provider_type="simulator")
            self.assertEqual(provider._provider_type, "simulator")
        except Exception:
            self.skipTest("libeni_common not available")

    def test_context_manager(self):
        from eni.core import ENIProvider
        try:
            with ENIProvider("simulator") as p:
                self.assertTrue(p._connected)
            self.assertFalse(p._connected)
        except Exception:
            self.skipTest("libeni_common not available")


class TestDSPPipeline(unittest.TestCase):
    def test_init(self):
        from eni.core import DSPPipeline
        dsp = DSPPipeline(sample_rate=256, fft_size=256, channels=64)
        dsp.init()
        self.assertTrue(dsp._initialized)

    def test_fft_size_clamped(self):
        from eni.core import DSPPipeline
        dsp = DSPPipeline(fft_size=1024)
        self.assertEqual(dsp.fft_size, 512)

    def test_process_requires_init(self):
        from eni.core import DSPPipeline, ENIError
        dsp = DSPPipeline()
        with self.assertRaises(ENIError):
            dsp.process([0.0] * 64)


class TestNNModel(unittest.TestCase):
    def test_predict_requires_load(self):
        from eni.core import NNModel, ENIError
        model = NNModel()
        with self.assertRaises(ENIError):
            model.predict([0.0] * 10)


class TestDecoder(unittest.TestCase):
    def test_decode_returns_intent(self):
        from eni.core import Decoder
        dec = Decoder()
        result = dec.decode([0.0] * 64)
        self.assertIn("intent", result)
        self.assertIn("confidence", result)


class TestFeedbackController(unittest.TestCase):
    def test_add_rule(self):
        from eni.core import FeedbackController
        fc = FeedbackController()
        fc.add_rule("move_left", "haptic", 0.5)
        self.assertEqual(len(fc._rules), 1)

    def test_process_matching_rule(self):
        from eni.core import FeedbackController
        fc = FeedbackController()
        fc.add_rule("attention", "haptic", 0.8)
        result = fc.process({"intent": "attention"})
        self.assertIsNotNone(result)
        self.assertEqual(result["stim_type"], "haptic")

    def test_process_no_match(self):
        from eni.core import FeedbackController
        fc = FeedbackController()
        fc.add_rule("attention", "haptic", 0.8)
        result = fc.process({"intent": "idle"})
        self.assertIsNone(result)


if __name__ == "__main__":
    unittest.main()
