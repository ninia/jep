import unittest
from java.lang import Object


class TestObject(unittest.TestCase):

    def test_hash(self):
        o = Object()
        self.assertTrue(isinstance(hash(o), int))

    def test_str(self):
        o = Object()
        self.assertIn('java.lang.Object@', str(o))

    def test_del_throws_exception(self):
        o = Object()
        with self.assertRaises(TypeError):
            del o.equals
