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
            self.assertEqual(b, self.methods.objectBoolean(b))
            self.assertEqual(b, self.methods.object(b))
            self.assertEqual(b, self.staticMethods.primitiveBoolean(b))
            self.assertEqual(b, self.staticMethods.objectBoolean(b))
            self.assertEqual(b, self.staticMethods.object(b))
            self.fields.primitiveBoolean = b
            self.assertEqual(b, self.fields.primitiveBoolean)
            self.fields.objectBoolean = b
            self.assertEqual(b, self.fields.objectBoolean)
            self.fields.object = b
            self.assertEqual(b, self.fields.object)
            self.staticFields.primitiveBoolean = b
            self.assertEqual(b, self.staticFields.primitiveBoolean)
            self.staticFields.objectBoolean = b
            self.assertEqual(b, self.staticFields.objectBoolean)
            self.staticFields.object = b
            self.assertEqual(b, self.staticFields.object)
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

    def test_char(self):
        for c in (' ', '\n', '\t', '1', 'A', 'a', '~'):
            self.assertEqual(c, self.methods.primitiveChar(c))
            self.assertEqual(c, self.methods.objectCharacter(c))
            self.assertEqual(c, self.methods.object(c))
            self.assertEqual(c, self.staticMethods.primitiveChar(c))
            self.assertEqual(c, self.staticMethods.objectCharacter(c))
            self.assertEqual(c, self.staticMethods.object(c))
            self.fields.primitiveChar = c
            self.assertEqual(c, self.fields.primitiveChar)
            self.fields.objectCharacter = c
            self.assertEqual(c, self.fields.objectCharacter)
            self.fields.object = c
            self.assertEqual(c, self.fields.object)
            self.staticFields.primitiveChar = c
            self.assertEqual(c, self.staticFields.primitiveChar)
            self.staticFields.objectCharacter = c
            self.assertEqual(c, self.staticFields.objectCharacter)
            self.staticFields.object = c
            self.assertEqual(c, self.staticFields.object)
            self.fields.verify()
            self.staticFields.verify()

    def test_char_coercion(self):
        for c in (1, 0.1, "string", [], {}):
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
        if sys.version_info.major == 2:
            for l in tuple(longs):
                longs.append(long(l))
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
        for s in ('', 's', 'string'):
            self.assertEqual(s, self.methods.objectString(s))
            self.assertEqual(s, self.methods.object(s))
            self.assertEqual(s, self.staticMethods.objectString(s))
            self.assertEqual(s, self.staticMethods.object(s))
            self.fields.objectString = s
            self.assertEqual(s, self.fields.objectString)
            self.fields.object = s
            self.assertEqual(s, self.fields.object)
            self.staticFields.objectString = s
            self.assertEqual(s, self.staticFields.objectString)
            self.staticFields.object = s
            self.assertEqual(s, self.staticFields.object)
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
