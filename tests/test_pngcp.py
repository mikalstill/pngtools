"""Tests for the pngcp tool."""

import os

from tests import base


class TestPngcp(base.PngtoolsTestCase):
    """Test pngcp image copying and transformation."""

    def test_simple_copy(self):
        """Copy input.png without changes preserves metadata."""
        output = self.tmp_path('copy.png')
        result = self.run_tool('pngcp', [
            self.sample_path('input.png'), output
        ])
        self.assertEqual(0, result.returncode)
        self.assertTrue(os.path.exists(output))

        # Verify the copy has the same dimensions
        info = self.run_pnginfo(output)
        self.assertIn('Image Width: 256', info.stdout)
        self.assertIn('Image Length: 256', info.stdout)

    def test_output_is_valid_png(self):
        """Copied file starts with PNG magic bytes."""
        output = self.tmp_path('copy.png')
        self.run_tool('pngcp', [
            self.sample_path('input.png'), output
        ])

        with open(output, 'rb') as f:
            magic = f.read(8)
        self.assertEqual(
            b'\x89PNG\r\n\x1a\n', magic
        )

    def test_change_bitdepth(self):
        """-d 16 changes the output bit depth."""
        output = self.tmp_path('depth16.png')
        result = self.run_tool('pngcp', [
            '-d', '16',
            self.sample_path('input.png'), output
        ])
        self.assertEqual(0, result.returncode)

        info = self.run_pnginfo(output)
        self.assertIn('16', info.stdout)

    def test_copy_preserves_dimensions(self):
        """Copied RGB image has the same dimensions as input."""
        output = self.tmp_path('copy2.png')
        result = self.run_tool('pngcp', [
            self.sample_path('input.png'), output
        ])
        self.assertEqual(0, result.returncode)

        info = self.run_pnginfo(output)
        self.assertIn('Image Width: 256', info.stdout)
        self.assertIn('Image Length: 256', info.stdout)


class TestPngcpErrors(base.PngtoolsTestCase):
    """Test pngcp error handling."""

    def test_no_arguments(self):
        """No arguments prints usage and exits 1."""
        result = self.run_tool('pngcp')
        self.assertEqual(1, result.returncode)
        self.assertIn('Usage:', result.stderr)

    def test_missing_input(self):
        """Missing input file prints error and exits non-zero."""
        output = self.tmp_path('out.png')
        result = self.run_tool('pngcp', [
            '/nonexistent/file.png', output
        ])
        self.assertNotEqual(0, result.returncode)

    def test_input_not_png(self):
        """Non-PNG input file prints error and exits non-zero."""
        output = self.tmp_path('out.png')
        result = self.run_tool('pngcp', [
            self.invalid_file, output
        ])
        self.assertNotEqual(0, result.returncode)
