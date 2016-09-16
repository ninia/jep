from __future__ import print_function
import unittest


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
