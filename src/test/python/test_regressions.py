import unittest
import sys

from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd
import jep
TestFieldTypes = jep.findClass('jep.test.types.TestFieldTypes')

class TestRegressions(unittest.TestCase):

    def test_out(self):
        from java.lang import System
        System.out.print("")

    def test_byte_value(self):
        from java.lang import Integer
        try:
            # call instance from class, should throw but not crash
            Integer.byteValue()
        except:
            pass

    def test_static_access_to_nonstatic_field(self):
        with self.assertRaises(TypeError):
            TestFieldTypes.objectString = ""

    def test_close_with_threads(self):
        jep_pipe(build_java_process_cmd('jep.test.TestCloseWithThreads'))

    def test_pyjtype_mro(self):
        # Both these throw errors with the default MRO
        jep.findClass('jep.test.TestPyJType$ProblemClass')
        jep.findClass('jep.test.TestPyJType$ProblemInterface')
        ChildTestingMethodInheritance = jep.findClass('jep.test.TestPyJType$ChildTestingMethodInheritance')
        self.assertEqual('ParentClassWithMethod', ChildTestingMethodInheritance().checkPrecedence())
        ClassInheritingDefault = jep.findClass('jep.test.TestPyJType$ClassInheritingDefault')
        self.assertEqual('InterfaceWithDefault', ClassInheritingDefault().checkPrecedence())

    def test_object_method_count(self):
        # There was a bug where each new sub-interpreter would add all the
        # Object methods onto multimethods on the Object type so the
        # multimethod would grow with each new sub-interpreter.
        from java.lang import Object
        self.assertEqual(3, len(Object.wait.__methods__))
        for i in range(3):
            SubInterpreterThread = jep.findClass('jep.test.util.SubInterpreterThread')
            thread = SubInterpreterThread();
            thread.start();
            thread.join();
        from java.lang import Object
        self.assertEqual(3, len(Object.wait.__methods__))