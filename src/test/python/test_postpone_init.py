import unittest
import sys
from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd
import jep


@unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
class TestPostponeInit(unittest.TestCase):

    def test_postpone_init(self):
        jep_pipe(build_java_process_cmd('jep.test.TestPostponeInit'))
