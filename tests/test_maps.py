import unittest

from java.util import HashMap


def makeJavaMap():
    return makeDict(HashMap())


def makePythonDict():
    return makeDict({})


def makeDict(obj):
    obj["a"] = 1
    obj["b"] = 2
    obj["c"] = "XYZ"
    obj["a"] = -1
    return obj


class TestMaps(unittest.TestCase):

    def test_iter(self):
        jmap = makeJavaMap()
        pymap = makePythonDict()
        for item in jmap:
            self.assertIn(item, pymap)
        for item in pymap:
            self.assertIn(item, jmap)

    def test_getitem(self):
        jmap = makeJavaMap()
        pymap = makePythonDict()
        self.assertEqual(jmap["a"], pymap["a"])
        self.assertEqual(jmap["b"], pymap["b"])
        self.assertEqual(jmap["c"], pymap["c"])

    def test_len(self):
        jmap = makeJavaMap()
        pymap = makePythonDict()
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
