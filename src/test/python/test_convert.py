import unittest
import jep
from datetime import datetime
from java.sql import Time
from java.util import Calendar, Date, TimeZone

def date_to_datetime(jdate):
    return datetime.utcfromtimestamp(jdate.getTime()/1000)

class TestConvert(unittest.TestCase):

    def setUp(self):
        Test = jep.findClass('jep.test.Test')
        self.javaPassThrough = Test().testObjectPassThrough

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
        # Convert Dates, not Times
        jep.setJavaToPythonConverter(Date, date_to_datetime)
        jep.setJavaToPythonConverter(Time, lambda x: x)
        after = self.javaPassThrough(Date())
        self.assertNotIsInstance(after, Date)
        self.assertIsInstance(after, datetime)
        after = self.javaPassThrough(Time(1))
        self.assertIsInstance(after, Date)
        self.assertNotIsInstance(after, datetime)
        jep.setJavaToPythonConverter(Date, None)
        jep.setJavaToPythonConverter(Time, None)

    def tearDown(self):
        # make sure converters are deregistered since it shouldn't affect other tests.
        jep.setJavaToPythonConverter(Date, None)
        jep.setJavaToPythonConverter(Time, None)
