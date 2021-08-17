import unittest
import sys
from jep_pipe import jep_pipe
from jep_pipe import build_java_process_cmd
from java.lang import Object


class TestSynchronized(unittest.TestCase):

    def test_synchronized(self):
        """
        Tests that multiple threads in both Python and Java using the same Java
        object as a lock will correctly allow only one thread inside the
        synchronized/with code blocks at a time.
        """
        jep_pipe(build_java_process_cmd('jep.test.synchronization.TestCrossLangSync'))

    def test_with(self):
        "Simple test illustrating basic usage"
        x = Object()
        with x.synchronized():
            y = 5 + 3
        self.assertEqual(y, 8)

    def test_explicit(self):
        "Tests explicit method calls, please don't do this in application code"
        x = Object()
        slock = x.synchronized()
        slock.__enter__()
        y = 5 + 3
        slock.__exit__()
        self.assertEqual(y, 8)

    def test_bad_form(self):
        "Tests attempting unlock of a non-locked object, please don't do this in application code"
        x = Object()
        slock = x.synchronized()
        with self.assertRaises(RuntimeError):
            slock.__exit__()

    def test_with_exception_unlocks(self):
        "Tests that the end of the with block will unlock even if an exception occurred"
        x = Object()
        slock = x.synchronized()
        try:
            with slock:
                from java.fake import ArrayList
        except ImportError as exc:
            pass
        with self.assertRaises(RuntimeError):
            # you cannot unlock that which has been unlocked
            slock.__exit__()

