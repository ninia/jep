from __future__ import print_function
import unittest
from jep import JepImporter, findClass, JavaException


Jep = findClass('jep.Jep')
Test = findClass('jep.Test')


class TestImport(unittest.TestCase):
    def test_java_sql(self):
        from java.sql import DriverManager

    def test_not_found(self):
        importer = JepImporter()
        mod = importer.load_module('java.lang')
        mod.Integer
        self.assertRaises(AttributeError, mod.__getattr__, 'asdf')

    def test_restricted_classloader(self):
        # should use the supplied classloader for hooks
        vm = Jep()
        try:
            vm.setInteractive(True)
            vm.setClassLoader(Test.restrictedClassLoader)
            with self.assertRaises(RuntimeError) as e:
                vm.eval("from java.io import File")
                vm.eval('f = File("failed.txt")')
        finally:
            vm.close()

    def test_without_restricted_classloader(self):
        vm = Jep()
        try:
            vm.setInteractive(True)
            vm.eval("from java.io import File")
            vm.eval('f = File("failed.txt")')
        finally:
            vm.close()

    def test_class_import(self):
        from java.lang import System
        System.out.print('')  # first

        with self.assertRaises(ImportError) as e:
            import java.lang.System

        from java.lang import System
        System.out.print('')  # should still work
