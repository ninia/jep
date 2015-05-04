from distutils.cmd import Command
from distutils.spawn import spawn
from commands.util import is_windows
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
            spawn(['{0}\Scripts\jep.bat'.format(os.environ['PYTHONHOME']), 'runtests.py'], search_path=0)
        else:
            spawn(['jep', 'runtests.py'])
