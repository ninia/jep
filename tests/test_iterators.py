import unittest
import sys

class TestIterators(unittest.TestCase):
    def setUp(self):
        pass
    
    def test_iteration(self):
        from java.util import ArrayList
        x = ArrayList()
        x.add("abc")
        x.add("adef")
        x.add("ahi")
        itr = x.iterator()
        for i in x:
            self.assertIn("a", i)

    @unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
    def test_iter_itr_crash(self):
        from .jep_pipe import jep_pipe
        with jep_pipe(['jep', 'tests/iter_itr_crash.py']) as p:
            self.assertEqual(next(p), 'success: no crash\n')
