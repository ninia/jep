import unittest
import sys
from tests.jep_pipe import jep_pipe
from tests.jep_pipe import build_java_process_cmd
import jep


@unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
class TestSharedModules(unittest.TestCase):

    def setUp(self):
        pass

    def test_shared_modules(self):
        jep_pipe(build_java_process_cmd('jep.test.TestSharedModules'))

    @unittest.skipIf(not jep.JEP_NUMPY_ENABLED, 'Jep library built without numpy support')
    def test_numpy_prod_succeeds(self):
        jep_pipe(build_java_process_cmd('jep.test.numpy.TestNumpyProdShared'))

    @unittest.skipIf(not jep.JEP_NUMPY_ENABLED, 'Jep library built without numpy support')
    def test_numpy_array_to_string(self):
        jep_pipe(build_java_process_cmd(
            'jep.test.numpy.TestNumpyArrayToString'))
