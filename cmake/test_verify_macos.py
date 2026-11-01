"""Exercise the release gate without requiring Apple tools or a macOS runner."""
import plistlib
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import verify_macos as gate


class DeploymentGateTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.bundle = Path(temporary.name) / 'App With Spaces.app'
        (self.bundle / 'Contents').mkdir(parents=True)
        self.info = {'CFBundleExecutable': 'App With Spaces', 'LSMinimumSystemVersion': '11.0'}
        self.save_plist()

    def save_plist(self):
        (self.bundle / 'Contents/Info.plist').write_bytes(plistlib.dumps(self.info))

    def verify(self, arm='11.0', intel='11.0.0', arches='x86_64 arm64', legacy=False):
        def output(*args):
            if 'lipo' in args:
                return arches
            minimum = arm if args[3] == 'arm64' else intel
            command = 'LC_VERSION_MIN_MACOSX' if legacy else 'LC_BUILD_VERSION'
            field = 'version' if legacy else 'minos'
            return f'Load command 9\n cmd {command}\n {field} {minimum}\n sdk 26.5\n'
        with patch.object(gate, 'run', side_effect=output):
            gate.verify(self.bundle)

    def test_valid_universal_bundle(self):
        self.verify()

    def test_legacy_load_command(self):
        self.verify(legacy=True)

    def test_newer_arm_target(self):
        with self.assertRaises(ValueError):
            self.verify(arm='26.0')

    def test_newer_intel_target(self):
        with self.assertRaises(ValueError):
            self.verify(intel='15.0')

    def test_missing_architecture(self):
        with self.assertRaises(ValueError):
            self.verify(arches='arm64')

    def test_missing_load_command(self):
        with patch.object(gate, 'run', side_effect=['arm64 x86_64', 'Load command 1\n cmd LC_UUID\n']):
            with self.assertRaises(ValueError):
                gate.verify(self.bundle)

    def test_incorrect_plist(self):
        self.info['LSMinimumSystemVersion'] = '26.0'
        self.save_plist()
        with self.assertRaises(ValueError):
            self.verify()

    def test_missing_bundle(self):
        with self.assertRaises(OSError):
            gate.verify(self.bundle / 'missing.app')


if __name__ == '__main__':
    unittest.main()
