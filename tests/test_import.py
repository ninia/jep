from __future__ import print_function
import unittest
from jep import JepJavaImporter, findClass


Jep = findClass('jep.Jep')
Test = findClass('jep.test.Test')


class TestImport(unittest.TestCase):

    def setUp(self):
        self.test = Test()

    def test_java_sql(self):
        from java.sql import DriverManager

    def test_not_found(self):
        importer = JepJavaImporter()
        mod = importer.load_module('java.lang')
        mod.Integer
        self.assertRaises(ImportError, mod.__getattr__, 'asdf')

    def test_restricted_classloader(self):
        # should use the supplied classloader for hooks
        self.test.testRestrictedClassLoader()

    def test_without_restricted_classloader(self):
        from java.io import File
        dir(File)

    def test_class_import(self):
        from java.lang import System
        System.out.print('')  # first

        with self.assertRaises(ImportError) as e:
            import java.lang.System

        from java.lang import System
        System.out.print('')  # should still work

    def test_conflicting_package(self):
        from io import DEFAULT_BUFFER_SIZE
