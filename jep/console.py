
HISTFILE = '.jep'

import traceback
import jep
from jep import *
from threading import Thread

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
    README file for details. Ignore this if you're running on Windows.

    i.e.: export LD_PRELOAD=/usr/lib/libpython2.3.so.1.0 """


# we do the prompting from a non-java thread, that way signals can be ignored
# this thread must not call any java methods. JNI's env pointer is not to be
# shared between threads...
class Prompt(Thread):
    PS1  = ">>> "
    PS2  = "... "
    ran  = True
    line = None
    eof  = False

    def __init__(self, ran):
        Thread.__init__(self)
        self.ran = ran
        
        
    def run(self):
        try:
            if(self.ran):
                self.line = raw_input(self.PS1)
            else:
                self.line = raw_input(self.PS2)
        except(EOFError):
            self.eof = True


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

            p = Prompt(ran)
            p.start()
            p.join()
            line = p.line

            if(p.eof):
                break
    finally:
        if(hasReadline):
            readline.write_history_file(HISTFILE)


if(__name__ == '__main__'):
    Jep = findClass('jep.Jep')
    jep = Jep(True)
    
    try:
        prompt(jep)
    except:
        traceback.print_exc()

    print ''
    jep.close()
