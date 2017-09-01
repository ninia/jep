from __future__ import print_function
import traceback
import os
import sys

history_file = None

# python 3 renamed raw_input to input
if sys.version_info[0] >= 3:
    raw_input = input

has_readline = False

try:
    import readline
    has_readline = True
except ImportError:
    try:
        import pyreadline as readline
        has_readline = True
    except ImportError:
        msg = """
              No readline available. History will not be available.
              """
        if os.name == "posix":
            msg += """
                   You may want to set the LD_PRELOAD environment variable, see the
                   README file for details.

                   i.e.: export LD_PRELOAD=/usr/lib/libpython2.7.so.1.0
                   """
        elif os.name == "nt":
            msg += """
                   For Windows use pyreadline and get it from the official git
                   repo on github:
                   https://github.com/pyreadline/pyreadline

                   Do NOT use the version on pypi.python.org, and therefore
                   Do NOT use the version installed by pip.  It is out of date
                   and doesn't work with Jep!
                   """
        print(msg)
except OSError as e:
    if hasattr(e, 'winerror'):
        print("Windows error importing readline: " + str(e))
        print("Please try using the latest pyreadline from https://github.com/pyreadline/pyreadline")
    else:
        print("Error importing readline: " + str(e))

if has_readline:
    try:
        history_file = os.path.join(os.path.expanduser('~'), '.jep')
        if not os.path.exists(history_file):
            readline.write_history_file(history_file)
        else:
            readline.read_history_file(history_file)
    except IOError as err:
        pass


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
                printedErr = False
                try:
                    if len(err.args):
                        if 'printStackTrace' in dir(err.args[0]):
                            err.args[0].printStackTrace()
                            printedErr = True
                except Exception as exc:
                    print("Error printing stacktrace:", str(exc))
                finally:
                    if not printedErr:
                        print(str(err))

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
            except IOError as err:
                pass
