"""Tests for the pngchunks tool."""

from tests import base


class TestPngchunks(base.PngtoolsTestCase):
    """Test pngchunks chunk listing."""

    def test_lists_ihdr(self):
        """Output contains an IHDR chunk."""
        result = self.run_tool(
            'pngchunks', [self.sample_path('input.png')]
        )
        self.assertEqual(0, result.returncode)
        self.assertIn('[IHDR]', result.stdout)

    def test_lists_iend(self):
        """Output contains an IEND chunk."""
        result = self.run_tool(
            'pngchunks', [self.sample_path('input.png')]
        )
        self.assertIn('[IEND]', result.stdout)

    def test_lists_idat(self):
        """Output contains at least one IDAT chunk."""
        result = self.run_tool(
            'pngchunks', [self.sample_path('input.png')]
        )
        self.assertIn('[IDAT]', result.stdout)

    def test_ihdr_dimensions(self):
        """IHDR chunk shows correct width and height."""
        result = self.run_tool(
            'pngchunks', [self.sample_path('input.png')]
        )
        self.assertIn('IHDR Width: 256', result.stdout)
        self.assertIn('IHDR Height: 256', result.stdout)

    def test_ihdr_bitdepth(self):
        """IHDR chunk shows correct bitdepth."""
        result = self.run_tool(
            'pngchunks', [self.sample_path('input.png')]
        )
        self.assertIn('IHDR Bitdepth: 8', result.stdout)

    def test_chunk_properties_critical(self):
        """IHDR is identified as critical and public."""
        result = self.run_tool(
            'pngchunks', [self.sample_path('input.png')]
        )
        # The line after IHDR should describe its properties
        self.assertIn('Critical, public', result.stdout)

    def test_ancillary_chunks(self):
        """input.png has ancillary chunks like bKGD or pHYs."""
        result = self.run_tool(
            'pngchunks', [self.sample_path('input.png')]
        )
        # input.png is known to have bKGD, pHYs, tIME
        has_ancillary = (
            '[bKGD]' in result.stdout
            or '[pHYs]' in result.stdout
            or '[tIME]' in result.stdout
        )
        self.assertTrue(
            has_ancillary,
            'Expected ancillary chunks in output'
        )

    def test_compression_algorithm(self):
        """IHDR compression algorithm is identified as Deflate."""
        result = self.run_tool(
            'pngchunks', [self.sample_path('input.png')]
        )
        self.assertIn(
            'Compression algorithm is Deflate', result.stdout
        )

    def test_chunk_crc(self):
        """Each chunk has a CRC value displayed."""
        result = self.run_tool(
            'pngchunks', [self.sample_path('input.png')]
        )
        self.assertIn('Chunk CRC:', result.stdout)

    def test_iend_no_data(self):
        """IEND chunk reports containing no data."""
        result = self.run_tool(
            'pngchunks', [self.sample_path('input.png')]
        )
        self.assertIn('IEND contains no data', result.stdout)


class TestPngchunksErrors(base.PngtoolsTestCase):
    """Test pngchunks error handling."""

    def test_no_arguments(self):
        """No arguments prints usage and exits 1."""
        result = self.run_tool('pngchunks')
        self.assertEqual(1, result.returncode)
        self.assertIn('Usage:', result.stderr)

    def test_missing_file(self):
        """Missing file prints error and exits 1."""
        result = self.run_tool(
            'pngchunks', ['/nonexistent/file.png']
        )
        self.assertEqual(1, result.returncode)
        self.assertIn('Could not open', result.stderr)

    def test_invalid_file(self):
        """Non-PNG file prints error and exits 1."""
        result = self.run_tool(
            'pngchunks', [self.invalid_file]
        )
        self.assertEqual(1, result.returncode)
        self.assertIn('not a PNG file', result.stderr)
