from __future__ import print_function
from distutils.util import get_platform
import sys

def configure_error(*args, **kw):
    print('Error: ', file=sys.stderr)
    print(file=sys.stderr, *args, **kw)
    sys.exit(1)

def is_osx():
    return 'macosx' in get_platform()

