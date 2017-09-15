import unittest
import sys

if __name__ == '__main__':
    tests = unittest.TestLoader().discover('src/test/python')
    result = unittest.TextTestRunner().run(tests)
    if not result.wasSuccessful():
        sys.exit(1)
