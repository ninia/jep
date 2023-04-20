import unittest
import jep

TestPyBuiltinsJava = jep.findClass('jep.test.TestPyBuiltins')

class TestPyBuiltins(unittest.TestCase):

    def test_pybuiltins(self):
        self.assertEqual(None, TestPyBuiltinsJava.test())
