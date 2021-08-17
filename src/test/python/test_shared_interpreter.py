import unittest
import sys
from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd

class TestSharedInterpreter(unittest.TestCase):

    def test_shared_interpreter(self):
        jep_pipe(build_java_process_cmd('jep.test.TestSharedInterpreter'))

