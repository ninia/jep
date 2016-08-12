import unittest

if __name__ == '__main__':
    tests = unittest.TestLoader().discover('tests')
    unittest.TextTestRunner().run(tests)
