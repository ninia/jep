import unittest
import sys
import jep
from java.util import ArrayList
TestOverload = jep.findClass('jep.test.TestOverload')


class TestTypes(unittest.TestCase):

    def test_any_primitive(self):
        if sys.version_info.major == 2:
            self.assertEqual(TestOverload.any_primitive(0), 'int')
            self.assertEqual(TestOverload.any_primitive(1), 'int')
        else:
            self.assertEqual(TestOverload.any_primitive(0), 'long')
            self.assertEqual(TestOverload.any_primitive(1), 'long')
        # float and double methods are both ranked the same so it is arbitrary
        # which one you get, this may be an area for improvement
        self.assertIn(TestOverload.any_primitive(0.0), ('float', 'double'))
        self.assertEqual(TestOverload.any_primitive(True), 'boolean')

    def test_boxing(self):
        self.assertEqual(TestOverload.int_or_Integer(0), 'int')
        self.assertEqual(TestOverload.int_or_Integer(1), 'int')
        self.assertEqual(TestOverload.int_or_Integer(None), 'Integer')
        self.assertEqual(TestOverload.int_or_Object(0), 'int')
        self.assertEqual(TestOverload.int_or_Object(1), 'int')
        self.assertEqual(TestOverload.int_or_Object(None), 'Object')
        self.assertEqual(TestOverload.int_or_Object('0'), 'Object')
        self.assertEqual(TestOverload.float_or_Float(0.0), 'float')
        self.assertEqual(TestOverload.float_or_Float(1.1), 'float')

    def test_string(self):
        # Not sure this one is actually what we want?
        self.assertEqual(TestOverload.char_or_String('a'), 'String')
        self.assertEqual(TestOverload.char_or_String('abc'), 'String')
        self.assertEqual(TestOverload.char_or_String(None), 'String')
        self.assertEqual(TestOverload.Object_or_String('a'), 'String')
        self.assertEqual(TestOverload.Object_or_String('abc'), 'String')
        self.assertEqual(TestOverload.Object_or_String(None), 'Object')
        self.assertEqual(TestOverload.Object_or_String(0), 'Object')
        self.assertEqual(TestOverload.Object_or_String(0.0), 'Object')

    def test_collections(self):
        self.assertEqual(TestOverload.Object_or_ArrayList(ArrayList()), 'ArrayList')
        self.assertEqual(TestOverload.ArrayList_or_List(ArrayList()), 'ArrayList')
        self.assertEqual(TestOverload.Object_or_Array(ArrayList().toArray()), 'Array')

    def test_class(self):
        self.assertEqual(TestOverload.Object_or_Class(ArrayList), 'Class')
        self.assertEqual(TestOverload.Object_or_Class(0), 'Object')
        self.assertEqual(TestOverload.Object_or_Class(0.0), 'Object')
        self.assertEqual(TestOverload.Object_or_Class('0'), 'Object')
        # This probably doesn't matter but it is currently deterministic
        self.assertEqual(TestOverload.Object_or_Class(None), 'Object')

    def things_that_might_need_fixing(self):
        # 64 bit python 2 stores this in an int but it is too big for a java int
        # The method matching doesn't actually check the size of the int so it
        # chooses the int method and gets a TypeError
        self.assertEqual(TestOverload.any_primitive(2147483648), 'long')
        # The conversion code will treat a tuple as a boolean but the method
        # matching code does not.
        self.assertEqual(TestOverload.any_primitive(()), 'boolean')
        # Matching does not allow ints to be floats.
        self.assertEqual(TestOverload.float_or_Float(0), 'float')
        self.assertEqual(TestOverload.float_or_Float(1), 'float')
        # All instances of Object are matched equally
        self.assertEqual(TestOverload.Object_or_Integer(0), 'Integer')
        # Method choosing doesn't know about unicode so picks an arbitrary one
        self.assertEqual(TestOverload.char_or_String(u'\u263A'), 'char')
        self.assertEqual(TestOverload.char_or_String(u'\U0001F604'), 'String')
        # The method matching doesn't handle lists so it picks an arbitrary method
        self.assertEqual(TestOverload.Object_or_List([]), 'List')
        self.assertEqual(TestOverload.Object_or_ArrayList([]), 'ArrayList')
        self.assertEqual(TestOverload.ArrayList_or_List([]), 'ArrayList')
        # The method matching doesn't handle tuples so it picks an arbitrary method
        self.assertEqual(TestOverload.Object_or_List(()), 'List')
        self.assertEqual(TestOverload.Object_or_ArrayList(()), 'Object')
        self.assertEqual(TestOverload.ArrayList_or_List(()), 'List')
        # interfaces are not prefered over Object so its arbitrary
        self.assertEqual(TestOverload.Object_or_List(ArrayList()), 'List')
        # The method matching doesn't handle dicts so it picks an arbitrart method
        self.assertEqual(TestOverload.Object_or_Map({}), 'Map')
        # The method choosing treats all Objects as equal and picks an arbitrary one
        self.assertEqual(TestOverload.Object_or_List(0), 'Object')
        self.assertEqual(TestOverload.Object_or_List(0.0), 'Object')
        self.assertEqual(TestOverload.Object_or_ArrayList(0), 'Object')
        self.assertEqual(TestOverload.Object_or_ArrayList(0.0), 'Object')
        self.assertEqual(TestOverload.Object_or_Map(0), 'Object')
        self.assertEqual(TestOverload.Object_or_Map(0.0), 'Object')
        self.assertEqual(TestOverload.Object_or_Array(0), 'Object')
        self.assertEqual(TestOverload.Object_or_Array(0.0), 'Object')

