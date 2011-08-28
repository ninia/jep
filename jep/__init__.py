__VERSION__ = '3.0.dev1'
VERSION = __VERSION__
try:
    from _jep import *
    from hook import *
except:
    pass


