import unittest

from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd

class TestAllowPythonEnquirer(unittest.TestCase):

    def test_allow_python_enquirer(self):
        jep_pipe(build_java_process_cmd('jep.test.TestAllowPythonEnquirer'))
