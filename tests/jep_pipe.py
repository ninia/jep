from __future__ import print_function
import subprocess
from subprocess import PIPE
from contextlib import contextmanager


@contextmanager
def jep_pipe(argv):
    """
    Launches a separate process to run a test.  This can be called from a
    unittest method.

    Primary use cases:
        1.  Testing that the JVM does not crash when interpreting specific
            Python statements.  We do not want to crash the current jep process
            that is running the unit tests.
        2.  Testing launching a Java main() process that sets up its own
            Jep instance for testing specific behavior.

    This will return the stdout of the separate process.  Therefore the
    separate process should print out expected output.  When asserting the
    output equals what is expected, the assert will need a trailing \n.

    Args:
            argv: a list of command line arguments to launch the process,
                    i.e. subprocess.Popen(argv)
                    example: ['jep', 'tests/some_python_file.py']

    Returns:
            the stdout of the process
    """
    p = subprocess.Popen(argv, stdout=PIPE, stderr=PIPE)
    stdout, stderr = p.communicate()
    if p.returncode != 0:
        if stderr:
            assert False, stderr
        else:
            assert False, stdout


def build_java_process_cmd(javaTestName):
    """
    Builds a list of args to provide to the jep_pipe method's call to
    subprocess.Popen.  This will take care of setting up the Java classpath
    and the -Djava.library.path variable necessary to run a Java main()
    from the command line.  This method should be used in conjunction with
    tests that get built into the jep.test jar and are Java mains().

    Args:
            javaTestName: a fully-qualified name of a Java class with a main()
                            method

    Returns:
            a list of values to be passed to jep_pipe (subprocess.Popen)
    """
    from java.lang import System
    cp = System.getProperty('java.class.path')

    lib_path = '-Djava.library.path='
    lib_path += System.getProperty("java.library.path")

    return ['java', '-ea', '-cp', cp, lib_path, javaTestName]


def build_python_process_cmd(pythonTestName):
    """
    Builds a list of args to provide to the jep_pipe method's call to
    subprocess.Popen.  This will take care of setting up the Java classpath
    and the -Djava.library.path variable necessary to run a Java main()
    from the command line.  This method should be used in conjunction with
    the jep.Run class and then running a Python file that uses Jep.

    Args:
            pythonTestName: path to a Python file that is using Jep


    Returns:
            a list of values to be passed to jep_pipe (subprocess.Popen)
    """
    java_cmd = build_java_process_cmd('jep.Run')
    java_cmd += [pythonTestName]
    return java_cmd
