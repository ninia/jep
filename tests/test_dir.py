import unittest


class TestDir(unittest.TestCase):
    def setUp(self):
        pass
    
    def test_dir_list(self):
        from java.util import ArrayList
        x = ArrayList()
        d = dir(x)
        self.assertIn("add", d)
        self.assertIn("addAll", d)
        self.assertIn("clear", d)
        self.assertIn("contains", d)
        self.assertIn("containsAll", d)
        self.assertIn("equals", d)
        self.assertIn("hashCode", d)
        self.assertIn("isEmpty", d)
        self.assertIn("iterator", d)
        self.assertIn("remove", d)
        self.assertIn("removeAll", d)
        self.assertIn("size", d)

        self.assertIn("ensureCapacity", d)
