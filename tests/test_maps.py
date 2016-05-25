import unittest

from java.util import HashMap
from java.lang import Integer

def makeJavaMap():
    return makeDict(HashMap())

def makePythonDict():
    return makeDict({})

def makeDict(obj):
    obj["a"] = Integer(1)
    obj["b"] = Integer(2)
    obj["c"] = "XYZ"
    obj["a"] = Integer(-1)
    return obj
    

class TestMaps(unittest.TestCase):
    def setUp(self):
        pass
    
    def test_map(self):
        jmap = makeJavaMap()
        pymap = makePythonDict()
        self.assertEqual(jmap["a"], pymap["a"])
        self.assertEqual(jmap["b"], pymap["b"])
        self.assertEqual(jmap["c"], pymap["c"])
        self.assertEqual(len(jmap), len(pymap))

    def test_del(self):
        jmap = makeJavaMap()
        pymap = makePythonDict()
        del jmap['a']
        del pymap['a']
        
        with self.assertRaises(KeyError):
            del jmap['a']
        with self.assertRaises(KeyError):
            del pymap['a']
                
        pydict = {}
        for i in jmap:
            pydict[i] = jmap[i]
        self.assertEqual(pydict, pymap)
        
