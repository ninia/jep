import unittest

import jep
Test = jep.findClass('jep.test.Test')
Boolean = jep.findClass('java.lang.Boolean')
StringBuilder = jep.findClass('java.lang.StringBuilder')
ArrayList = jep.findClass('java.util.ArrayList')

class TestTypes(unittest.TestCase):
    def setUp(self):
        self.test = Test()

    def test_string(self):
        self.assertEqual("toString(). Thanks for calling Java(tm).", self.test.toString())

    def test_enum(self):
        testEnum = self.test.getEnum()
        self.assertEqual(0, testEnum.ordinal())

    def test_long(self):
        self.assertEqual(9223372036854775807, self.test.getClassLong())

    def test_double(self):
        self.assertEqual(4.9E-324, self.test.getClassDouble())

    def test_float(self):
        self.assertAlmostEqual(3.4028234663852886e+38, self.test.getClassFloat())

    def test_intobj(self):
        self.assertEqual(-2147483648, self.test.getInteger())

    def test_boolobj(self):
        self.assertTrue(self.test.testBooleanObject(True))
        self.assertFalse(self.test.testBooleanObject(False))

    def test_charobj(self):
        self.assertEquals(self.test.getCharacterField(), 'c')

    def test_getobj(self):
        obj = self.test.getObject()
        self.assertEqual("list 0", str(obj.get(0)))

    def test_stringasobject(self):
        self.assertEqual(self.test.getStringField(), self.test.getObjectStringField())

    def test_getstring_array(self):
        obj = self.test.getStringArray()
        self.assertEqual('one', obj[0])
        self.assertEqual('two', obj[1])
        self.assertEqual('one two', ' '.join(obj))

    def test_string_string_array(self):
        obj = self.test.getStringStringArray()
        self.assertEqual('one', obj[0][0])

    def test_int_array(self):
        obj = self.test.getIntArray()
        self.assertEqual(1, obj[0])

    def test_bool_array(self):
        obj = self.test.getBooleanArray()
        self.assertTrue(obj[1])

    def test_short_array(self):
        obj = self.test.getShortArray()
        self.assertEqual(123, obj[0])

    def test_float_array(self):
        obj = self.test.getFloatArray()
        self.assertAlmostEqual(123.12300109863281, obj[0])

    def test_object_array(self):
        obj = self.test.getObjectArray()
        self.assertEqual(self.test.toString(), obj[0].toString())

    def test_equals(self):
        self.assertTrue(self.test.getClass() == Test)
        from java.lang import Class, String, Integer
        self.assertFalse(self.test.getClass() == Class)
        self.assertEqual(String('one'), String('one'))
        self.assertTrue(String('1') == String('1'))
        self.assertEqual(self.test, self.test)
        self.assertNotEqual(self.test, Test())
        self.assertNotEqual(String('two'), String('one'))

        self.assertEqual(String, String)
        self.assertNotEqual(String, Integer)

    def test_boxing(self):
        self.assertEquals(self.test.testObjectPassThrough(True), True)
        self.assertEquals(self.test.testObjectPassThrough(1), 1)
        self.assertEquals(self.test.testObjectPassThrough(1.5), 1.5)
        self.assertEquals(self.test.testObjectPassThrough("string"), "string")

    def test_overload(self):
        builder = StringBuilder()
        builder.append(1)
        self.assertTrue(builder.toString() == "1")
        builder = StringBuilder()
        builder.append(StringBuilder)
        self.assertTrue(builder.toString() == "class java.lang.StringBuilder")
        list = ArrayList()
        list.add("One")
        list.add("Two")
        list.add("Three")
        list.remove(1)
        self.assertEqual(list.size(), 2)
