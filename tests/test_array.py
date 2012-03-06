import unittest

class TestArray(unittest.TestCase):
    def test_long_len(self):
        import java.lang
        String = java.lang.String
        c = String(''.join(['t'] * 1269000)).toCharArray()
        self.assertEqual(1269000, len(c))

if __name__ == '__main__':
    unittest.main()
