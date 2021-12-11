import unittest
import sys
from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd

def containsBug46006():
    # cpython 3.10.0 and 3.10.1 are affected by https://bugs.python.org/issue46006.
    # Since this bug should not occur in most environments we do not want to stop 
    # jep from building by failing.
    # More discussion can be found at https://github.com/ninia/jep/issues/358
    # At the time of this writing the bug has not been fixed and may affect
    # upcoming releases such as 3.10.2 and 3.11.
    if sys.version_info.major == 3 and sys.version_info.minor == 10: 
        return sys.version_info.micro == 0 or sys.version_info.micro == 1

class TestPreInits(unittest.TestCase):

    @unittest.skipIf(containsBug46006, 'Python version contains cpython bug 46006 and may not work with DontWriteBytecodeFlag')
    def test_inits(self):
        jep_pipe(build_java_process_cmd('jep.test.TestPreInitVariables'))


