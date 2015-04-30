
history_file = None

import traceback, os
import jep
from jep import *

has_readline = False

try:
    import readline

    try:
        has_readline = True        
        history_file = os.path.join(os.environ['HOME'], '.jep')
        if not os.path.exists(history_file):
            readline.write_history_file(history_file)
        else:
            readline.read_history_file(history_file)
    except IOError as e:
        pass
except ImportError:
    msg = """
    No readline available.
    """
    if os.name == "posix":
        msg += """
    You may want to set the LD_PRELOAD environment variable, see the
    README file for details.

    i.e.: export LD_PRELOAD=/usr/lib/libpython2.7.so.1.0
    """

    msg += """
    If your platform really doesn't have readline, try this:
    http://newcenturycomputers.net/projects/readline.html
    """
    print msg


PS1 = ">>> "
PS2 = "... "


def prompt(jep):
    try:
        line = None
        while True:
            ran = True
            try:
                ran = jep.eval(line)
            except Exception as err:
                # if a user uses exit(), don't print the error
                if 'exceptions.SystemExit' not in str(err.message):
                    traceback.print_exc()

            try:
                if ran:
                    line = raw_input(PS1)
                else:
                    line = raw_input(PS2)
            except:
                break

    finally:
        if has_readline:
            try:
                readline.write_history_file(history_file)
            except IOError:
                pass

