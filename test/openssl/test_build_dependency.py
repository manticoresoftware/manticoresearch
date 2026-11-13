#!/usr/bin/env python3
"""Exercise the actual OpenSSL configure gate and vcpkg install invocation."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[2]
CMAKE = shutil.which("cmake")


@unittest.skipUnless(CMAKE, "cmake is required")
class OpenSSLBuildTest(unittest.TestCase):
    def run_cmake(self, source):
        with tempfile.TemporaryDirectory(prefix="manticore openssl ") as temp:
            script = Path(temp) / "check.cmake"
            script.write_text(source)
            return subprocess.run([CMAKE, "-P", str(script)], capture_output=True, text=True)

    def version_result(self, version, enabled=True, modern=False):
        variable = "OpenSSL_VERSION" if modern else "OPENSSL_VERSION"
        return self.run_cmake(
            f'set(WITH_SSL {"ON" if enabled else "OFF"})\n'
            f'set({variable} "{version}")\n'
            f'include("{REPO / "cmake/CheckOpenSSLVersion.cmake"}")\n')

    def test_affected_36_releases_rejected(self):
        for version in [f"3.6.{patch}" for patch in range(5)]:
            with self.subTest(version=version):
                result = self.version_result(version)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("CVE-2026-84782", result.stderr)
                self.assertIn("3.6.5", result.stderr)

    def test_fixed_36_releases_accepted(self):
        for version in ["3.6.5", "3.6.6"]:
            with self.subTest(version=version):
                result = self.version_result(version)
                self.assertEqual(result.returncode, 0, result.stderr)

    def test_other_branches_keep_provider_policy(self):
        for version in ["3.0.2", "3.5.9", "4.0.3", ""]:
            with self.subTest(version=version):
                result = self.version_result(version)
                self.assertEqual(result.returncode, 0, result.stderr)

    def test_disabled_ssl_does_not_reject_unused_version(self):
        result = self.version_result("3.6.1", enabled=False)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_modern_cmake_version_variable(self):
        result = self.version_result("3.6.1", modern=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("CVE-2026-84782", result.stderr)

    def discovery_result(self, version, modern=False):
        with tempfile.TemporaryDirectory(prefix="manticore openssl discovery ") as temp:
            root = Path(temp)
            variable = "OpenSSL_VERSION" if modern else "OPENSSL_VERSION"
            (root / "FindOpenSSL.cmake").write_text(
                'set(OpenSSL_FOUND TRUE)\nset(OPENSSL_FOUND TRUE)\n'
                f'set({variable} "{version}")\n'
                'add_library(OpenSSL::SSL INTERFACE IMPORTED)\n')
            (root / "CMakeLists.txt").write_text(
                'cmake_minimum_required(VERSION 3.17)\nproject(OpenSSLDiscovery NONE)\n'
                f'set(CMAKE_MODULE_PATH "{root}" "{REPO / "cmake"}")\n'
                'include(FeatureSummary)\ninclude(helpers)\n'
                # Only the top-level project's banner logger is omitted.
                'function(bannervar variable)\nendfunction()\n'
                'with_menu_comp(OpenSSL SSL "OpenSSL" "for encrypted networking")\n'
                'include(CheckOpenSSLVersion)\n')
            return subprocess.run([CMAKE, "-S", str(root), "-B", str(root / "build")],
                                  capture_output=True, text=True)

    def test_real_discovery_helper_rejects_affected_library(self):
        for modern in [False, True]:
            with self.subTest(modern=modern):
                result = self.discovery_result("3.6.1", modern=modern)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("CVE-2026-84782", result.stderr)

    def test_real_discovery_helper_accepts_fixed_library(self):
        for modern in [False, True]:
            with self.subTest(modern=modern):
                result = self.discovery_result("3.6.5", modern=modern)
                self.assertEqual(result.returncode, 0, result.stderr)

    @unittest.skipIf(os.name == "nt", "executable fixture requires a POSIX host")
    def install_result(self, code, enabled=True):
        with tempfile.TemporaryDirectory(prefix="manticore openssl ") as temp:
            root = Path(temp)
            calls = root / "calls.json"
            command = root / "vcpkg-fixture"
            command.write_text(
                f"#!{sys.executable}\nimport json,sys\nfrom pathlib import Path\n"
                f"Path({str(calls)!r}).write_text(json.dumps(sys.argv[1:]))\n"
                f"sys.exit({code})\n")
            command.chmod(0o755)
            (root / "GetVCPKG.cmake").write_text(
                f'set(VCPKG "{command}")\n'
                'set(vcpkg_params "--triplet=arm64-linux")\n')
            script = root / "check.cmake"
            script.write_text(
                f'set(WITH_SSL {"ON" if enabled else "OFF"})\n'
                f'set(MANTICORE_SOURCE_DIR "{root}")\n'
                f'set(CMAKE_MODULE_PATH "{root}")\n'
                f'include("{REPO / "cmake/GetSSL.cmake"}")\n')
            result = subprocess.run([CMAKE, "-P", str(script)], capture_output=True, text=True)
            arguments = json.loads(calls.read_text()) if calls.exists() else None
            expected = f"--overlay-ports={root}/cmake/vcpkg-overlay-ports"
            return result, arguments, expected

    def test_overlay_and_triplet_passed_as_separate_arguments(self):
        result, arguments, overlay = self.install_result(0)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(arguments, ["install", "--triplet=arm64-linux", overlay, "openssl"])

    def test_failed_install_cannot_fall_back_to_old_library(self):
        result, arguments, _ = self.install_result(7)
        self.assertNotEqual(result.returncode, 0)
        self.assertIsNotNone(arguments)
        self.assertIn("refusing to fall back", result.stderr)

    def test_disabled_ssl_skips_install(self):
        result, arguments, _ = self.install_result(7, enabled=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIsNone(arguments)


if __name__ == "__main__":
    unittest.main(verbosity=2)
