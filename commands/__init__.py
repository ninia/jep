# https://github.com/pypa/setuptools/issues/2928
from distutils.command.build import build
from commands.java import build_java

class jep_build(build):
    sub_commands = [
        ('setup_java', None),
        ('build_java', None),
        ('build_jar', None),
    ] + build.sub_commands
