import unittest
import time

import jep
Test = jep.findClass('jep.test.Test')
from java.lang import Integer
from java.util import Date


class TestCompare(unittest.TestCase):

    def setUp(self):
        self.test = Test()

    def test_int_compare(self):
        x = Integer(5)
        y = Integer(7)

        self.assertTrue(x < y)
        self.assertFalse(x > y)
        self.assertTrue(x <= y)
        self.assertFalse(x >= y)
        self.assertTrue(x != y)
        self.assertFalse(x == y)

        self.assertTrue(y > x)
        self.assertFalse(y < x)
        self.assertTrue(y >= x)
        self.assertFalse(y <= x)
        self.assertTrue(y != x)
        self.assertFalse(y == x)

        z = Integer(7)
        self.assertFalse(y < z)
        self.assertFalse(y > z)
        self.assertTrue(y >= z)
        self.assertTrue(y <= z)
        self.assertFalse(y != z)
        self.assertTrue(y == z)

        self.assertFalse(z < y)
        self.assertFalse(z > y)
        self.assertTrue(z >= y)
        self.assertTrue(z <= y)
        self.assertFalse(z != y)
        self.assertTrue(z == y)

    def test_date_compare(self):
        x = Date()
        time.sleep(.1)
        y = Date()

        self.assertTrue(x < y)
        self.assertFalse(x > y)
        self.assertTrue(x <= y)
        self.assertFalse(x >= y)
        self.assertTrue(x != y)
        self.assertFalse(x == y)

        self.assertTrue(y > x)
        self.assertFalse(y < x)
        self.assertTrue(y >= x)
        self.assertFalse(y <= x)
        self.assertTrue(y != x)
        self.assertFalse(y == x)

    def test_equals(self):
        self.assertEqual(self.test, self.test)
        self.assertNotEqual(self.test, self.test.getObject().get(0))
        self.assertNotEqual(Integer, self.test)
        self.assertNotEqual(self.test, Integer)
        self.assertEqual(Integer, Integer)
        from java.lang import Class
        self.assertNotEqual(Integer, Class)
