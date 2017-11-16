import unittest

import jep

class TestCallable(object):
    def __init__(self, result):
        self.result = result
    def call(self):
        return self.result

class TestJProxy(unittest.TestCase):

    def test_default_methods(self):
        proxy = jep.jproxy(self, ["jep.test.TestDefaultMethods"])
        proxy.testVoidReturn();
        self.assertEqual(True, proxy.testBooleanArg(True));
        self.assertEqual(True, proxy.testBooleanReturn(True));
        self.assertEqual(1, proxy.testByteArg(1));
        self.assertEqual(1, proxy.testByteReturn(1));
        self.assertEqual('a', proxy.testCharArg('a'));
        self.assertEqual('a', proxy.testCharReturn('a'));
        self.assertEqual(1, proxy.testShortArg(1));
        self.assertEqual(1, proxy.testShortReturn(1));
        self.assertEqual(1, proxy.testIntArg(1));
        self.assertEqual(1, proxy.testIntReturn(1));
        self.assertEqual(1, proxy.testLongArg(1));
        self.assertEqual(1, proxy.testLongReturn(1));
        self.assertEqual(1.0, proxy.testFloatArg(1.0));
        self.assertEqual(1.0, proxy.testFloatReturn(1.0));
        self.assertEqual(1.0, proxy.testDoubleArg(1.0));
        self.assertEqual(1.0, proxy.testDoubleReturn(1.0));

    def test_runnable(self):
        proxy = jep.jproxy(TestCallable(True), ["java.util.concurrent.Callable"])
        self.assertEqual(True, proxy.call())
