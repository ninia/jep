import unittest
import sys
from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd


class TestPreInits(unittest.TestCase):

    def test_inits(self):
        jep_pipe(build_java_process_cmd('jep.test.TestPreInitVariables'))
