import unittest
import sys

if __name__ == '__main__':
    tests = unittest.TestLoader().discover('tests')
    result = unittest.TextTestRunner().run(tests)
    if not result.wasSuccessful():
        sys.exit(1)
