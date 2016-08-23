import unittest

import jep
Test = jep.findClass('jep.test.Test')
from java.util import ArrayList

COUNT = 17


def makeJavaList():
    jlist = ArrayList()
    for i in range(COUNT):
        jlist.add(i)
    return jlist


def makePythonList():
    pylist = []
    for i in range(COUNT):
        pylist.append(i)
    return pylist


class TestLists(unittest.TestCase):

    def setUp(self):
        self.test = Test()

    def test_sequence(self):
        jlist = makeJavaList()
        pylist = makePythonList()
        self.assertSequenceEqual(jlist, pylist)

    def test_len(self):
        jlist = makeJavaList()
        self.assertEqual(len(jlist), COUNT)

    def test_loop(self):
        n = 0
        jlist = makeJavaList()
        for i in jlist:
            n += 1
        self.assertEqual(n, COUNT)

    def test_contains(self):
        jlist = makeJavaList()
        self.assertTrue(14 in jlist)
        self.assertFalse(20 in jlist)
        self.assertFalse("abc" in jlist)
        self.assertFalse("0" in jlist)

    def test_getitem(self):
        jlist = makeJavaList()
        pylist = makePythonList()
        self.assertEqual(jlist[1], pylist[1])
        self.assertEqual(jlist[5], pylist[5])
        self.assertEqual(jlist[-1], pylist[-1])
        self.assertEqual(jlist[-5], pylist[-5])

    def test_getslice(self):
        jlist = makeJavaList()
        pylist = makePythonList()
        self.assertSequenceEqual(jlist[2:4], pylist[2:4])
        self.assertSequenceEqual(jlist[5:11], pylist[5:11])
        self.assertSequenceEqual(jlist[7:-2], pylist[7:-2])

    def test_setitem(self):
        jlist = makeJavaList()
        pylist = makePythonList()
        jlist[5] = 55
        pylist[5] = 55
        jlist[-3] = 99
        pylist[-3] = 99
        self.assertSequenceEqual(jlist, pylist)

    def test_setslice(self):
        jlist = makeJavaList()
        pylist = makePythonList()
        jlist[2:4] = [7, 1]
        pylist[2:4] = [7, 1]
        self.assertEqual(len(jlist), len(pylist))
        self.assertSequenceEqual(jlist, pylist)
        jlist[9:-5] = [4, 88, 19]
        pylist[9:-5] = [4, 88, 19]
        self.assertEqual(len(jlist), len(pylist))
        self.assertSequenceEqual(jlist, pylist)

    def test_add(self):
        jlist = makeJavaList()
        x = jlist + [1, 2, 3]
        self.assertIn('jep.PyJList', str(type(x)))
        self.assertEqual(len(x), COUNT + 3)

    def test_addequals(self):
        jlist = makeJavaList()
        pylist = makePythonList()
        jlist += [COUNT + 1]
        pylist += [COUNT + 1]
        self.assertSequenceEqual(jlist, pylist)
        toAdd = [1, 2, 3]
        jlist += toAdd
        pylist += toAdd
        self.assertSequenceEqual(jlist, pylist)
        with self.assertRaises(TypeError):
            jlist += None

    def test_multiply(self):
        jlist = makeJavaList()
        x = jlist * 3
        self.assertIn('jep.PyJList', str(type(x)))
        self.assertEqual(len(x), COUNT * 3)

    def test_multiplyequals(self):
        jlist = makeJavaList()
        pylist = makePythonList()
        jlist *= 2
        pylist *= 2
        self.assertSequenceEqual(jlist, pylist)

    def test_del(self):
        jlist = makeJavaList()
        pylist = makePythonList()
        with self.assertRaises(IndexError):
            del jlist[COUNT + 5]
        with self.assertRaises(IndexError):
            del pylist[COUNT + 5]

        del jlist[0]
        del pylist[0]
        self.assertSequenceEqual(jlist, pylist)

        del jlist[-1]
        del pylist[-1]
        self.assertSequenceEqual(jlist, pylist)

    def test_getstringbyindex(self):
        jlist = ArrayList()
        jlist.add("string")
        self.assertEqual(jlist[0], "string")

    def test_getstringbyiterator(self):
        jlist = ArrayList()
        jlist.add("string")
        self.assertEqual(next(iter(jlist)), "string")
