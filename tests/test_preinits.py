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
                    "Py_NoUserSiteDirectory 1\n",
                    # TODO figure out how to get the unit test to work with
                    # "Py_VerboseFlag 1\n"
                    "Py_VerboseFlag 0\n",
                    "Py_OptimizeFlag 1\n",
                    "Py_DontWriteBytecodeFlag 1\n",
                    "Py_HashRandomizationFlag 1\n"
                    ]
        
        index = 0
        from .jep_pipe import jep_pipe
        from itertools import izip
        with jep_pipe(build_java_process_cmd('jep.test.TestPreInitVariables')) as p:
            for got_line, expected_line in izip(p, expected):
                self.assertEqual(got_line, expected_line)
