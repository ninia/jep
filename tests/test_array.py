# reads this file
import unittest
from jep import jarray, JINT_ID, JBYTE_ID


class TestArray(unittest.TestCase):

    def test_slice(self):
        ar = jarray(20, JINT_ID, 0)
        ar[0] = 12
        ar[1] = 22
        self.assertEqual([12, 22], list(ar[0:2]))

    def test_read_file(self):
        from java.io import FileInputStream
        from java.lang import String

        fin = FileInputStream(__file__)
        ar = jarray(20, JBYTE_ID)
        count = fin.read(ar)
        s = String(ar, 0, count)
        self.assertTrue(str(s).startswith('# reads this file'))
