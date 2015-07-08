import unittest, time

from java.lang import Integer
from java.util import Date

    

class TestCompare(unittest.TestCase):
    def setUp(self):
        pass
    
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