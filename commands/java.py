from __future__ import print_function

from distutils.cmd import Command
from distutils.command import install
from distutils.spawn import spawn
from distutils.util import get_platform, newer
import platform
import os
import sys
from commands.util import configure_error

JAVA_HOME = os.environ.get('JAVA_HOME')
MAC_JAVA_HOME = '/Library/Java/Home'

def get_java_home():
    global JAVA_HOME
    if JAVA_HOME and os.path.exists(JAVA_HOME):
        return JAVA_HOME

    # mac's JAVA_HOME is predictable, just use that if we can
    if 'macosx' in get_platform() and os.path.exists(MAC_JAVA_HOME):
        JAVA_HOME = MAC_JAVA_HOME
        return JAVA_HOME

    configure_error("Please set JAVA_HOME to a path containing the JDK.")

def get_java_include():
    """
    Locate the Java include folder containing jni.h.
    """
    inc = os.path.join(JAVA_HOME, "include")
    if not os.path.exists(inc):
        configure_error("Include folder should be at '{0}' but doesn't exist. Please check you've installed the JDK properly.".format(inc))
    jni = os.path.join(inc, "jni.h")
    if not os.path.exists(jni):
        configure_error("jni.h should be in '{0}' but doesn't exist. Please check you've installed the JDK properly.".format(jni))
    return inc

def get_java_lib():
    lib = os.path.join(JAVA_HOME, "lib")
    if not os.path.exists(lib):
        configure_error("Lib folder should be at '{0}' but doesn't exist. Please check you've installed the JDK properly.".format(lib))
    return lib

def get_java_linker_args():
    if platform.system() == 'Darwin':
        return ['-framework JavaVM']

class build_java(Command):
    outdir = None

    user_options = [
        ('javac=', None, 'use javac (default: JAVA_HOME/bin/javac)'),
    ]

    def initialize_options(self):
        build_java.outdir = os.path.join('build', 'java')
        if not os.path.exists(build_java.outdir):
            os.makedirs(build_java.outdir)

        self.java_files = []
        self.javac = os.path.join(get_java_home(), 'bin', 'javac')

    def finalize_options(self):
        self.java_files = self.distribution.java_files

    def build(self, *jclasses):
        spawn([self.javac, '-deprecation', '-d', build_java.outdir, '-classpath', 'src/'] + list(*jclasses))

    def run(self):
        tobuild = []
        for jclass in self.java_files:
            tobuild.append(jclass)
        self.build(tobuild)

class build_javah(Command):
    outdir = None

    user_options = [
        ('javah=', None, 'use javah (default: JAVA_HOME/bin/javah)'),
    ]

    def initialize_options(self):
        build_javah.outdir = os.path.join('build', 'include')
        if not os.path.exists(build_javah.outdir):
            os.mkdir(build_javah.outdir)

        self.javah = os.path.join(get_java_home(), 'bin', 'javah')
        self.javah_files = []

    def finalize_options(self):
        self.javah_files = self.distribution.javah_files

    def build(self, jclass, header):
        spawn([self.javah, '-classpath', build_java.outdir, '-o', os.path.join(build_javah.outdir, header), jclass])

    def run(self):
        for jclass, header in self.javah_files:
            self.build(jclass, header)
