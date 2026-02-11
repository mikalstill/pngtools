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

    def test_reduce_16bit_to_8bit(self):
        """16-bit grayscale reduced to 8-bit."""
        output = self.tmp_path('reduced.png')
        result = self.run_tool('pngcp', [
            '-d', '8',
            self.sample_path('multibytesample.png'), output
        ])
        self.assertEqual(0, result.returncode)

        info = self.run_pnginfo(output)
        self.assertIn('Bitdepth (Bits/Sample): 8', info.stdout)
        self.assertIn('Channels (Samples/Pixel): 1', info.stdout)

    def test_combined_bitdepth_and_channels(self):
        """Combined bitdepth and channel change in one pass."""
        output = self.tmp_path('combined.png')
        result = self.run_tool('pngcp', [
            '-d', '16', '-s', '3',
            self.sample_path('grayscale.png'), output
        ])
        self.assertEqual(0, result.returncode)

        info = self.run_pnginfo(output)
        self.assertIn('Bitdepth (Bits/Sample): 16', info.stdout)
        self.assertIn('Channels (Samples/Pixel): 3', info.stdout)

    def test_add_alpha_channel(self):
        """Adding alpha channel to RGB image."""
        output = self.tmp_path('with_alpha.png')
        result = self.run_tool('pngcp', [
            '-s', '4',
            self.sample_path('input.png'), output
        ])
        self.assertEqual(0, result.returncode)

        info = self.run_pnginfo(output)
        self.assertIn('Channels (Samples/Pixel): 4', info.stdout)
        self.assertIn('RGB with alpha channel', info.stdout)

    def test_gray_to_rgb(self):
        """Grayscale to RGB channel expansion."""
        output = self.tmp_path('gray_to_rgb.png')
        result = self.run_tool('pngcp', [
            '-s', '3',
            self.sample_path('grayscale.png'), output
        ])
        self.assertEqual(0, result.returncode)

        info = self.run_pnginfo(output)
        self.assertIn('Channels (Samples/Pixel): 3', info.stdout)
        self.assertIn('RGB', info.stdout)

    def test_drop_alpha_channel(self):
        """Dropping alpha channel from RGBA image."""
        output = self.tmp_path('no_alpha.png')
        result = self.run_tool('pngcp', [
            '-s', '3',
            self.sample_path('foursamplesperpixel.png'), output
        ])
        self.assertEqual(0, result.returncode)

        info = self.run_pnginfo(output)
        self.assertIn('Channels (Samples/Pixel): 3', info.stdout)


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
