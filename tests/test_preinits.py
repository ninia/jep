import unittest
import sys

from .jep_pipe import build_java_process_cmd

class TestPreInits(unittest.TestCase):
    def setUp(self):
        pass

    @unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
    def test_inits(self):        
        expected = [
                    # TODO: figure out how to get the unit test to work with
                    #"Py_NoSiteFlag 1",
                    "Py_NoSiteFlag 0\n",
                    "Py_IgnoreEnvironmentFlag 1\n",
                    "Py_NoUserSiteDirectory 1\n"
                    ]
        
        index = 0
        from .jep_pipe import jep_pipe
        # TestPreInitVariables will call subprocess/py_preinit.py
        with jep_pipe(build_java_process_cmd('jep.test.TestPreInitVariables')) as p:
            self.assertEqual(next(p), expected[index])
            index += 1
