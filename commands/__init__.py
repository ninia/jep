from __future__ import print_function
from distutils.command.build import build
from commands.java import build_javah, build_java


class jep_build(build):
    sub_commands = [
        ('setup_java', None),
        ('build_java', None),
        ('build_javah', None),
        ('build_jar', None),
    ] + build.sub_commands
