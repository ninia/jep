import os
import unittest
from jep import jarray, JINT_ID, JBYTE_ID


class TestArray(unittest.TestCase):

    def test_initialization(self):
        ar = jarray(1, JINT_ID, 7)
        self.assertEqual(ar[0], 7)

    def test_setitem(self):
        ar = jarray(1, JINT_ID, 0)
        ar[0] = 1234
        self.assertEqual(ar[0], 1234)

    def test_setitem_out_of_bounds_throws_exception(self):
        ar = jarray(1, JINT_ID, 0)
        with self.assertRaises(IndexError):
            ar[1] = 1234

    def test_int_setitem_wrong_type_throws_exception(self):
        ar = jarray(1, JINT_ID, 0)
        with self.assertRaises(TypeError):
            ar[0] = 'fail'

    def test_slice(self):
        ar = jarray(20, JINT_ID, 0)
        py_ar = list(range(20))
        for i in py_ar:
            ar[i] = i
        self.assertEqual(py_ar[0:2], list(ar[0:2]))
        self.assertEqual(py_ar[1:-1], list(ar[1:-1]))

    def test_slice_with_step_not_1_throws_exception(self):
        ar = jarray(10, JINT_ID, 0)
        with self.assertRaises(TypeError):
            ar[::-1]
        with self.assertRaises(TypeError):
            ar[::2]

    def test_slice_out_of_bounds_handled_cleanly(self):
        ar = jarray(10, JINT_ID, 0)
        self.assertEqual(len(ar[100:]), 0)
        self.assertEqual(len(ar[:100]), 10)

    def test_string_index_conversion(self):
        from java.lang import Object
        from java.lang import String
        ar = jarray(1, Object)
        ar[0] = String("String")
        self.assertEqual(ar[0], "String")

    def test_set_string_array(self):
        from java.lang import String
        from jep.test import Test
        t = Test()
        ar = jarray(1, String)
        ar[0] = String("String")
        result = t.setStringArray(ar)
        self.assertEqual(result[0], ar[0])

    def test_read_file(self):
        from java.io import FileInputStream
        from java.lang import String

        filename = os.path.join(
            os.path.abspath(os.path.dirname(__file__)),
            'data/read_file.txt')
        fin = FileInputStream(filename)
        ar = jarray(20, JBYTE_ID)
        count = fin.read(ar)
        fin.close()
        s = String(ar, 0, count)
        self.assertEqual(str(s).strip(), 'aewrv3v')

    def test_iter(self):
        ar = jarray(20, JINT_ID, 0)
        for i in range(20):
            ar[i] = i
        for array_item, i in enumerate(ar):
            self.assertEqual(array_item, i)
