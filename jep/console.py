
try:
    import readline
except:
    print """
    No readline available.
    You may want to set the LD_PRELOAD environment variable, see the
    README file for details. Ignore this if you're running on Windows.

    i.e.: export LD_PRELOAD=/usr/lib/libpython2.3.so.1.0 """

import traceback
import jep
from jep import *

PS1 = ">>> "
PS2 = "... "


def prompt(jep):
    line = raw_input(PS1)
    while(1):
        ran = True
        try:
            ran = jep.eval(line)
        except:
            traceback.print_exc()
            pass

        try:
            if(ran):
                line = raw_input(PS1)
            else:
                line = raw_input(PS2)
        except(EOFError):
            break


if(__name__ == '__main__'):
    Jep = findClass('jep.Jep')
    jep = Jep(True)
    
    try:
        prompt(jep)
    except:
        pass
    
    jep.close()

