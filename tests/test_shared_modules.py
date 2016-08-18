import unittest
import sys
from tests.jep_pipe import jep_pipe
from tests.jep_pipe import build_java_process_cmd

class TestGetValue(unittest.TestCase):
    def setUp(self):
        pass

    @unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
    def test_get_collection_boxing(self):
        jep_pipe(build_java_process_cmd('jep.test.TestGetCollectionBoxing'))


    @unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
    def test_get_bytearray(self):
        jep_pipe(build_java_process_cmd('jep.test.TestGetByteArray'))
