import unittest

import jep
Test = jep.findClass('jep.test.Test')
from java.lang import Integer

class TestFields(unittest.TestCase):
    def setUp(self):
        self.test = Test()

    def test_string(self):
        self.assertEqual('a stringField', self.test.stringField)
        self.test.stringField = 'asdf'
        self.assertEqual('asdf', self.test.stringField)
        
    def test_boolean_field(self):
        self.assertEqual(True, self.test.booleanField)
        self.assertEqual(True, self.test.isBooleanField())
        self.test.booleanField = False
        self.assertEqual(False, self.test.booleanField)
        self.assertEqual(False, self.test.isBooleanField())

    def test_short_field(self):
        self.assertEqual(321, self.test.shortField)
        self.assertEqual(321, self.test.getShortField())
        self.test.shortField = 123
        self.assertEqual(123, self.test.shortField)
        self.assertEqual(123, self.test.getShortField())

    def test_int_field(self):
        self.assertEqual(123, self.test.intField)
        self.assertEqual(123, self.test.getIntField())
        self.test.intField = 345
        self.assertEqual(345, self.test.intField)
        self.assertEqual(345, self.test.getIntField())

    def test_long_field(self):
        self.assertEqual(9223372036854775807, self.test.longField)
        self.assertEqual(9223372036854775807, self.test.getLongField())
        self.test.longField = 2
        self.assertEqual(2, self.test.longField)
        self.assertEqual(2, self.test.getLongField())

    def test_double_field(self):
        self.assertEqual(123.123, self.test.doubleField)
        self.assertEqual(123.123, self.test.getDoubleField())
        self.test.doubleField = .3
        self.assertEqual(.3, self.test.doubleField)
        self.assertEqual(.3, self.test.getDoubleField())

    def test_float_field(self):
        self.assertAlmostEqual(3.4028234663852886e+38, self.test.floatField)
        self.assertAlmostEqual(3.4028234663852886e+38, self.test.getFloatField())
        self.test.floatField = .1
        self.assertAlmostEqual(.1, self.test.floatField)
        self.assertAlmostEqual(.1, self.test.getFloatField())

    def test_byte_field(self):
        self.assertEqual(43, self.test.byteField)
        self.assertEqual(43, self.test.getByteField())
        self.test.byteField = 2
        self.assertEqual(2, self.test.byteField)
        self.assertEqual(2, self.test.getByteField())

    def test_char_field(self):
        self.assertEqual('c', self.test.charField)
        self.assertEqual('c', self.test.getCharField())
        self.test.charField = 'a'
        self.assertEqual('a', self.test.charField)
        self.assertEqual('a', self.test.getCharField())

    def test_class_field(self):
        self.assertIsNotNone(self.test.classField)
        self.assertIsNotNone(self.test.getClassField())
        self.test.classField = Integer
        self.assertEqual(Integer, self.test.classField)
        self.assertEqual(Integer, self.test.getClassField())

    def test_static_string(self):
        self.assertEqual('stringField', self.test.staticString)
        self.assertEqual('stringField', self.test.getStaticString())
        self.test.staticString = 'asdf'
        self.assertEqual('asdf', self.test.staticString)
        self.assertEqual('asdf', self.test.getStaticString())

    def test_static_boolean_field(self):
        self.assertEqual(True, self.test.staticBoolean)
        self.assertEqual(True, self.test.isStaticBoolean())
        self.test.staticBoolean = False
        self.assertEqual(False, self.test.staticBoolean)
        self.assertEqual(False, self.test.isStaticBoolean())

    def test_static_short_field(self):
        self.assertEqual(321, self.test.staticShort)
        self.assertEqual(321, self.test.getStaticShort())
        self.test.staticShort = 123
        self.assertEqual(123, self.test.staticShort)
        self.assertEqual(123, self.test.getStaticShort())

    def test_static_int_field(self):
        self.assertEqual(123, self.test.staticInt)
        self.assertEqual(123, self.test.getStaticInt())
        self.test.staticInt = 345
        self.assertEqual(345, self.test.staticInt)
        self.assertEqual(345, self.test.getStaticInt())

    def test_static_long_field(self):
        self.assertEqual(9223372036854775807, self.test.staticLong)
        self.assertEqual(9223372036854775807, self.test.getStaticLong())
        self.test.staticLong = 2
        self.assertEqual(2, self.test.staticLong)
        self.assertEqual(2, self.test.getStaticLong())

    def test_static_double_field(self):
        self.assertEqual(123.123, self.test.staticDouble)
        self.assertEqual(123.123, self.test.getStaticDouble())
        self.test.staticDouble = .3
        self.assertEqual(.3, self.test.staticDouble)
        self.assertEqual(.3, self.test.getStaticDouble())

    def test_static_float_field(self):
        self.assertAlmostEqual(3.4028234663852886e+38, self.test.staticFloat)
        self.assertAlmostEqual(3.4028234663852886e+38, self.test.getStaticFloat())
        self.test.staticFloat = .1
        self.assertAlmostEqual(.1, self.test.staticFloat)
        self.assertAlmostEqual(.1, self.test.getStaticFloat())

    def test_static_byte_field(self):
        self.assertEqual(125, self.test.staticByte)
        self.assertEqual(125, self.test.getStaticByte())
        self.test.staticByte = 2
        self.assertEqual(2, self.test.staticByte)
        self.assertEqual(2, self.test.getStaticByte())

    def test_static_char_field(self):
        self.assertEqual('j', self.test.staticChar)
        self.assertEqual('j', self.test.getStaticChar())
        self.test.staticChar = 'a'
        self.assertEqual('a', self.test.staticChar)
        self.assertEqual('a', self.test.getStaticChar())

    def test_static_class_field(self):
        self.assertNotEqual(None, self.test.staticClass)
        self.assertNotEqual(None, self.test.getStaticClass())
        self.test.staticClass = Integer
        self.assertEqual(Integer, self.test.staticClass)
        self.assertNotEqual(Integer, self.test.getStaticClass())

    def test_equals(self):
        self.assertEqual(self.test, self.test)
        self.assertNotEqual(self.test, self.test.getObject().get(0))
        self.assertNotEqual(Integer, self.test)
        self.assertNotEqual(self.test, Integer)
        self.assertEqual(Integer, Integer)
        from java.lang import Class
        self.assertNotEqual(Integer, Class)

