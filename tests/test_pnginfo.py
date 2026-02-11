"""Tests for the pnginfo tool."""

from tests import base


class TestPnginfoBasicMetadata(base.PngtoolsTestCase):
    """Test pnginfo metadata display for various image types."""

    def test_rgb_image(self):
        """sample.png: 640x480 8-bit RGB."""
        result = self.run_pnginfo(self.sample_path('sample.png'))
        self.assertEqual(0, result.returncode)
        self.assertIn('Image Width: 640', result.stdout)
        self.assertIn('Image Length: 480', result.stdout)
        self.assertIn('Channels (Samples/Pixel): 3', result.stdout)
        self.assertIn(
            'Colour Type (Photometric Interpretation): RGB',
            result.stdout
        )

    def test_rgba_image(self):
        """foursamplesperpixel.png: 32x32 8-bit RGBA."""
        result = self.run_pnginfo(
            self.sample_path('foursamplesperpixel.png')
        )
        self.assertEqual(0, result.returncode)
        self.assertIn('Image Width: 32', result.stdout)
        self.assertIn('Image Length: 32', result.stdout)
        self.assertIn('Channels (Samples/Pixel): 4', result.stdout)
        self.assertIn('RGB with alpha channel', result.stdout)

    def test_grayscale_image(self):
        """grayscale.png: 32x32 4-bit grayscale."""
        result = self.run_pnginfo(
            self.sample_path('grayscale.png')
        )
        self.assertEqual(0, result.returncode)
        self.assertIn(
            'Bitdepth (Bits/Sample): 4', result.stdout
        )
        self.assertIn('Channels (Samples/Pixel): 1', result.stdout)
        self.assertIn('GRAYSCALE', result.stdout)

    def test_16bit_grayscale(self):
        """multibytesample.png: 32x32 16-bit grayscale."""
        result = self.run_pnginfo(
            self.sample_path('multibytesample.png')
        )
        self.assertEqual(0, result.returncode)
        self.assertIn(
            'Bitdepth (Bits/Sample): 16', result.stdout
        )
        self.assertIn('GRAYSCALE', result.stdout)

    def test_image_with_phys(self):
        """input.png has a pHYs chunk with resolution data."""
        result = self.run_pnginfo(self.sample_path('input.png'))
        self.assertEqual(0, result.returncode)
        self.assertIn('pixels per meter', result.stdout)

    def test_exit_code_success(self):
        """Successful run returns exit code 0."""
        result = self.run_pnginfo(self.sample_path('input.png'))
        self.assertEqual(0, result.returncode)

    def test_multiple_files(self):
        """Multiple filenames on command line all appear."""
        result = self.run_tool('pnginfo', [
            self.sample_path('sample.png'),
            self.sample_path('input.png')
        ])
        self.assertEqual(0, result.returncode)
        self.assertIn('sample.png', result.stdout)
        self.assertIn('input.png', result.stdout)

    def test_compression_scheme(self):
        """All PNG files show deflate compression."""
        result = self.run_pnginfo(self.sample_path('sample.png'))
        self.assertIn(
            'Compression Scheme: Deflate method 8', result.stdout
        )

    def test_fill_order(self):
        """FillOrder and byte order are always the same."""
        result = self.run_pnginfo(self.sample_path('sample.png'))
        self.assertIn('FillOrder: msb-to-lsb', result.stdout)
        self.assertIn(
            'Byte Order: Network (Big Endian)', result.stdout
        )


