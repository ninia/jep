from __future__ import print_function
import sys

def configure_error(*args, **kw):
    print('Error: ', file=sys.stderr)
    print(file=sys.stderr, *args, **kw)
    sys.exit(1)
