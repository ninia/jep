# test memory
# run with something like:
# java -Xmx2m -Xnoclassgc -classpath ../ jep.Run memtest.py

import sys
import os

# attach gdb
raw_input('gdb: %i' % (os.getpid()))

import jep
from jep import *

while 1:
    test = findClass('jep.Test')()
    test.run()
