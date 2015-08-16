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
              No readline available.
              """
        if os.name == "posix":
            msg += """
                   You may want to set the LD_PRELOAD environment variable, see the
                   README file for details.

                   i.e.: export LD_PRELOAD=/usr/lib/libpython2.7.so.1.0
                   """
        # TODO actually pyreadline doesn't work right with Jep on Windows   :\
        msg += """
               If your platform doesn't have readline, try this:
               https://pypi.python.org/pypi/readline
               """
        print(msg)
except WindowsError as we:
    print("Windows error importing readline: " + str(we))

if has_readline:
    try:
        history_file = os.path.join(os.environ['HOME'], '.jep')
        if not os.path.exists(history_file):
           readline.write_history_file(history_file)
        else:
            readline.read_history_file(history_file)
    except (IOError, KeyError) as e:
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
