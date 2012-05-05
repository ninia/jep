import unittest

import sys
# embedding does not setup argv
sys.argv = ['']
# make sure we can run scripts from the current folder
sys.path.insert(0, '')

if __name__ == '__main__':
    try:
        unittest.main(module='tests', argv=[''])
    except SystemExit:
        pass
    
