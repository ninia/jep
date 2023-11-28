import unittest
import jep
import sys

TestSubInterpOptionsJava = jep.findClass('jep.test.TestSubInterpOptions')

class TestSubInterpOptions(unittest.TestCase):

    @unittest.skipIf(sys.version_info.major == 3 and sys.version_info.minor < 12,
            "SubInterpreterOptions not supported before Python 3.12")
    def test_sub_interp_options(self):
        self.assertEqual(None, TestSubInterpOptionsJava.test())
