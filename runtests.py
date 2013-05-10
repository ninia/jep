#!/usr/bin/env jep

import unittest

import sys
# make sure we can run scripts from the current folder
sys.path.insert(0, '')

if __name__ == '__main__':
    try:
        unittest.main(module='tests')
    except SystemExit:
        pass
