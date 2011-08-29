import unittest
import jep
from java.lang import Integer
from java.io import FileInputStream

class TestExceptions(unittest.TestCase):
    def test_number_format(self):
        if jep.USE_MAPPED_EXCEPTIONS:
            try:
                Integer.parseInt('asdf')
            except jep.NumberFormatException as ex:
                pass
            else:
                self.assertTrue(False, 'Did not throw NumberFormatException')

    def test_io_exception(self):
        if jep.USE_MAPPED_EXCEPTIONS:
            try:
                FileInputStream('asdf')
            except jep.FileNotFoundException:
                pass
            else:
                self.assertTrue(False, 'Did not throw FileInputStream')
