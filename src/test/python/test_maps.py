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

    def test_keys(self):
        jmap = makeJavaMap()
        keylist = jmap.keys()
        self.assertIn("a", keylist)
        self.assertIn("b", keylist)
        self.assertIn("c", keylist)
        self.assertNotIn("d", keylist)
        self.assertNotIn(2, keylist)

    def test_items(self):
        jmap = makeJavaMap()
        itemlist = jmap.items()
        self.assertIn(("a", -1), itemlist)
        self.assertIn(("b", 2), itemlist)
        self.assertIn(("c", "XYZ"), itemlist)
        self.assertNotIn(("a", 1), itemlist)
        self.assertNotIn(("a", 2), itemlist)
        self.assertNotIn("a", itemlist)
        self.assertNotIn(2, itemlist)

    def test_map_to_dict(self):
        jmap = makeJavaMap()
        pydict = dict(jmap)
        for k in pydict:
            self.assertEqual(pydict[k], jmap[k])
