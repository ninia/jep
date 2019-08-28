import unittest
from java.lang import System

if __name__ == '__main__':
    tests = unittest.TestLoader().discover('src/test/python')
    result = unittest.TextTestRunner().run(tests)
    if not result.wasSuccessful():
        System.exit(1)
