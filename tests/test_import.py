import unittest
from jep import JepImporter, findClass


class TestImport(unittest.TestCase):
    def test_java_sql(self):
        from java.sql import DriverManager

    def test_not_found(self):
        importer = JepImporter()
        mod = importer.load_module('java.lang')
        mod.Integer
        self.assertRaises(AttributeError, mod.__getattr__, 'asdf')
