import unittest
import sys
from tests.jep_pipe import jep_pipe
from tests.jep_pipe import build_java_process_cmd
import jep

@unittest.skipIf(not jep.JEP_NUMPY_ENABLED, 'Jep library built without numpy support')
class TestSharedModules(unittest.TestCase):
    def setUp(self):
        pass

    @unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
    def test_numpy_prod_fails(self):
        jep_pipe(build_java_process_cmd('jep.test.numpy.TestNumpyProdLost'))


    @unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
    def test_numpy_prod_ordering(self):
        jep_pipe(build_java_process_cmd('jep.test.numpy.TestNumpyProdOrdering'))


    @unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
    def test_numpy_prod_succeeds(self):
        jep_pipe(build_java_process_cmd('jep.test.numpy.TestNumpyProdShared'))


    @unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
    def test_numpy_array_to_string(self):
        jep_pipe(build_java_process_cmd('jep.test.numpy.TestNumpyArrayToString'))
