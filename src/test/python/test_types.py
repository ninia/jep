import unittest
from math import isnan
import sys
import jep
TestMethodTypes = jep.findClass('jep.test.types.TestMethodTypes')
TestStaticMethodTypes = jep.findClass('jep.test.types.TestStaticMethodTypes')
TestFieldTypes = jep.findClass('jep.test.types.TestFieldTypes')
TestStaticFieldTypes = jep.findClass('jep.test.types.TestStaticFieldTypes')
from java.lang import Object, Integer, Class


class TestTypes(unittest.TestCase):

    def setUp(self):
        self.methods = TestMethodTypes()
        self.staticMethods = TestStaticMethodTypes
        self.fields = TestFieldTypes()
        self.staticFields = TestStaticFieldTypes

    def test_boolean(self):
        for b in (True, False):
            self.assertEqual(b, self.methods.primitiveBoolean(b))
            self.assertIsInstance(self.methods.primitiveBoolean(b), bool)
            self.assertEqual(b, self.methods.objectBoolean(b))
            self.assertIsInstance(self.methods.objectBoolean(b), bool)
            self.assertEqual(b, self.methods.object(b))
            self.assertIsInstance(self.methods.object(b), bool)
            self.assertEqual(b, self.staticMethods.primitiveBoolean(b))
            self.assertIsInstance(self.staticMethods.primitiveBoolean(b), bool)
            self.assertEqual(b, self.staticMethods.objectBoolean(b))
            self.assertIsInstance(self.staticMethods.objectBoolean(b), bool)
            self.assertEqual(b, self.staticMethods.object(b))
            self.assertIsInstance(self.staticMethods.object(b), bool)
            self.fields.primitiveBoolean = b
            self.assertEqual(b, self.fields.primitiveBoolean)
            self.assertIsInstance(self.fields.primitiveBoolean, bool)
            self.fields.objectBoolean = b
            self.assertEqual(b, self.fields.objectBoolean)
            self.assertIsInstance(self.fields.objectBoolean, bool)
            self.fields.object = b
            self.assertEqual(b, self.fields.object)
            self.assertIsInstance(self.fields.object, bool)
            self.staticFields.primitiveBoolean = b
            self.assertEqual(b, self.staticFields.primitiveBoolean)
            self.assertIsInstance(self.staticFields.primitiveBoolean, bool)
            self.staticFields.objectBoolean = b
            self.assertEqual(b, self.staticFields.objectBoolean)
            self.assertIsInstance(self.staticFields.objectBoolean, bool)
            self.staticFields.object = b
            self.assertEqual(b, self.staticFields.object)
            self.assertIsInstance(self.staticFields.object, bool)
            self.fields.verify()
            self.staticFields.verify()

    # Python will coerce anything to a bool so anything is legal as a
    # primitive boolean
    def test_primitive_boolean_coercion(self):
        for b in (0, 1, 0.0, 0.1, "", "string", [], ["item"], {}, {"key": "value"}):
            self.assertEqual(bool(b), self.methods.primitiveBoolean(b))
            self.assertEqual(bool(b), self.staticMethods.primitiveBoolean(b))
            self.fields.primitiveBoolean = b
            self.assertEqual(bool(b), self.fields.primitiveBoolean)
            self.staticFields.primitiveBoolean = b
            self.assertEqual(bool(b), self.staticFields.primitiveBoolean)
            self.fields.verify()
            self.staticFields.verify()

    def test_object_boolean_coercion(self):
        for b in (0, 1, 0.0, 0.1, "", "string", [], ["item"], {}, {"key": "value"}):
            with self.assertRaises(TypeError):
                self.methods.objectBoolean(b)
            with self.assertRaises(TypeError):
                self.staticMethods.objectBoolean(b)
            with self.assertRaises(TypeError):
                self.fields.objectBoolean = b
            with self.assertRaises(TypeError):
                self.staticFields.objectBoolean = b

    def test_byte(self):
        for b in (-128, -1, 0, 1, 127):
            self.assertEqual(b, self.methods.primitiveByte(b))
            self.assertEqual(b, self.methods.objectByte(b))
            self.assertEqual(b, self.staticMethods.primitiveByte(b))
            self.assertEqual(b, self.staticMethods.objectByte(b))
            self.fields.primitiveByte = b
            self.assertEqual(b, self.fields.primitiveByte)
            self.fields.objectByte = b
            self.assertEqual(b, self.fields.objectByte)
            self.staticFields.primitiveByte = b
            self.assertEqual(b, self.staticFields.primitiveByte)
            self.staticFields.objectByte = b
            self.assertEqual(b, self.staticFields.objectByte)
            self.fields.verify()
            self.staticFields.verify()

    def test_byte_overflow(self):
        for b in (-9223372036854775809, -9223372036854775808, -2147483648, -32768, -129, 128, 32767, 2147483647, 9223372036854775807, 9223372036854775808):
            with self.assertRaises(TypeError):
                self.methods.primitiveByte(b)
            with self.assertRaises(TypeError):
                self.methods.objectByte(b)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveByte(b)
            with self.assertRaises(TypeError):
                self.staticMethods.objectByte(b)
            with self.assertRaises(OverflowError):
                self.fields.primitiveByte = b
            with self.assertRaises(OverflowError):
                self.fields.objectByte = b
            with self.assertRaises(OverflowError):
                self.staticFields.primitiveByte = b
            with self.assertRaises(OverflowError):
                self.staticFields.objectByte = b

    def test_primitive_byte_boolean_coercion(self):
        for b in (True, False):
            self.assertEqual(b, self.methods.primitiveByte(b))
            self.assertEqual(b, self.staticMethods.primitiveByte(b))
            self.fields.primitiveByte = b
            self.assertEqual(b, self.fields.primitiveByte)
            self.staticFields.primitiveByte = b
            self.assertEqual(b, self.staticFields.primitiveByte)
            self.fields.verify()
            self.staticFields.verify()

    def test_object_byte_boolean_coercion(self):
        for b in (True, False):
            with self.assertRaises(TypeError):
                self.methods.objectByte(b)
            with self.assertRaises(TypeError):
                self.staticMethods.objectByte(b)
            with self.assertRaises(TypeError):
                self.fields.objectByte = b
            with self.assertRaises(TypeError):
                self.staticFields.objectByte = b

    def test_byte_coercion(self):
        for b in (0.1, "string", [], {}):
            with self.assertRaises(TypeError):
                self.methods.primitiveByte(b)
            with self.assertRaises(TypeError):
                self.methods.objectByte(b)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveByte(b)
            with self.assertRaises(TypeError):
                self.staticMethods.objectByte(b)
            with self.assertRaises(TypeError):
                self.fields.primitiveByte = b
            with self.assertRaises(TypeError):
                self.fields.objectByte = b
            with self.assertRaises(TypeError):
                self.staticFields.primitiveByte = b
            with self.assertRaises(TypeError):
                self.staticFields.objectByte = b

    def test_short(self):
        for s in (-32768, -128, -1, 0, 1, 127, 32767):
            self.assertEqual(s, self.methods.primitiveShort(s))
            self.assertEqual(s, self.methods.objectShort(s))
            self.assertEqual(s, self.staticMethods.primitiveShort(s))
            self.assertEqual(s, self.staticMethods.objectShort(s))
            self.fields.primitiveShort = s
            self.assertEqual(s, self.fields.primitiveShort)
            self.fields.objectShort = s
            self.assertEqual(s, self.fields.objectShort)
            self.staticFields.primitiveShort = s
            self.assertEqual(s, self.staticFields.primitiveShort)
            self.staticFields.objectShort = s
            self.assertEqual(s, self.staticFields.objectShort)
            self.fields.verify()
            self.staticFields.verify()

    def test_short_overflow(self):
        for s in (-9223372036854775808, -9223372036854775809, -2147483648, -32769, 327678, 2147483647, 9223372036854775807, 9223372036854775808):
            with self.assertRaises(TypeError):
                self.methods.primitiveShort(s)
            with self.assertRaises(TypeError):
                self.methods.objectShort(s)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveShort(s)
            with self.assertRaises(TypeError):
                self.staticMethods.objectShort(s)
            with self.assertRaises(OverflowError):
                self.fields.primitiveShort = s
            with self.assertRaises(OverflowError):
                self.fields.objectShort = s
            with self.assertRaises(OverflowError):
                self.staticFields.primitiveShort = s
            with self.assertRaises(OverflowError):
                self.staticFields.objectShort = s

    def test_primitive_short_boolean_coercion(self):
        for s in (True, False):
            self.assertEqual(s, self.methods.primitiveShort(s))
            self.assertEqual(s, self.staticMethods.primitiveShort(s))
            self.fields.primitiveShort = s
            self.assertEqual(s, self.fields.primitiveShort)
            self.staticFields.primitiveShort = s
            self.assertEqual(s, self.staticFields.primitiveShort)
            self.fields.verify()
            self.staticFields.verify()

    def test_object_short_boolean_coercion(self):
        for s in (True, False):
            with self.assertRaises(TypeError):
                self.methods.objectShort(s)
            with self.assertRaises(TypeError):
                self.staticMethods.objectShort(s)
            with self.assertRaises(TypeError):
                self.fields.objectShort = s
            with self.assertRaises(TypeError):
                self.staticFields.objectShort = s

    def test_short_coercion(self):
        for s in (0.1, "string", [], {}):
            with self.assertRaises(TypeError):
                self.methods.primitiveShort(s)
            with self.assertRaises(TypeError):
                self.methods.objectShort(s)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveShort(s)
            with self.assertRaises(TypeError):
                self.staticMethods.objectShort(s)
            with self.assertRaises(TypeError):
                self.fields.primitiveShort = s
            with self.assertRaises(TypeError):
                self.fields.objectShort = s
            with self.assertRaises(TypeError):
                self.staticFields.primitiveShort = s
            with self.assertRaises(TypeError):
                self.staticFields.objectShort = s

    def convert_unicode_strings(self, in_strings):
        '''
           Generates test input and expected outcome for char and string tests.
           This method takes a list of strings that should be tested and
           generates a list of 2-tuples, each containing an input and an
           expected output. Starting in Python 3 all strings are unicode so the
           input and output should always be the same but in Python 2 it is more
           complex. Python 2 has support for a unicode type, but it is more
           common to use a string that contains UTF-8 encoded data. Java strings
           and chars are converted to UTF-8 encoded strings in python. This
           method expects the input list to contain a mix of unicode and string
           objects. For each unicode input value there will be 2 test cases
           generated, the first ensures that when a unicode object is passed
           into java and then back to python that it comes back as a UTF-8
           encoded string. The second test ensures that when a UTF-8 encoded
           string is passed to java and back that it is the same.

        '''
        return [(string, string) for string in in_strings]
            
     

    def test_char(self):
        chars = (' ', '\n', '\t', '1', 'A', 'a', '~', u'\u263A', '\0', '\xe9')
        char_pairs = self.convert_unicode_strings(chars)
        for c, e in char_pairs:
            self.assertEqual(e, self.methods.primitiveChar(c))
            self.assertEqual(e, self.methods.objectCharacter(c))
            self.assertEqual(e, self.staticMethods.primitiveChar(c))
            self.assertEqual(e, self.staticMethods.objectCharacter(c))
            self.fields.primitiveChar = c
            self.assertEqual(e, self.fields.primitiveChar)
            self.fields.objectCharacter = c
            self.assertEqual(e, self.fields.objectCharacter)
            self.staticFields.primitiveChar = c
            self.assertEqual(e, self.staticFields.primitiveChar)
            self.staticFields.objectCharacter = c
            self.assertEqual(e, self.staticFields.objectCharacter)
            self.fields.verify()
            self.staticFields.verify()

    def test_char_coercion(self):
        for c in (1, 0.1, "string", [], {}, u'\U0001F604'):
            with self.assertRaises(TypeError):
                self.methods.primitiveChar(c)
            with self.assertRaises(TypeError):
                self.methods.objectCharacter(c)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveChar(c)
            with self.assertRaises(TypeError):
                self.staticMethods.objectCharacter(c)
            with self.assertRaises(TypeError):
                self.fields.primitiveChar = c
            with self.assertRaises(TypeError):
                self.fields.objectCharacter = c
            with self.assertRaises(TypeError):
                self.staticFields.primitiveChar = c
            with self.assertRaises(TypeError):
                self.staticFields.objectCharacter = c

    def test_int(self):
        for i in (-2147483648, -32768, -128, -1, 0, 1, 127, 32767, 2147483647):
            self.assertEqual(i, self.methods.primitiveInt(i))
            self.assertEqual(i, self.methods.objectInteger(i))
            self.assertEqual(i, self.methods.object(i))
            self.assertEqual(i, self.staticMethods.primitiveInt(i))
            self.assertEqual(i, self.staticMethods.objectInteger(i))
            self.assertEqual(i, self.staticMethods.object(i))
            self.fields.primitiveInt = i
            self.assertEqual(i, self.fields.primitiveInt)
            self.fields.objectInteger = i
            self.assertEqual(i, self.fields.objectInteger)
            self.fields.object = i
            self.assertEqual(i, self.fields.object)
            self.staticFields.primitiveInt = i
            self.assertEqual(i, self.staticFields.primitiveInt)
            self.staticFields.objectInteger = i
            self.assertEqual(i, self.staticFields.objectInteger)
            self.staticFields.object = i
            self.assertEqual(i, self.staticFields.object)
            self.fields.verify()
            self.staticFields.verify()

    def test_int_overflow(self):
        for i in (-9223372036854775809, -9223372036854775808, -2147483649, 2147483648, 9223372036854775807, 9223372036854775808):
            with self.assertRaises(TypeError):
                self.methods.primitiveInt(i)
            with self.assertRaises(TypeError):
                self.methods.objectInteger(i)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveInt(i)
            with self.assertRaises(TypeError):
                self.staticMethods.objectInteger(i)
            with self.assertRaises(OverflowError):
                self.fields.primitiveInt = i
            with self.assertRaises(OverflowError):
                self.fields.objectInteger = i
            with self.assertRaises(OverflowError):
                self.staticFields.primitiveInt = i
            with self.assertRaises(OverflowError):
                self.staticFields.objectInteger = i

    def test_primitive_int_boolean_coersion(self):
        for i in (True, False):
            self.assertEqual(i, self.methods.primitiveInt(i))
            self.assertEqual(i, self.staticMethods.primitiveInt(i))
            self.fields.primitiveInt = i
            self.assertEqual(i, self.fields.primitiveInt)
            self.staticFields.primitiveInt = i
            self.assertEqual(i, self.staticFields.primitiveInt)
            self.fields.verify()
            self.staticFields.verify()

    def test_object_int_boolean_coersion(self):
        for i in (True, False):
            with self.assertRaises(TypeError):
                self.methods.objectInteger(i)
            with self.assertRaises(TypeError):
                self.staticMethods.objectInteger(i)
            with self.assertRaises(TypeError):
                self.fields.objectInteger = i
            with self.assertRaises(TypeError):
                self.staticFields.objectInteger = i

    def test_int_coersion(self):
        for i in (0.1, "string", [], {}):
            with self.assertRaises(TypeError):
                self.methods.primitiveInt(i)
            with self.assertRaises(TypeError):
                self.methods.objectInteger(i)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveInt(i)
            with self.assertRaises(TypeError):
                self.staticMethods.objectInteger(i)
            with self.assertRaises(TypeError):
                self.fields.primitiveInt = i
            with self.assertRaises(TypeError):
                self.fields.objectInteger = i
            with self.assertRaises(TypeError):
                self.staticFields.primitiveInt = i
            with self.assertRaises(TypeError):
                self.staticFields.objectInteger = i

    def test_float(self):
        for f in (float('-inf'), -1.5, 0.0, 1.5, float('inf')):
            self.assertEqual(f, self.methods.primitiveFloat(f))
            self.assertEqual(f, self.methods.objectFloat(f))
            self.assertEqual(f, self.staticMethods.primitiveFloat(f))
            self.assertEqual(f, self.staticMethods.objectFloat(f))
            self.fields.primitiveFloat = f
            self.assertEqual(f, self.fields.primitiveFloat)
            self.fields.objectFloat = f
            self.assertEqual(f, self.fields.objectFloat)
            self.staticFields.primitiveFloat = f
            self.assertEqual(f, self.staticFields.primitiveFloat)
            self.staticFields.objectFloat = f
            self.assertEqual(f, self.staticFields.objectFloat)
            self.fields.verify()
            self.staticFields.verify()

    def test_float_nan(self):
        self.assertTrue(isnan(self.methods.primitiveFloat(float('nan'))))
        self.assertTrue(isnan(self.methods.objectFloat(float('nan'))))
        self.assertTrue(isnan(self.staticMethods.primitiveFloat(float('nan'))))
        self.assertTrue(isnan(self.staticMethods.objectFloat(float('nan'))))
        self.fields.primitiveFloat = float('nan')
        self.assertTrue(isnan(self.fields.primitiveFloat))
        self.fields.objectFloat = float('nan')
        self.assertTrue(isnan(self.fields.objectFloat))
        self.staticFields.primitiveFloat = float('nan')
        self.assertTrue(isnan(self.staticFields.primitiveFloat))
        self.staticFields.objectFloat = float('nan')
        self.assertTrue(isnan(self.staticFields.objectFloat))
        self.fields.verify()
        self.staticFields.verify()

    def test_primitive_float_int_coercion(self):
        for f in (-16777216, -32768, -128, -1, 0, 1, 127, 32767, 16777216, True, False):
            self.assertEqual(f, self.methods.primitiveFloat(f))
            self.assertEqual(f, self.staticMethods.primitiveFloat(f))
            self.fields.primitiveFloat = f
            self.assertEqual(f, self.fields.primitiveFloat)
            self.staticFields.primitiveFloat = f
            self.assertEqual(f, self.staticFields.primitiveFloat)
            self.fields.verify()
            self.staticFields.verify()

    def test_object_float_int_coercion(self):
        for f in (-16777216, -32768, -128, -1, 0, 1, 127, 32767, 16777216, True, False):
            with self.assertRaises(TypeError):
                self.methods.objectFloat(f)
            with self.assertRaises(TypeError):
                self.staticMethods.objectFloat(f)
            with self.assertRaises(TypeError):
                self.fields.objectFloat = f
            with self.assertRaises(TypeError):
                self.staticFields.objectFloat = f

    def test_float_coercion(self):
        for f in ("string", [], {}):
            with self.assertRaises(TypeError):
                self.methods.primitiveFloat(f)
            with self.assertRaises(TypeError):
                self.methods.objectFloat(f)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveFloat(f)
            with self.assertRaises(TypeError):
                self.staticMethods.objectFloat(f)
            with self.assertRaises(TypeError):
                self.fields.primitiveFloat = f
            with self.assertRaises(TypeError):
                self.fields.objectFloat = f
            with self.assertRaises(TypeError):
                self.staticFields.primitiveFloat = f
            with self.assertRaises(TypeError):
                self.staticFields.objectFloat = f

    def test_long(self):
        longs = [-9223372036854775808, -2147483648, -32768, -128, -
                 1, 0, 1, 127, 32767, 2147483647, 9223372036854775807]
        for l in longs:
            self.assertEqual(l, self.methods.primitiveLong(l))
            self.assertEqual(l, self.methods.objectLong(l))
            self.assertEqual(l, self.methods.object(l))
            self.assertEqual(l, self.staticMethods.primitiveLong(l))
            self.assertEqual(l, self.staticMethods.objectLong(l))
            self.assertEqual(l, self.staticMethods.object(l))
            self.fields.primitiveLong = l
            self.assertEqual(l, self.fields.primitiveLong)
            self.fields.objectLong = l
            self.assertEqual(l, self.fields.objectLong)
            self.fields.object = l
            self.assertEqual(l, self.fields.object)
            self.staticFields.primitiveLong = l
            self.assertEqual(l, self.staticFields.primitiveLong)
            self.staticFields.objectLong = l
            self.assertEqual(l, self.staticFields.objectLong)
            self.staticFields.object = l
            self.assertEqual(l, self.staticFields.object)
            self.fields.verify()
            self.staticFields.verify()

    def test_long_overflow(self):
        for l in (-9223372036854775809, 9223372036854775808):
            with self.assertRaises(TypeError):
                self.methods.primitiveLong(l)
            with self.assertRaises(TypeError):
                self.methods.objectLong(l)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveLong(l)
            with self.assertRaises(TypeError):
                self.staticMethods.objectLong(l)
            with self.assertRaises(OverflowError):
                self.fields.primitiveLong = l
            with self.assertRaises(OverflowError):
                self.fields.objectLong = l
            with self.assertRaises(OverflowError):
                self.staticFields.primitiveLong = l
            with self.assertRaises(OverflowError):
                self.staticFields.objectLong = l

    def test_primitive_long_boolean_coersion(self):
        for l in (True, False):
            self.assertEqual(l, self.methods.primitiveLong(l))
            self.assertEqual(l, self.staticMethods.primitiveLong(l))
            self.fields.primitiveLong = l
            self.assertEqual(l, self.fields.primitiveLong)
            self.staticFields.primitiveLong = l
            self.assertEqual(l, self.staticFields.primitiveLong)
            self.fields.verify()
            self.staticFields.verify()

    def test_object_long_boolean_coersion(self):
        for l in (True, False):
            with self.assertRaises(TypeError):
                self.methods.objectLong(l)
            with self.assertRaises(TypeError):
                self.staticMethods.objectLong(l)
            with self.assertRaises(TypeError):
                self.fields.objectLong = l
            with self.assertRaises(TypeError):
                self.staticFields.objectLong = l

    def test_long_coersion(self):
        for l in (0.1, "string", [], {}):
            with self.assertRaises(TypeError):
                self.methods.primitiveLong(l)
            with self.assertRaises(TypeError):
                self.methods.objectLong(l)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveLong(l)
            with self.assertRaises(TypeError):
                self.staticMethods.objectLong(l)
            with self.assertRaises(TypeError):
                self.fields.primitiveLong = l
            with self.assertRaises(TypeError):
                self.fields.objectLong = l
            with self.assertRaises(TypeError):
                self.staticFields.primitiveLong = l
            with self.assertRaises(TypeError):
                self.staticFields.objectLong = l

    def test_double(self):
        for d in (float('-inf'), -1.5, 0.0, 1.5, float('inf')):
            self.assertEqual(d, self.methods.primitiveDouble(d))
            self.assertEqual(d, self.methods.objectDouble(d))
            self.assertEqual(d, self.methods.object(d))
            self.assertEqual(d, self.staticMethods.primitiveDouble(d))
            self.assertEqual(d, self.staticMethods.objectDouble(d))
            self.assertEqual(d, self.staticMethods.object(d))
            self.fields.primitiveDouble = d
            self.assertEqual(d, self.fields.primitiveDouble)
            self.fields.objectDouble = d
            self.assertEqual(d, self.fields.objectDouble)
            self.fields.object = d
            self.assertEqual(d, self.fields.object)
            self.staticFields.primitiveDouble = d
            self.assertEqual(d, self.staticFields.primitiveDouble)
            self.staticFields.objectDouble = d
            self.assertEqual(d, self.staticFields.objectDouble)
            self.staticFields.object = d
            self.assertEqual(d, self.staticFields.object)
            self.fields.verify()
            self.staticFields.verify()

    def test_double_nan(self):
        self.assertTrue(isnan(self.methods.primitiveDouble(float('nan'))))
        self.assertTrue(isnan(self.methods.objectDouble(float('nan'))))
        self.assertTrue(isnan(self.methods.object(float('nan'))))
        self.assertTrue(isnan(self.staticMethods.primitiveDouble(float('nan'))))
        self.assertTrue(isnan(self.staticMethods.objectDouble(float('nan'))))
        self.assertTrue(isnan(self.staticMethods.object(float('nan'))))
        self.fields.primitiveDouble = float('nan')
        self.assertTrue(isnan(self.fields.primitiveDouble))
        self.fields.objectDouble = float('nan')
        self.assertTrue(isnan(self.fields.objectDouble))
        self.fields.object = float('nan')
        self.assertTrue(isnan(self.fields.object))
        self.staticFields.primitiveDouble = float('nan')
        self.assertTrue(isnan(self.staticFields.primitiveDouble))
        self.staticFields.objectDouble = float('nan')
        self.assertTrue(isnan(self.staticFields.objectDouble))
        self.staticFields.object = float('nan')
        self.assertTrue(isnan(self.staticFields.object))
        self.fields.verify()
        self.staticFields.verify()

    def test_primitive_double_int_coercion(self):
        for d in (-9007199254740992, -2147483648, -16777216, -32768, -128, -1, 0, 1, 127, 32767, 16777216, 2147483647, 9007199254740992, True, False):
            self.assertEqual(d, self.methods.primitiveDouble(d))
            self.assertEqual(d, self.staticMethods.primitiveDouble(d))
            self.fields.primitiveDouble = d
            self.assertEqual(d, self.fields.primitiveDouble)
            self.staticFields.primitiveDouble = d
            self.assertEqual(d, self.staticFields.primitiveDouble)
            self.fields.verify()
            self.staticFields.verify()

    def test_object_double_int_coercion(self):
        for d in (-9007199254740992, -2147483648, -16777216, -32768, -128, -1, 0, 1, 127, 32767, 16777216, 2147483647, 9007199254740992, True, False):
            with self.assertRaises(TypeError):
                self.methods.objectDouble(d)
            with self.assertRaises(TypeError):
                self.staticMethods.objectDouble(d)
            with self.assertRaises(TypeError):
                self.fields.objectDouble = d
            with self.assertRaises(TypeError):
                self.staticFields.objectDouble = d

    def test_double_coercion(self):
        for d in ("string", [], {}):
            with self.assertRaises(TypeError):
                self.methods.primitiveDouble(d)
            with self.assertRaises(TypeError):
                self.methods.objectDouble(d)
            with self.assertRaises(TypeError):
                self.staticMethods.primitiveDouble(d)
            with self.assertRaises(TypeError):
                self.staticMethods.objectDouble(d)
            with self.assertRaises(TypeError):
                self.fields.primitiveDouble = d
            with self.assertRaises(TypeError):
                self.fields.objectDouble = d
            with self.assertRaises(TypeError):
                self.staticFields.primitiveDouble = d
            with self.assertRaises(TypeError):
                self.staticFields.objectDouble = d

    def test_string(self):
        strings = ('', 's', 'string', u'test\u00e9', u'\u263A', u'\U0001F604', 'null\0char', None)
        string_pairs = self.convert_unicode_strings(strings)
        for s, e in string_pairs:
            self.assertEqual(e, self.methods.objectString(s))
            self.assertEqual(e, self.methods.object(s))
            self.assertEqual(e, self.staticMethods.objectString(s))
            self.assertEqual(e, self.staticMethods.object(s))
            self.fields.objectString = s
            self.assertEqual(e, self.fields.objectString)
            self.fields.object = s
            self.assertEqual(e, self.fields.object)
            self.staticFields.objectString = s
            self.assertEqual(e, self.staticFields.objectString)
            self.staticFields.object = s
            self.assertEqual(e, self.staticFields.object)
            self.fields.verify()
            self.staticFields.verify()

    def test_string_coercion(self):
        for s in (False, True, 0, 1, 0.1, [], {}):
            with self.assertRaises(TypeError):
                self.methods.objectString(s)
            with self.assertRaises(TypeError):
                self.staticMethods.objectString(s)
            with self.assertRaises(TypeError):
                self.fields.objectString = s
            with self.assertRaises(TypeError):
                self.staticFields.objectString = s

    def test_class(self):
        for c in (Object, Integer, Class):
            self.assertEqual(c, self.methods.objectClass(c))
            self.assertEqual(c, self.methods.object(c))
            self.assertEqual(c, self.staticMethods.objectClass(c))
            self.assertEqual(c, self.staticMethods.object(c))
            self.fields.objectClass = c
            self.assertEqual(c, self.fields.objectClass)
            self.fields.object = c
            self.assertEqual(c, self.fields.object)
            self.staticFields.objectClass = c
            self.assertEqual(c, self.staticFields.objectClass)
            self.staticFields.object = c
            self.assertEqual(c, self.staticFields.object)
            self.fields.verify()
            self.staticFields.verify()

    def test_class_coercion(self):
        for c in (False, True, 0, 1, 0.1, [], {}):
            with self.assertRaises(TypeError):
                self.methods.objectClass(c)
            with self.assertRaises(TypeError):
                self.staticMethods.objectClass(c)
            with self.assertRaises(TypeError):
                self.fields.objectClass = c
            with self.assertRaises(TypeError):
                self.staticFields.objectClass = c

    def test_list(self):
        for l in ([], (), [1, 2, 3], (1, 2, 3)):
            self.assertSequenceEqual(l, self.methods.list(l))
            self.assertSequenceEqual(l, self.methods.object(l))
            self.assertSequenceEqual(l, self.staticMethods.list(l))
            self.assertSequenceEqual(l, self.staticMethods.object(l))
            self.fields.list = l
            self.assertSequenceEqual(l, self.fields.list)
            self.fields.object = l
            self.assertSequenceEqual(l, self.fields.object)
            self.staticFields.list = l
            self.assertSequenceEqual(l, self.staticFields.list)
            self.staticFields.object = l
            self.assertSequenceEqual(l, self.staticFields.object)
            self.fields.verify()
            self.staticFields.verify()

    def test_list_coercion(self):
        for l in (False, True, 0, 1, 0.1, {}, 'string'):
            with self.assertRaises(TypeError):
                self.methods.list(l)
            with self.assertRaises(TypeError):
                self.staticMethods.list(l)
            with self.assertRaises(TypeError):
                self.fields.list = l
            with self.assertRaises(TypeError):
                self.staticFields.list = l

    def test_arrayList(self):
        for l in ([], [1, 2, 3]):
            self.assertSequenceEqual(l, self.methods.arrayList(l))
            self.assertSequenceEqual(l, self.staticMethods.arrayList(l))
            self.fields.arrayList = l
            self.assertSequenceEqual(l, self.fields.arrayList)
            self.staticFields.arrayList = l
            self.assertSequenceEqual(l, self.staticFields.arrayList)
            self.fields.verify()
            self.staticFields.verify()

    def test_arrayList_coercion(self):
        for l in ((), (1,2,3), False, True, 0, 1, 0.1, {}, 'string'):
            with self.assertRaises(TypeError):
                self.methods.arrayList(l)
            with self.assertRaises(TypeError):
                self.staticMethods.arrayList(l)
            with self.assertRaises(TypeError):
                self.fields.arrayList = l
            with self.assertRaises(TypeError):
                self.staticFields.arrayList = l

    # disabled because a dict cannot be constructed from a pyjmap
    def todo_test_map(self):
        for m in ({}, {"a":"b", "c":"d"}):
            self.assertEqual(m, dict(self.methods.map(m)))
            self.assertEqual(m, dict(self.methods.object(m)))
            self.assertEqual(m, dict(self.staticMethods.map(m)))
            self.assertEqual(m, dict(self.staticMethods.object(m)))
            self.fields.map = m
            self.assertEqual(m, dict(self.fields.map))
            self.fields.object = m
            self.assertEqual(m, dict(self.fields.object))
            self.staticFields.map = m
            self.assertEqual(m, dict(self.staticFields.map))
            self.staticFields.object = m
            self.assertEqual(m, dict(self.staticFields.object))
            self.fields.verify()
            self.staticFields.verify()

    def test_map_coercion(self):
        for m in ((), [], False, True, 0, 1, 0.1, 'string'):
            with self.assertRaises(TypeError):
                self.methods.map(m)
            with self.assertRaises(TypeError):
                self.staticMethods.map(m)
            with self.assertRaises(TypeError):
                self.fields.map = m
            with self.assertRaises(TypeError):
                self.staticFields.map = m

    def test_string_array(self):
        for l in ([], (), ["1", "2", "3"], ("1", "2", "3")):
            self.assertSequenceEqual(l, self.methods.stringArray(l))
            self.assertSequenceEqual(l, self.staticMethods.stringArray(l))
            self.fields.stringArray = l
            self.assertSequenceEqual(l, self.fields.stringArray)
            self.staticFields.stringArray = l
            self.assertSequenceEqual(l, self.staticFields.stringArray)
            self.fields.verify()
            self.staticFields.verify()

    def test_string_array_coercion(self):
        for l in ([1, 2, 3], (1, 2, 3)):
            with self.assertRaises(TypeError):
                self.methods.stringArray(l)
            with self.assertRaises(TypeError):
                self.staticMethods.stringArray(l)
            with self.assertRaises(TypeError):
                self.fields.stringArray = l
            with self.assertRaises(TypeError):
                self.staticFields.stringArray = l

    def test_int_array(self):
        examples = [[], (), [1, 2, 3], (1, 2, 3)]
        import array
        a = array.array('i', [1,2,3,4])
        examples.append(a)
        v = memoryview(a)
        v = v[::2]
        examples.append(v)
        for l in examples:
            self.assertSequenceEqual(l, self.methods.intArray(l))
            self.assertSequenceEqual(l, self.staticMethods.intArray(l))
            self.fields.intArray = l
            self.assertSequenceEqual(l, self.fields.intArray)
            self.staticFields.intArray = l
            self.assertSequenceEqual(l, self.staticFields.intArray)
            self.assertSequenceEqual(self.methods.object(a), a)
            self.fields.verify()
            self.staticFields.verify()

    def test_int_array_coercion(self):
        examples = [["1", "2", "3"], ("1", "2", "3")]
        import array
        examples.append(array.array('f', [1,2,3,4]))
        for l in examples:
            with self.assertRaises(TypeError):
                self.methods.intArray(l)
            with self.assertRaises(TypeError):
                self.staticMethods.intArray(l)
            with self.assertRaises(TypeError):
                self.fields.intArray = l
            with self.assertRaises(TypeError):
                self.staticFields.intArray = l

    def test_byte_array(self):
        from java.nio import ByteBuffer
        import array
        l = [1,2,3,4]
        bb = ByteBuffer.wrap(bytearray(l))
        self.assertSequenceEqual(bb.array(), l)
        bb = ByteBuffer.wrap(bytes(l))
        self.assertSequenceEqual(bb.array(), l)
        bb = ByteBuffer.wrap(array.array('b', l))
        self.assertSequenceEqual(bb.array(), l)
        bb = ByteBuffer.wrap(array.array('B', l))
        self.assertSequenceEqual(bb.array(), l)
        self.assertSequenceEqual(self.methods.object(l), l)
        v = memoryview(array.array('B', l))
        v = v[::2]
        bb = ByteBuffer.wrap(v)
        self.assertSequenceEqual(bb.array(), v)
        with self.assertRaises(TypeError):
            ByteBuffer.wrap(array.array('f', [1,2,3,4]))
                
    def test_float_array(self):
        from java.nio import FloatBuffer
        import array
        a = array.array('f', [1,2,3,4])
        fb = FloatBuffer.wrap(a)
        self.assertSequenceEqual(fb.array(), a)
        self.assertSequenceEqual(self.methods.object(a), a)
        v = memoryview(a)
        v = v[::2]
        fb = FloatBuffer.wrap(v)
        self.assertSequenceEqual(fb.array(), v)
        with self.assertRaises(TypeError):
            FloatBuffer.wrap(array.array('i', [1,2,3,4]))

    def test_long_array(self):
        from java.nio import LongBuffer
        import array
        a = array.array('q', [1,2,3,4])
        lb = LongBuffer.wrap(a)
        self.assertSequenceEqual(lb.array(), a)
        self.assertSequenceEqual(self.methods.object(a), a)
        v = memoryview(a)
        v = v[::2]
        lb = LongBuffer.wrap(v)
        self.assertSequenceEqual(lb.array(), v)
        with self.assertRaises(TypeError):
            LongBuffer.wrap(array.array('f', [1,2,3,4]))

    def test_buffers(self):
        from java.nio import ByteBuffer, ByteOrder
        b = ByteBuffer.allocateDirect(16).order(ByteOrder.nativeOrder())
        v = memoryview(b)
        v[0] = 7
        self.assertEqual(v[0], b.get(0))
        s = b.asShortBuffer();
        v = memoryview(s)
        self.assertEqual(v[0], s.get(0))
        i = b.asIntBuffer()
        v = memoryview(i)
        self.assertEqual(v[0], i.get(0))
        l = b.asLongBuffer()
        v = memoryview(l)
        self.assertEqual(v[0], l.get(0))
        f = b.asFloatBuffer()
        v = memoryview(f)
        v[0] = -100
        self.assertEqual(v[0], f.get(0))
        d = b.asDoubleBuffer()
        v = memoryview(d)
        v[0] = -100
        self.assertEqual(v[0], d.get(0))
        try:
           # memoryview only supports native order so numpy is required for the other one.
            from numpy import asarray
            for order in (ByteOrder.LITTLE_ENDIAN, ByteOrder.BIG_ENDIAN):
                b = ByteBuffer.allocateDirect(16).order(order)
                v = asarray(b)
                v[0] = 7
                self.assertEqual(v[0], b.get(0))
                s = b.asShortBuffer();
                v = asarray(s)
                self.assertEqual(v[0], s.get(0))
                i = b.asIntBuffer()
                v = asarray(i)
                self.assertEqual(v[0], i.get(0))
                l = b.asLongBuffer()
                v = asarray(l)
                self.assertEqual(v[0], l.get(0))
                f = b.asFloatBuffer()
                v = asarray(f)
                v[0] = -100
                self.assertEqual(v[0], f.get(0))
                d = b.asDoubleBuffer()
                v = asarray(d)
                v[0] = -100
                self.assertEqual(v[0], d.get(0))
        except ImportError:
            pass # not built with numpy

