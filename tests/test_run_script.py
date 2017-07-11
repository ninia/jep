import unittest
import os
import os.path

from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd


class TestRunScript(unittest.TestCase):

    def setUp(self):
        with open('build/testScript.py', 'w') as testScript:
            testScript.write("def isGood():\n")
            testScript.write("    return True\n")
        

    def test_compiledScript(self):
        jep_pipe(build_java_process_cmd('jep.test.TestCompiledScript'))


    def tearDown(self):
        if os.path.exists('build/testScript.py'):
            os.remove('build/testScript.py')
        if os.path.exists('build/testScript.pyc'):
            os.remove('build/testScript.pyc')

