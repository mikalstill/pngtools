"""Tests for the pngchunkdesc tool."""

from tests import base


class TestPngchunkdesc(base.PngtoolsTestCase):
    """Test pngchunkdesc chunk name decoding."""

    def test_critical_public_chunk(self):
        """IHDR: all uppercase = critical, public, compliant,
        unsafe."""
        result = self.run_tool(
            'pngchunkdesc', stdin_data='IHDR\n'
        )
        self.assertEqual(0, result.returncode)
        self.assertIn('Critical', result.stdout)
        self.assertIn('public', result.stdout)
        self.assertIn('PNG 1.2 compliant', result.stdout)
        self.assertIn('unsafe to copy', result.stdout)

    def test_ancillary_safe_chunk(self):
        """tEXt: lowercase 1st = ancillary, lowercase 4th =
        safe to copy."""
        result = self.run_tool(
            'pngchunkdesc', stdin_data='tEXt\n'
        )
        self.assertIn('Ancillary', result.stdout)
        self.assertIn('public', result.stdout)
        self.assertIn('safe to copy', result.stdout)

    def test_private_chunk(self):
        """tPRv: lowercase 2nd letter = private."""
        result = self.run_tool(
            'pngchunkdesc', stdin_data='tpRv\n'
        )
        self.assertIn('private', result.stdout)

    def test_reserved_chunk_space(self):
        """IHdR: lowercase 3rd letter = reserved chunk space."""
        result = self.run_tool(
            'pngchunkdesc', stdin_data='IHdR\n'
        )
        self.assertIn('in reserved chunk space', result.stdout)

    def test_iccp_chunk(self):
        """iCCP: ancillary, public, compliant, unsafe."""
        result = self.run_tool(
            'pngchunkdesc', stdin_data='iCCP\n'
        )
        self.assertIn('Ancillary', result.stdout)
        self.assertIn('public', result.stdout)
        self.assertIn('PNG 1.2 compliant', result.stdout)
        self.assertIn('unsafe to copy', result.stdout)

    def test_multiple_inputs(self):
        """Multiple chunk names produce multiple output lines."""
        result = self.run_tool(
            'pngchunkdesc', stdin_data='IHDR\ntEXt\n'
        )
        lines = result.stdout.strip().split('\n')
        self.assertEqual(2, len(lines))
        self.assertIn('IHDR:', lines[0])
        self.assertIn('tEXt:', lines[1])

    def test_output_format(self):
        """Output format is 'NAME: prop1, prop2, prop3, prop4'."""
        result = self.run_tool(
            'pngchunkdesc', stdin_data='PLTE\n'
        )
        line = result.stdout.strip()
        self.assertTrue(line.startswith('PLTE:'))
        # Should have exactly 4 comma-separated properties
        props = line.split(': ', 1)[1]
        self.assertEqual(3, props.count(','))
