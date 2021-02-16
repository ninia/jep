from unittest import TestCase

from java.util import List


class FakeTestCase(TestCase):
    "This class exists to get the assert methods without having a real TestCase"
    
    def runTest(self):
        pass

def invokeNoArgs():
    x = FakeTestCase()
    x.assertTrue(True)

def invokeArgs(arg1, arg2, arg3):
    x = FakeTestCase()
    x.assertEqual(arg1, "a")
    x.assertIsNone(arg2, None)
    x.assertEqual(arg3, 5.4)
    return True

def invokeVarArgsExplicit(arg1, arg2, arg3, arg4):
    x = FakeTestCase()
    x.assertEqual(arg1, True)
    x.assertIsNone(arg2)
    x.assertEqual(arg3, 2)
    x.assertEqual(arg4, "xyz")
    return None

def invokeVarArgs(*args):
    x = FakeTestCase()
    x.assertEqual(len(args), 4)
    x.assertEqual(args[0], True)
    x.assertIsNone(args[1])
    x.assertEqual(args[2], 2)
    x.assertEqual(args[3], "xyz")
    return args[3]

def invokeKeywordArgsExplicit(argnull, arg4, arg5, arg6=None):
    x = FakeTestCase()
    x.assertIsNone(argnull)
    x.assertEqual(arg4, "xyz")
    x.assertTrue(isinstance(arg5,List.__pytype__))
    x.assertIsNone(arg6)
    return arg5

def invokeKeywordArgs(**kwargs):
    x = FakeTestCase()
    x.assertEqual(len(kwargs), 3)
    x.assertEqual(kwargs['arg4'], "xyz")
    x.assertIsNone(kwargs['argnull'])
    arg5 = kwargs['arg5']
    x.assertTrue(isinstance(arg5,List.__pytype__))
    x.assertNotIn("arg1", kwargs)
    x.assertNotIn("arg2", kwargs)
    x.assertNotIn("arg3", kwargs)
    x.assertNotIn("arg6", kwargs)
    return kwargs['argnull']

def invokeArgsAndKeywordArgs(arg1, arg2, arg3=True, arg4=False, arg5=None, arg6=10, argnull=5.1):
    x = FakeTestCase()
    x.assertEqual(arg1, 15)
    x.assertEqual(arg2, "add")
    x.assertEqual(arg3, False)
    x.assertEqual(arg4, "xyz")
    x.assertIsNotNone(arg5)
    x.assertEqual(arg6, 10)
    x.assertIsNone(argnull)
    return [arg1, arg2, arg3, arg4, arg5, arg6, argnull]

class ClassWithMethod(object):
    """Use to test invoke on methods"""
    def theMethod(self, arg):
        return arg

objectWithMethod = ClassWithMethod()
