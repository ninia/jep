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

    def test_inner_class(self):
        from java.lang import Thread
        self.assertEquals(Thread.currentThread().getState(), Thread.State.RUNNABLE)

    def test_type_name(self):
        from java.util import Date
        self.assertEquals("java.util", Date.__pytype__.__module__)
        self.assertEquals("Date", Date.__pytype__.__name__)

    def test_type_inheritance(self):
        from java.lang import Object
        from java.io import Serializable
        from java.util import Date
        self.assertTrue(isinstance(Date(), Object.__pytype__))
        self.assertTrue(isinstance(Date(), Serializable.__pytype__))

