import unittest
import sys
from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd


class TestInvokes(unittest.TestCase):

    @unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
    def test_inits(self):
        jep_pipe(build_java_process_cmd('jep.test.TestInvoke'))
