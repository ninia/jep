import unittest
import jep
TestAutoCloseable = jep.findClass('jep.test.closeable.TestAutoCloseable')

class TestAutoCloseables(unittest.TestCase):

    def test_with(self):
        with TestAutoCloseable() as writer:
            writer.write("abc")
            self.assertFalse(writer.isClosed())
        self.assertTrue(writer.isClosed())
    
    def test_io_exception(self):
        with TestAutoCloseable() as writer:
            writer.write("abc")
        with self.assertRaises(IOError):
            writer.write("def")

    def test_inner_exception(self):
        try:
            with TestAutoCloseable() as writer:
                writer.write("abc")
                from java.fake import ArrayList
                writer.write("def")
        except ImportError as exc:
            pass
        self.assertTrue(writer.isClosed())
            
