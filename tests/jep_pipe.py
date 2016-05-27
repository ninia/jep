import subprocess
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

    try:
        p = subprocess.Popen(argv, stdout=subprocess.PIPE)
        yield(line.decode('utf-8') for line in p.stdout)
    finally:
        p.kill()
