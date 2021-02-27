import os
import unittest

from jep import jarray
from jep import JBOOLEAN_ID, JCHAR_ID
from jep import JBYTE_ID, JSHORT_ID, JINT_ID, JLONG_ID
from jep import JFLOAT_ID, JDOUBLE_ID

from java.util import Arrays
from java.lang.reflect import Array
from java.lang import Boolean, Character
from java.lang import Byte, Short, Integer, Long
from java.lang import Float, Double


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

    def test_object_setitem(self):
        from java.lang import Object
        ar = jarray(1, Object)
        t = Object()
        ar[0] = t
        self.assertEqual(ar[0], t)

    def test_object_setitem_jclass(self):
        from java.lang import Object
        ar = jarray(1, Object)
        t = Object
        ar[0] = t
        self.assertEqual(ar[0], t)

    def test_object_setitem_jarray(self):
        from java.lang import Object
        ar = jarray(1, Object)
        t = jarray(1, JINT_ID)
        t[0] = 7
        ar[0] = t
        self.assertEqual(ar[0][0], t[0])

    def test_object_setitem_int(self):
        from java.lang import Object
        ar = jarray(1, Object)
        t = 1
        ar[0] = t
        self.assertEqual(ar[0], t)

    def test_object_setitem_None(self):
        from java.lang import Object
        ar = jarray(1, Object)
        t = None
        ar[0] = t
        self.assertEqual(ar[0], t)

    def test_integer_setitem_int(self):
        from java.lang import Integer
        ar = jarray(1, Integer)
        t = 1
        ar[0] = t
        self.assertEqual(ar[0], t)

    def test_integer_setitem_object_throws_exception(self):
        from java.lang import Integer, Object
        ar = jarray(1, Integer)
        t = Object()
        with self.assertRaises(TypeError):
            ar[0] = t

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

    def test_jarray_one_arg_throws_exception(self):
        with self.assertRaises(Exception):
            jep.jarray(1)

    def test_primitive_bool_array_creation(self):
        base = Array.newInstance(Boolean.TYPE, 1);
        self.assertTrue(base, jarray(1, JBOOLEAN_ID))
        self.assertTrue(base, jarray(1, 'z'))
        self.assertTrue(base, jarray(1, Boolean.TYPE))

    def test_primitive_byte_array_creation(self):
        base = Array.newInstance(Byte.TYPE, 1);
        self.assertTrue(base, jarray(1, JBYTE_ID))
        self.assertTrue(base, jarray(1, 'b'))
        self.assertTrue(base, jarray(1, Byte.TYPE))

    def test_primitive_char_array_creation(self):
        base = Array.newInstance(Character.TYPE, 1);
        self.assertTrue(base, jarray(1, JCHAR_ID))
        self.assertTrue(base, jarray(1, 'c'))
        self.assertTrue(base, jarray(1, Character.TYPE))

    def test_primitive_short_array_creation(self):
        base = Array.newInstance(Short.TYPE, 1);
        self.assertTrue(base, jarray(1, JSHORT_ID))
        self.assertTrue(base, jarray(1, 's'))
        self.assertTrue(base, jarray(1, Short.TYPE))

    def test_primitive_int_array_creation(self):
        base = Array.newInstance(Integer.TYPE, 1);
        self.assertTrue(base, jarray(1, JINT_ID))
        self.assertTrue(base, jarray(1, 'i'))
        self.assertTrue(base, jarray(1, Integer.TYPE))

    def test_primitive_long_array_creation(self):
        base = Array.newInstance(Long.TYPE, 1);
        self.assertTrue(base, jarray(1, JLONG_ID))
        self.assertTrue(base, jarray(1, 'j'))
        self.assertTrue(base, jarray(1, Long.TYPE))

    def test_primitive_float_array_creation(self):
        base = Array.newInstance(Float.TYPE, 1);
        self.assertTrue(base, jarray(1, JFLOAT_ID))
        self.assertTrue(base, jarray(1, 'f'))
        self.assertTrue(base, jarray(1, Float.TYPE))

    def test_primitive_double_array_creation(self):
        base = Array.newInstance(Double.TYPE, 1);
        self.assertTrue(base, jarray(1, JDOUBLE_ID))
        self.assertTrue(base, jarray(1, 'd'))
        self.assertTrue(base, jarray(1, Double.TYPE))
