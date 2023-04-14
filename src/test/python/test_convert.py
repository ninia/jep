import unittest
import jep
from datetime import datetime
from java.util import Calendar, Date, HashMap, Map, TimeZone, TreeMap

def map_to_dict(jmap):
    result = {}
    for entry in jmap.entrySet():
        result[entry.getKey()] = entry.getValue()
    return result

def date_to_datetime(jdate):
    return datetime.utcfromtimestamp(jdate.getTime()/1000)

class TestConvert(unittest.TestCase):

    def setUp(self):
        Test = jep.findClass('jep.test.Test')
        self.javaPassThrough = Test().testObjectPassThrough

    def test_j2p_converter_map(self):
        # Register a conversion function, all Maps coming from java are converted to dicts
        jep.setJavaToPythonConverter(Map, map_to_dict)
        # Constructors do not convert so construct a java Map for testing.
        jmap = HashMap()
        jmap.put("key1","value1")
        jmap.put("key2","value2")

        # The passthrough is a java method that returns whatever it is given so the resulting
        # value is automatically passed through the converter by jep.
        after = self.javaPassThrough(jmap)
        # The result is not a java Map
        self.assertNotIsInstance(after, Map)
        # It is a python Dict
        self.assertIsInstance(after, dict)
        # a Map is not equal to a dict
        self.assertNotEqual(jmap, after)
        # But it has items() which a Map does not
        self.assertTrue(hasattr(after,"items"))
        # The contents of the dict match what you would expect from a map
        self.assertEqual(len(jmap), len(after))
        self.assertIn(("key1", "value1"), after.items())
        self.assertIn(("key2", "value2"), after.items())

        # Remove the converter to return to the default behavior
        jep.setJavaToPythonConverter(Map, None)
        after = self.javaPassThrough(jmap)
        # With no converter the result is a Map, not a dict
        self.assertIsInstance(after, Map)
        self.assertNotIsInstance(after, dict)
        self.assertEqual(jmap, after)
        self.assertFalse(hasattr(after,"items"))
        
    def test_j2p_converter_date(self):
        cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"))
        cal.clear() # Set to January 1, 1970 00:00:00.000 GMT 

        # Register a conversion function, all Dates coming from java are converted to datetimes
        jep.setJavaToPythonConverter(Date, date_to_datetime)
        # A java method returning a Date will use the converter
        after = cal.getTime()
        # The result is not a java Date
        self.assertNotIsInstance(after, Date)
        # It is a python datetime
        self.assertIsInstance(after, datetime)
        # datetime fields wok as expected
        self.assertEqual(1970, after.year)
        self.assertEqual(1, after.month)
        self.assertEqual(1, after.day)
        self.assertEqual(0, after.hour)
        self.assertEqual(0, after.minute)
        self.assertEqual(0, after.second)

        # Remove the converter to return to the default behavior
        jep.setJavaToPythonConverter(Date, None)
        after = cal.getTime()
        # With no converter the result is a Map, not a dict
        self.assertIsInstance(after, Date)
        self.assertNotIsInstance(after, datetime)
        # The Date methods work but use the timezone from your locale so values are system dependent.
        self.assertIsInstance(after.getYear(), int)
        self.assertIsInstance(after.getMonth(), int)
        self.assertIsInstance(after.getDay(), int)
        self.assertIsInstance(after.getHours(), int)
        self.assertIsInstance(after.getMinutes(), int)
        self.assertIsInstance(after.getSeconds(), int)
        
    def test_j2p_converter_override(self):
        # Convert all Maps, except TreeMaps
        jep.setJavaToPythonConverter(Map, map_to_dict)
        jep.setJavaToPythonConverter(TreeMap, lambda x: x)
        after = self.javaPassThrough(HashMap())
        self.assertNotIsInstance(after, Map)
        self.assertIsInstance(after, dict)
        after = self.javaPassThrough(TreeMap())
        self.assertIsInstance(after, Map)
        self.assertNotIsInstance(after, dict)

    def tearDown(self):
        # make sure converters are deregistered since it shouldn't affect other tests.
        jep.setJavaToPythonConverter(Map, None)
        jep.setJavaToPythonConverter(Date, None)
        jep.setJavaToPythonConverter(TreeMap, None)
