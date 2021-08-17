from __future__ import print_function
import unittest
import sys

from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd
import jep
TestFieldTypes = jep.findClass('jep.test.types.TestFieldTypes')

class TestRegressions(unittest.TestCase):

    def test_out(self):
        from java.lang import System
        System.out.print("")

    def test_byte_value(self):
        from java.lang import Integer
        try:
            # call instance from class, should throw but not crash
            Integer.byteValue()
        except:
            pass

    def test_static_access_to_nonstatic_field(self):
        with self.assertRaises(TypeError):
            TestFieldTypes.objectString = ""

    def test_close_with_threads(self):
        jep_pipe(build_java_process_cmd('jep.test.TestCloseWithThreads'))

