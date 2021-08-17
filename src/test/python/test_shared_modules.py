import unittest
import sys
from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd
import jep


class TestSharedModules(unittest.TestCase):

    def test_shared_modules(self):
        jep_pipe(build_java_process_cmd('jep.test.TestSharedModules'))

    def test_shared_modules_threads(self):
        jep_pipe(build_java_process_cmd('jep.test.TestSharedModulesThreads'))

    @unittest.skipIf(not jep.JEP_NUMPY_ENABLED, 'Jep library built without numpy support')
    def test_numpy_prod_succeeds(self):
        jep_pipe(build_java_process_cmd('jep.test.numpy.TestNumpyProdShared'))

    @unittest.skipIf(not jep.JEP_NUMPY_ENABLED, 'Jep library built without numpy support')
    def test_numpy_array_to_string(self):
        jep_pipe(build_java_process_cmd(
            'jep.test.numpy.TestNumpyArrayToString'))

    def test_unshared(self):
        # The default jep has no shared modules so it should not be possible
        # to access mainInterpreterModules
        with self.assertRaises(AttributeError):
            jep.mainInterpreterModules
    
    def test_shared_argv(self):
        jep_pipe(build_java_process_cmd('jep.test.TestSharedArgv'))
