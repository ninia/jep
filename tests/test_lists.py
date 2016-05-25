import unittest

import jep
Test = jep.findClass('jep.test.Test')
from java.util import ArrayList
from java.lang import Integer

COUNT = 17

def makeJavaList():
    jlist = ArrayList()
    for i in range(COUNT):
        jlist.add(Integer(i))
    return jlist

def makePythonList():
    pylist = []
    for i in range(COUNT):
        # At present have to make it a java.lang.Integer for
        # assertSequenceEqual to work. If in the future a  
        # java.lang.Integer can compare equality to a python int,
        # then this should be updated to use python ints.
        pylist.append(Integer(i))
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
        self.assertTrue(Integer(14) in jlist)
        self.assertFalse(Integer(20) in jlist)
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
        jlist[5] = Integer(55)
        pylist[5] = Integer(55)
        jlist[-3] = Integer(99)
        pylist[-3] = Integer(99)
        self.assertSequenceEqual(jlist, pylist)

    def test_setslice(self):
        jlist = makeJavaList()
        pylist = makePythonList()
        jlist[2:4] = [Integer(7), Integer(1)]
        pylist[2:4] = [Integer(7), Integer(1)]
        self.assertEqual(len(jlist), len(pylist))
        self.assertSequenceEqual(jlist, pylist)
        jlist[9:-5] = [Integer(4), Integer(88), Integer(19)]
        pylist[9:-5] = [Integer(4), Integer(88), Integer(19)]
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
        jlist += [Integer(COUNT + 1)]
        pylist += [Integer(COUNT + 1)]
        self.assertSequenceEqual(jlist, pylist)
        toAdd = [Integer(1), Integer(2), Integer(3)]
        jlist += toAdd
        pylist += toAdd
        self.assertSequenceEqual(jlist, pylist)
        
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
            del jlist[COUNT+5]
        with self.assertRaises(IndexError):
            del pylist[COUNT+5]
        
        del jlist[0]
        del pylist[0]
        self.assertSequenceEqual(jlist, pylist)
        
        del jlist[-1]
        del pylist[-1]
        self.assertSequenceEqual(jlist, pylist)
