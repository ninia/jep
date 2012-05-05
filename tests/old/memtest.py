# test memory
# run with something like:
# java -Xmx2m -Xnoclassgc -classpath ../ jep.Run memtest.py

import os

import jep
from jep import *

Jep = findClass('jep.Jep')

for i in range(1, 50000):
    j = Jep()
    j.eval("a = %i" % i)
    print 'a = ', j.getValue("a")
    j.close()
    del j

# attach gdb
#raw_input('gdb: %i' % (os.getpid()))
