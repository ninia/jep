
HISTFILE = '.jep'

import traceback
import jep
from jep import *

has_readline = False

try:
    import readline

    try:
        has_readline = True
        import os
        HISTFILE = '%s/.jep' % (os.environ['HOME'])
        if(not os.access(HISTFILE, os.W_OK)):
            os.open(HISTFILE, os.O_CREAT)
        readline.read_history_file(HISTFILE)
    except IOError:
        traceback.print_exc()
        pass
except ImportError:
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
    try:
        line = None
        while True:
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
        if has_readline:
            readline.write_history_file(HISTFILE)


if __name__ == '__main__':
    Jep = findClass('jep.Jep')
    jep = Jep(True)

    # Pass argv to interactive prompt. We can't pass a Python object,
    # but we can make a new one.
    jep.eval("argv = %s" % argv)

    # make sure we can import from the current directory
    jep.eval('import sys; sys.path.append("")')

    try:
        prompt(jep)
    except:
        traceback.print_exc()

    print ''
    jep.close()