class TestPnginfoGeneratedImages(base.PngtoolsTestCase):
    """Test pnginfo with generated test images."""

    def test_paletted_image(self):
        """paletted.png shows PALETTED COLOUR type."""
        result = self.run_pnginfo(
            self.generated_path('paletted.png')
        )
        self.assertEqual(0, result.returncode)
        self.assertIn('PALETTED COLOUR', result.stdout)

    def test_interlaced_image(self):
        """interlaced.png shows Adam7 interlacing."""
        result = self.run_pnginfo(
            self.generated_path('interlaced.png')
        )
        self.assertEqual(0, result.returncode)
        self.assertIn('Adam7 interlacing', result.stdout)

    def test_text_chunks(self):
        """with_text.png shows text chunk metadata."""
        result = self.run_pnginfo(
            self.generated_path('with_text.png')
        )
        self.assertEqual(0, result.returncode)
        self.assertIn('Number of text strings: 2', result.stdout)
        self.assertIn('Author', result.stdout)
        self.assertIn('Description', result.stdout)

    def test_paletted_with_transparency(self):
        """with_transparency.png shows paletted with alpha."""
        result = self.run_pnginfo(
            self.generated_path('with_transparency.png')
        )
        self.assertEqual(0, result.returncode)
        self.assertIn('PALETTED COLOUR', result.stdout)


class TestPnginfoTiffMode(base.PngtoolsTestCase):
    """Test pnginfo -t (tiffinfo compatible labels)."""

    def test_tiff_labels(self):
        """-t uses tiffinfo-style label names."""
        result = self.run_pnginfo(
            self.sample_path('input.png'), flags=['-t']
        )
        self.assertEqual(0, result.returncode)
        self.assertIn('Bits/Sample:', result.stdout)
        self.assertIn('Samples/Pixel:', result.stdout)
        self.assertIn('Pixel Depth:', result.stdout)
        # Should NOT contain the default PNG-style labels
        self.assertNotIn('Bitdepth (Bits/Sample)', result.stdout)
        self.assertNotIn(
            'Channels (Samples/Pixel)', result.stdout
        )

    def test_tiff_mode_header(self):
        """-t includes compatibility note in filename header."""
        result = self.run_pnginfo(
            self.sample_path('input.png'), flags=['-t']
        )
        self.assertIn(
            '(tiffinfo compatible labels)', result.stdout
        )


class TestPnginfoBitmap(base.PngtoolsTestCase):
    """Test pnginfo -d (dump bitmap) and -D (verify bitmap)."""

    def test_dump_bitmap(self):
        """-d dumps hex bitmap data."""
        result = self.run_pnginfo(
            self.sample_path('grayscale.png'), flags=['-d']
        )
        self.assertEqual(0, result.returncode)
        self.assertIn('Dumping the bitmap', result.stdout)
        # Bitmap contains hex pixel values in brackets
        self.assertIn('[', result.stdout)

    def test_dump_bitmap_format(self):
        """-d output includes pixel format description."""
        result = self.run_pnginfo(
            self.sample_path('grayscale.png'), flags=['-d']
        )
        self.assertIn('bytes per pixel', result.stdout)
        self.assertIn('channels', result.stdout)

    def test_verify_bitmap(self):
        """-D extracts bitmap without displaying it."""
        result = self.run_pnginfo(
            self.sample_path('sample.png'), flags=['-D']
        )
        self.assertEqual(0, result.returncode)
        # Metadata should appear but not bitmap dump
        self.assertIn('Image Width:', result.stdout)
        self.assertNotIn('Dumping the bitmap', result.stdout)


class TestPnginfoErrors(base.PngtoolsTestCase):
    """Test pnginfo error handling."""

    def test_no_arguments(self):
        """No arguments prints usage and exits 1."""
        result = self.run_tool('pnginfo')
        self.assertEqual(1, result.returncode)
        self.assertIn('Usage:', result.stderr)

    def test_missing_file(self):
        """Missing file prints error and exits 1."""
        result = self.run_pnginfo('/nonexistent/file.png')
        self.assertEqual(1, result.returncode)
        self.assertIn('Could not open', result.stderr)

    def test_invalid_file(self):
        """Non-PNG file prints error and exits 1."""
        result = self.run_pnginfo(self.invalid_file)
        self.assertEqual(1, result.returncode)
        self.assertIn('not a valid PNG', result.stderr)
