# This test case tests that various operations on java objects do not increase
# the total number of python references, because that would cause memory leaks.
#
# This test case will only run if python was built --with-pydebug because
# otherwise there is no way to track refcounts.
#
# In each test the refcount is expected to increase by 1, this one reference is
# the variable refcount1.
#
# Be very careful changing any of the format, things that seem harmless can
# impact the refcount, for example changing the assert to something like this:
#     self.assertEquals(refcount1, sys.gettotalrefcount() - 1)
# will not work because python will create new references for self.assertEquals,
# the implicit self argument, and refcount1.

import unittest
import sys


@unittest.skipIf(not hasattr(sys, "gettotalrefcount"),
                 "Python must be compiled --with-pydebug.")
class TestPythonRefCounts(unittest.TestCase):

    def setUp(self):
        from java.lang import Object
        self.obj = Object()

    def test_method_binding(self):
        refcount1 = sys.gettotalrefcount()
        result = self.obj.hashCode
        del result
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_method_call(self):
        # First call to hashCode will cache info about the hashCode method
        self.obj.hashCode()
        refcount1 = sys.gettotalrefcount()
        result = self.obj.hashCode()
        del result
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_package_import(self):
        refcount1 = sys.gettotalrefcount()
        import java.lang as result
        del result
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_class_import(self):
        refcount1 = sys.gettotalrefcount()
        from java.lang import Object as result
        del result
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_construction(self):
        from java.lang import Object
        refcount1 = sys.gettotalrefcount()
        result = Object()
        del result
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_field_access(self):
        from java.lang import System
        # First time will cache info about the PrintStream class
        result = System.out
        del result
        refcount1 = sys.gettotalrefcount()
        result = System.out
        del result
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_array_creation(self):
        from jep import jarray
        from java.lang import Object
        refcount1 = sys.gettotalrefcount()
        result = jarray(1, Object)
        del result
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_array_assignment(self):
        from jep import jarray
        from java.lang import Object
        arr = jarray(1, Object)
        refcount1 = sys.gettotalrefcount()
        arr[0] = self.obj
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_array_access(self):
        from jep import jarray
        from java.lang import Object
        arr = jarray(1, Object)
        arr[0] = self.obj
        refcount1 = sys.gettotalrefcount()
        result = arr[0]
        del result
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_number_compare(self):
        x = 5
        from java.lang import Integer
        y = Integer(9)
        refcount1 = sys.gettotalrefcount()
        result = x < y
        del result
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_list_setslice(self):
        from java.util import ArrayList
        jlist = ArrayList()
        for i in range(5):
            jlist.add(i)
        refcount1 = sys.gettotalrefcount()
        jlist[2:4] = [7, 19]
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)

    def test_dir_class(self):
        from java.util import ArrayList
        refcount1 = sys.gettotalrefcount()
        result = dir(ArrayList)
        del result
        refcount2 = sys.gettotalrefcount()
        self.assertEquals(refcount1, refcount2 - 1)
