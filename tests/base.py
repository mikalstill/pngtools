"""Base test case for pngtools tests."""

import os
import subprocess
import types

import fixtures
import testtools


# Project root is the parent of the tests/ directory
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(
    __file__)))

# Paths to the sample images shipped in the repository
SAMPLE_DIR = PROJECT_ROOT

# Path to generated test images
TESTDATA_DIR = os.path.join(PROJECT_ROOT, 'testdata')


def _ensure_test_images():
    """Generate test images if they don't exist yet."""
    marker = os.path.join(TESTDATA_DIR, 'with_text.png')
    if not os.path.exists(marker):
        script = os.path.join(
            PROJECT_ROOT, 'tests', 'generate_test_images.py'
        )
        subprocess.run(
            ['python3', script],
            check=True,
            capture_output=True
        )


class PngtoolsTestCase(testtools.TestCase):
    """Base test case with helpers for running pngtools binaries."""

    def setUp(self):
        super().setUp()
        self.tmpdir = self.useFixture(fixtures.TempDir())

        # Verify binaries are built
        for binary in ['pnginfo', 'pngchunks', 'pngchunkdesc',
                       'pngcp']:
            path = os.path.join(PROJECT_ROOT, binary)
            if not os.path.exists(path):
                self.skipTest(
                    f'{binary} not built; run make first'
                )

        # Ensure generated test images exist
        _ensure_test_images()

        # Create an invalid (non-PNG) file for error tests
        self.invalid_file = os.path.join(
            self.tmpdir.path, 'invalid.txt'
        )
        with open(self.invalid_file, 'w') as f:
            f.write('this is not a png file')

    def run_tool(self, name, args=None, stdin_data=None):
        """Run a pngtools binary and return the result.

        Returns a SimpleNamespace with .stdout, .stderr,
        and .returncode.
        """
        if args is None:
            args = []

        binary = os.path.join(PROJECT_ROOT, name)
        cmd = [binary] + args

        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            input=stdin_data,
            cwd=PROJECT_ROOT,
            timeout=30
        )

        return types.SimpleNamespace(
            stdout=result.stdout,
            stderr=result.stderr,
            returncode=result.returncode
        )

    def run_pnginfo(self, filename, flags=None):
        """Convenience wrapper for running pnginfo."""
        if flags is None:
            flags = []
        return self.run_tool('pnginfo', flags + [filename])

    def sample_path(self, filename):
        """Return the full path to a sample PNG in the repo."""
        return os.path.join(SAMPLE_DIR, filename)

    def generated_path(self, filename):
        """Return the full path to a generated test PNG."""
        return os.path.join(TESTDATA_DIR, filename)

    def tmp_path(self, filename):
        """Return a path in the temp directory for output."""
        return os.path.join(self.tmpdir.path, filename)
