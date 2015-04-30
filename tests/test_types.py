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
        self.assertEquals(True, self.test.booleanField)
        self.assertEquals(True, self.test.isBooleanField())
        self.test.booleanField = False
        self.assertEquals(False, self.test.booleanField)
        self.assertEquals(False, self.test.isBooleanField())

    def test_short_field(self):
        self.assertEquals(321, self.test.shortField)
        self.assertEquals(321, self.test.getShortField())
        self.test.shortField = 123
        self.assertEquals(123, self.test.shortField)
        self.assertEquals(123, self.test.getShortField())

    def test_int_field(self):
        self.assertEquals(123, self.test.intField)
        self.assertEquals(123, self.test.getIntField())
        self.test.intField = 345
        self.assertEquals(345, self.test.intField)
        self.assertEquals(345, self.test.getIntField())

    def test_long_field(self):
        self.assertEquals(9223372036854775807, self.test.longField)
        self.assertEquals(9223372036854775807, self.test.getLongField())
        self.test.longField = 2
        self.assertEquals(2, self.test.longField)
        self.assertEquals(2, self.test.getLongField())

    def test_double_field(self):
        self.assertEquals(123.123, self.test.doubleField)
        self.assertEquals(123.123, self.test.getDoubleField())
        self.test.doubleField = .3
        self.assertEquals(.3, self.test.doubleField)
        self.assertEquals(.3, self.test.getDoubleField())

    def test_float_field(self):
        self.assertAlmostEquals(3.4028234663852886e+38, self.test.floatField)
        self.assertAlmostEquals(3.4028234663852886e+38, self.test.getFloatField())
        self.test.floatField = .1
        self.assertAlmostEquals(.1, self.test.floatField)
        self.assertAlmostEquals(.1, self.test.getFloatField())

    def test_byte_field(self):
        self.assertEquals(43, self.test.byteField)
        self.assertEquals(43, self.test.getByteField())
        self.test.byteField = 2
        self.assertEquals(2, self.test.byteField)
        self.assertEquals(2, self.test.getByteField())

    def test_char_field(self):
        self.assertEquals('c', self.test.charField)
        self.assertEquals('c', self.test.getCharField())
        self.test.charField = 'a'
        self.assertEquals('a', self.test.charField)
        self.assertEquals('a', self.test.getCharField())

    def test_class_field(self):
        self.assertIsNotNone(self.test.classField)
        self.assertIsNotNone(self.test.getClassField())
        self.test.classField = Integer
        self.assertEquals(Integer, self.test.classField)
        self.assertEquals(Integer, self.test.getClassField())

    def test_static_string(self):
        self.assertEqual('stringField', self.test.staticString)
        self.assertEqual('stringField', self.test.getStaticString())
        self.test.staticString = 'asdf'
        self.assertEqual('asdf', self.test.staticString)
        self.assertEqual('asdf', self.test.getStaticString())

    def test_static_boolean_field(self):
        self.assertEquals(True, self.test.staticBoolean)
        self.assertEquals(True, self.test.isStaticBoolean())
        self.test.staticBoolean = False
        self.assertEquals(False, self.test.staticBoolean)
        self.assertEquals(False, self.test.isStaticBoolean())

    def test_static_short_field(self):
        self.assertEquals(321, self.test.staticShort)
        self.assertEquals(321, self.test.getStaticShort())
        self.test.staticShort = 123
        self.assertEquals(123, self.test.staticShort)
        self.assertEquals(123, self.test.getStaticShort())

    def test_static_int_field(self):
        self.assertEquals(123, self.test.staticInt)
        self.assertEquals(123, self.test.getStaticInt())
        self.test.staticInt = 345
        self.assertEquals(345, self.test.staticInt)
        self.assertEquals(345, self.test.getStaticInt())

    def test_static_long_field(self):
        self.assertEquals(9223372036854775807, self.test.staticLong)
        self.assertEquals(9223372036854775807, self.test.getStaticLong())
        self.test.staticLong = 2
        self.assertEquals(2, self.test.staticLong)
        self.assertEquals(2, self.test.getStaticLong())

    def test_static_double_field(self):
        self.assertEquals(123.123, self.test.staticDouble)
        self.assertEquals(123.123, self.test.getStaticDouble())
        self.test.staticDouble = .3
        self.assertEquals(.3, self.test.staticDouble)
        self.assertEquals(.3, self.test.getStaticDouble())

    def test_static_float_field(self):
        self.assertAlmostEquals(3.4028234663852886e+38, self.test.staticFloat)
        self.assertAlmostEquals(3.4028234663852886e+38, self.test.getStaticFloat())
        self.test.staticFloat = .1
        self.assertAlmostEquals(.1, self.test.staticFloat)
        self.assertAlmostEquals(.1, self.test.getStaticFloat())

    def test_static_byte_field(self):
        self.assertEquals(125, self.test.staticByte)
        self.assertEquals(125, self.test.getStaticByte())
        self.test.staticByte = 2
        self.assertEquals(2, self.test.staticByte)
        self.assertEquals(2, self.test.getStaticByte())

    def test_static_char_field(self):
        self.assertEquals('j', self.test.staticChar)
        self.assertEquals('j', self.test.getStaticChar())
        self.test.staticChar = 'a'
        self.assertEquals('a', self.test.staticChar)
        self.assertEquals('a', self.test.getStaticChar())

    def test_static_class_field(self):
        self.assertNotEquals(None, self.test.staticClass)
        self.assertNotEquals(None, self.test.getStaticClass())
        self.test.staticClass = Integer
        self.assertEquals(Integer, self.test.staticClass)
        self.assertNotEquals(Integer, self.test.getStaticClass())

    def test_equals(self):
        self.assertEqual(self.test, self.test)
        self.assertNotEqual(self.test, self.test.getObject().get(0))
        self.assertNotEqual(Integer, self.test)
        self.assertNotEqual(self.test, Integer)
        self.assertEqual(Integer, Integer)
        from java.lang import Class
        self.assertNotEqual(Integer, Class)

