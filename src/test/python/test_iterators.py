import unittest
import sys
import jep
from jep_pipe import jep_pipe
from jep_pipe import build_python_process_cmd


class TestIterators(unittest.TestCase):

    def test_iteration(self):
        from java.util import ArrayList
        x = ArrayList()
        x.add("abc")
        x.add("adef")
        x.add("ahi")
        itr = x.iterator()
        for i in x:
            self.assertIn("a", i)

    def test_iter_itr_crash(self):
        jep_pipe(build_python_process_cmd(
            'src/test/python/subprocess/iter_itr_crash.py'))

    def test_iteratorable(self):
        TestIteratorable = jep.findClass('jep.test.TestIteratorable')
        iterator = TestIteratorable()
        iterable = TestIteratorable()
        for item in iterable:
            self.assertEquals(item, next(iterator))
