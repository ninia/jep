from __future__ import print_function

from distutils.cmd import Command
from distutils.spawn import spawn
from distutils.util import get_platform, newer
import os
import sys

JAVA_HOME = os.environ.get('JAVA_HOME')
MAC_JAVA_HOME = '/Library/Java/Home'

def configure_error(*args, **kw):
    print('Error: ', file=sys.stderr)
    print(file=sys.stderr, *args, **kw)
    sys.exit(1)

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

class build_java(Command):
    user_options = [
        ('javac=', None, 'use javac (default: JAVA_HOME/bin/javac)'),
    ]

    def initialize_options(self):
        self.java_files = []
        self.javac = os.path.join(get_java_home(), 'bin', 'javac')

    def finalize_options(self):
        self.java_files = self.distribution.java_files

    def build(self, jclass):
        spawn([self.javac, '-classpath', 'src/', jclass])

    def run(self):
        for jclass in self.java_files:
            jobj = jclass.replace('.java', '.class')
            if newer(jclass, jobj):
                self.build(jclass)

class build_javah(Command):
    user_options = [
        ('javah=', None, 'use javah (default: JAVA_HOME/bin/javah)'),
    ]

    def initialize_options(self):
        self.javah = os.path.join(get_java_home(), 'bin', 'javah')
        self.javah_files = []

    def finalize_options(self):
        self.javah_files = self.distribution.javah_files

    def build(self, jclass, header):
        spawn([self.javah, '-classpath', 'src/', '-o', header, jclass])

    def run(self):
        for jclass, header in self.javah_files:
            self.build(jclass, header)
