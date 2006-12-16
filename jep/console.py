
HISTFILE = '.jep'

import traceback
import jep
from jep import *

hasReadline = False

try:
    import readline

    try:
        hasReadline = True
        import os
        HISTFILE = '%s/.jep' % (os.environ['HOME'])
        if(not os.access(HISTFILE, os.W_OK)):
            os.open(HISTFILE, os.O_CREAT)
        readline.read_history_file(HISTFILE)
    except:
        traceback.print_exc()
        pass
except:
    print """
    No readline available.
    You may want to set the LD_PRELOAD environment variable, see the
    README file for details.


    i.e.: export LD_PRELOAD=/usr/lib/libpython2.3.so.1.0

    If your platform really doesn't have readline, try this:
    http://newcenturycomputers.net/projects/readline.html """


PS1  = ">>> "
PS2  = "... "


def prompt(jep):
    global hasReadline
    
    try:
        line = None
        while(1):
            ran = True
            try:
                ran = jep.eval(line)
            except:
                traceback.print_exc()

            try:
                if(ran):
                    line = raw_input(PS1)
                else:
                    line = raw_input(PS2)
            except:
                break

    finally:
        if(hasReadline):
            readline.write_history_file(HISTFILE)


if(__name__ == '__main__'):
    Jep = findClass('jep.Jep')
    jep = Jep(True)

    # Pass argv to interactive prompt. We can't pass a Python object,
    # but we can make a new one.
    jep.eval("argv = %s" % argv)

    try:
        prompt(jep)
    except:
        traceback.print_exc()

    print ''
    jep.close()
