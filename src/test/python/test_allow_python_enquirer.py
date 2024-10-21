import sys
import unittest

from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd

class TestAllowPythonEnquirer(unittest.TestCase):

    @unittest.skipIf(sys.version_info.major == 3 and sys.version_info.minor < 7,
            "Test needs re.Pattern which was added in Python 3.7")
    def test_allow_python_enquirer(self):
        jep_pipe(build_java_process_cmd('jep.test.TestAllowPythonEnquirer'))
