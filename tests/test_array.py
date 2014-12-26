import os
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

        filename = os.path.join(
            os.path.abspath(os.path.dirname(__file__)),
            'data/read_file.txt')
        fin = FileInputStream(filename)
        ar = jarray(20, JBYTE_ID)
        count = fin.read(ar)
        s = String(ar, 0, count)
        self.assertEqual(str(s).strip(), 'aewrv3v')
