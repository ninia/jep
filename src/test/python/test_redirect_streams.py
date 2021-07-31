import unittest

from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd

class TestRedirectStreams(unittest.TestCase):

    def test_compiledScript(self):
        jep_pipe(build_java_process_cmd('jep.test.TestRedirectStreams'))

