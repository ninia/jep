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
        if is_windows():
            os.environ['CLASSPATH'] = 'build/java/;tests/lib/sqlitejdbc-v056.jar'
            spawn(['build\scripts-2.7\jep.bat', 'runtests.py'])
        else:
            os.environ['CLASSPATH'] = 'build/java/:tests/lib/sqlitejdbc-v056.jar'
            spawn(['jep', 'runtests.py'])
