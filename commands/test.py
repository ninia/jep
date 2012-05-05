from distutils.cmd import Command
from distutils.spawn import spawn
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
        spawn(['java', '-cp', 'build/java/', 'jep.Test'])
