from traceback import print_exc
import unittest
import jep
from java.lang import Integer, String
from java.io import FileInputStream

class TestExceptions(unittest.TestCase):
    def test_number_format(self):
        try:
            Integer.parseInt('asdf')
        except Exception as ex:            
            self.assertIn('java.lang.NumberFormatException', str(ex.message))

    def test_io_exception(self):
        try:
            FileInputStream('asdf')
        except Exception as ex:
            self.assertIn('java.io.FileNotFoundException', str(ex.message))

    def test_null_pointer_exception(self):
        try:
            # throws http://stackoverflow.com/questions/3131865/why-does-string-valueofnull-throw-a-nullpointerexception
            String.valueOf(None)
        except Exception as ex:
            # because it's not a checked exception, mapped exceptions doesn't apply here (all Runtime)            
            self.assertIn('java.lang.NullPointerException', str(ex.message))
