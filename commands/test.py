from distutils.cmd import Command
from distutils.spawn import spawn
from commands.util import is_windows
from commands.util import configure_error
import os

class test(Command):
    description = "test build"

    user_options = [
        ('build-base=', 'b', "base directory for build library"),
        ('build-java=', 'j', "base directory for java classes"),
        ]

    def initialize_options(self):
        self.build_base = 'build'
        self.java_build = os.path.join('build', 'join')

    def finalize_options(self):
        pass

    def run(self):
        os.environ['CLASSPATH'] = 'build/java/jep.test-{0}.jar{1}tests/lib/sqlitejdbc-v056.jar'.format(self.distribution.metadata.get_version(), os.pathsep)
        if is_windows():
            # Use full path as spawn will only search the system PATH for *.exe on Windows
            if 'VIRTUAL_ENV' in os.environ:
                py_loc = os.environ['VIRTUAL_ENV']
            else:
                if 'PYTHONHOME' in os.environ:
                    py_loc = os.environ['PYTHONHOME']
                else:
                    configure_error('Please set the environment variable PYTHONHOME for running the tests on Windows without a virtualenv.')
            spawn(['{0}\Scripts\jep.bat'.format(py_loc), 'runtests.py'], search_path=0)
        else:
            spawn(['jep', 'runtests.py'])
