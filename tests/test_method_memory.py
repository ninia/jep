# This test case attempts to verify that pyjmethods are correctly managing their refcounts.

import unittest
from .leak_tool import test_leak

class TestMethodMemory(unittest.TestCase):

    def setUp(self):
        from java.lang import Object
        self.obj = Object()

    def test_repeated_call(self):
        method = self.obj.hashCode
        test_leak(self, lambda:method(), "Called method")

    def test_access_call(self):
        test_leak(self, lambda:self.obj.hashCode(), "Access and call method")

    def test_access_no_call(self):
        test_leak(self, lambda:self.obj.hashCode, "Access method")
 
