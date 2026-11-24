"""The KGSL clock floor's beat (dm/peanuts/peanuts_gpufloor.c) is off unless PN_GPU_FLOOR_BEAT=1: with it on, the 0.1.8 release on the AYN Thor lost the images of
the app's own windows (the second screen's stats page, the in-game panel) for single frames a few times a second. Every read of the switch takes the one default,
and docs/env_switches.tsv says the same default."""
import re
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SRC = REPO / 'dm/peanuts/peanuts_gpufloor.c'
SWITCHES = REPO / 'docs/env_switches.tsv'


class BeatOffByDefault(unittest.TestCase):
    def test_default_is_off(self):
        src = SRC.read_text(encoding='utf-8')
        m = re.search(r'enum \{ BEAT_DEFAULT = (\d+) \}', src)
        self.assertIsNotNone(m, 'BEAT_DEFAULT in peanuts_gpufloor.c')
        self.assertEqual(m.group(1), '0')

    def test_every_read_takes_that_default(self):
        src = SRC.read_text(encoding='utf-8')
        reads = re.findall(r'env_int\("PN_GPU_FLOOR_BEAT",\s*([^)]+)\)', src)
        self.assertGreaterEqual(len(reads), 2, 'the set-up and the beat read the switch')
        self.assertEqual(set(r.strip() for r in reads), {'BEAT_DEFAULT'})

    def test_switch_row_says_default_off(self):
        row = next(l for l in SWITCHES.read_text(encoding='utf-8').splitlines() if l.startswith('PN_GPU_FLOOR_BEAT\t'))
        self.assertIn('default 0', row)
        self.assertNotIn('default 1', row)


if __name__ == '__main__':
    unittest.main()
