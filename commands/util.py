from __future__ import print_function
from collections import namedtuple
from distutils.util import get_platform
import subprocess
import sys


def configure_error(*args, **kw):
    print('Error: ', file=sys.stderr)
    print(file=sys.stderr, *args, **kw)
    sys.exit(1)


def warning(*args):
    print(*args, file=sys.stderr)


def is_osx():
    return 'macosx' in get_platform()


def is_windows():
    return 'win' in get_platform()


def is_bsd():
    return 'bsd' in get_platform()


class CommandFailed(Exception):
    """
    The command failed to run for any reason
    """
    pass


class CommandError(CommandFailed):
    """
    The command returned an exit code
    """

    def __init__(self, message, result):
        self.result = result
        super(CommandError, self).__init__(message)


class CommandResult(namedtuple('Result', 'stdout, stderr, returncode, failed')):
    pass


def shell(command, capture=True):
    """
    Run a command on the local system.

    This is borrowed from fabric.operations, with simplifications

    `local` is simply a convenience wrapper around the use of the builtin
    Python ``subprocess`` module with ``shell=True`` activated. If you need to
    do anything special, consider using the ``subprocess`` module directly.

    `local` is not currently capable of simultaneously printing and
    capturing output, as `~fabric.operations.run`/`~fabric.operations.sudo`
    do. The ``capture`` kwarg allows you to switch between printing and
    capturing as necessary, and defaults to ``False``.

    When ``capture=False``, the local subprocess' stdout and stderr streams are
    hooked up directly to your terminal, though you may use the global
    :doc:`output controls </usage/output_controls>` ``output.stdout`` and
    ``output.stderr`` to hide one or both if desired. In this mode, the return
    value's stdout/stderr values are always empty.

    When ``capture=True``, you will not see any output from the subprocess in
    your terminal, but the return value will contain the captured
    stdout/stderr.
    """
    if capture:
        out_stream = subprocess.PIPE
        err_stream = subprocess.PIPE
    else:
        # Non-captured streams are left to stdout
        out_stream = subprocess.STDOUT
        err_stream = subprocess.STDOUT

    try:
        cmd_arg = command if is_windows() else [command]
        p = subprocess.Popen(cmd_arg, shell=True,
                             stdout=out_stream, stderr=err_stream)
        stdout, stderr = p.communicate()
    except Exception:
        e = CommandFailed('command failed', sys.exc_info()[1])
        e.__traceback__ = sys.exc_info()[2]
        raise e

    # Handle error condition (deal with stdout being None, too)
    out = stdout.strip() if stdout else ""
    err = stderr.strip() if stderr else ""

    failed = p.returncode != 0
    result = CommandResult(out, err, p.returncode, failed)

    if result.failed:
        msg = "Encountered an error (return code %s) while executing '%s'" % (
            p.returncode, command)
        raise CommandError(message=msg, result=result)

    return result
