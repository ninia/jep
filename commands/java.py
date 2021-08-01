from __future__ import print_function

from distutils import log
from distutils.cmd import Command
from distutils.spawn import spawn, find_executable
from distutils.dep_util import newer_group

import shutil
import os
import fnmatch
import traceback
import sys

from commands.util import configure_error
from commands.util import is_osx
from commands.util import shell
from commands.util import CommandFailed
from commands.util import warning
from commands.util import is_windows


_java_home = None


def get_java_home():
    global _java_home
    if _java_home is not None:
        return _java_home

    env_home = os.environ.get('JAVA_HOME')
    if env_home:
        if is_windows():
           # remove quotes from each end if necessary
            env_home = env_home.strip('"')
        if os.path.exists(env_home):
            _java_home = env_home
            return env_home
        else:
            configure_error('Path ' + env_home +
                            ' indicated by JAVA_HOME does not exist.')

    if is_osx():
        # newer macs have an executable to help us
        try:
            result = shell('/usr/libexec/java_home')
            _java_home = result.stdout.decode('utf-8')
            return _java_home
        except CommandFailed:
            traceback.print_exc()

    configure_error(
        'Please set the environment variable JAVA_HOME to a path containing the JDK.')


def get_java_include():
    """
    Locate the Java include folders for compiling JNI applications.
    """
    inc_name = 'include'
    inc = os.path.join(get_java_home(), inc_name)
    if not os.path.exists(inc):
        configure_error("Include folder should be at '{0}' but doesn't exist. "
                        "Please check you've installed the JDK properly.".format(inc))
    jni = os.path.join(inc, "jni.h")
    if not os.path.exists(jni):
        configure_error("jni.h should be in '{0}' but doesn't exist. "
                        "Please check you've installed the JDK properly.".format(jni))

    paths = [inc]

    # Include platform specific headers if found
    include_linux = os.path.join(inc, 'linux')
    if os.path.exists(include_linux):
        paths.append(include_linux)

    include_darwin = os.path.join(inc, 'darwin')
    if os.path.exists(include_darwin):
        paths.append(include_darwin)

    include_win32 = os.path.join(inc, 'win32')
    if os.path.exists(include_win32):
        paths.append(include_win32)

    include_bsd = os.path.join(inc, 'freebsd')
    if os.path.exists(include_bsd):
        paths.append(include_bsd)
    return paths


def get_java_lib():
    lib_name = 'lib'
    lib = os.path.join(get_java_home(), lib_name)
    if not os.path.exists(lib):
        configure_error("Lib folder should be at '{0}' but doesn't exist. "
                        "Please check you've installed the JDK properly.".format(lib))
    return lib


def get_java_libraries():
    if not is_osx():
        return ['jvm']
    return []


def get_java_lib_folders():
    if not is_osx():
        if is_windows():
            jre = os.path.join(get_java_home(), 'lib')
        else:
            jre = os.path.join(get_java_home(), 'jre', 'lib')
            if not os.path.exists(jre):
                jre = os.path.join(get_java_home(), 'lib')
        folders = []
        for root, dirnames, filenames in os.walk(jre):
            if is_windows():
                for filename in fnmatch.filter(filenames, '*jvm.lib'):
                    folders.append(os.path.join(
                        root, os.path.dirname(filename)))
            else:
                for filename in fnmatch.filter(filenames, '*jvm.so'):
                    folders.append(os.path.join(
                        root, os.path.dirname(filename)))

        return list(set(folders))
    return []


def get_java_linker_args():
    return []


def get_output_jar_paths(version):
    """
    Gets the list of output jars, which includes the primary
    jep-${version}.jar, the test jar, and the src jars.
    """
    return [
        'build/java/jep-{0}-sources.jar'.format(version),
        'build/java/jep-{0}.jar'.format(version),
        'build/java/jep-{0}-test-sources.jar'.format(version),
        'build/java/jep-{0}-test.jar'.format(version)
    ]


def skip_java_build(command):
    """
    Checks if the .java files in the src directory are newer than
    the build directory's .jar files.  If so, returns True that
    the java build can be skipped, otherwise returns False.
    """
    version = command.distribution.metadata.get_version()
    jar_newer = False
    for outjar in get_output_jar_paths(version):
        jar_newer |= newer_group(
            command.distribution.java_files, outjar, 'newer')
    if not jar_newer:
        return True
    return False


class setup_java(Command):
    """
    Output some useful information about the java environment
    This is useful when people report errors
    """

    user_options = []

    def initialize_options(self):
        pass

    def run(self):
        if skip_java_build(self):
            log.debug('skipping Java build (up-to-date)')
            return
        warning('Using JAVA_HOME:', get_java_home())

        if is_osx():
            target = os.environ.get('MACOSX_DEPLOYMENT_TARGET')

            if target:
                warning(
                    'INFO: the MACOSX_DEPLOYMENT_TARGET environment variable is set:', target)

                result = shell('sw_vers -productVersion')
                if target.split('.')[:2] != result.stdout.decode("utf-8").split('.')[:2]:
                    warning(
                        'This target appears to be incorrect for the system version:', result.stdout)

    def finalize_options(self):
        pass


class build_java(Command):
    outdir = None
    testoutdir = None

    user_options = [
        ('javac=', None,
         'use javac (default: {0}/bin/javac)'.format(get_java_home())),
    ]

    def initialize_options(self):
        build_java.outdir = os.path.join('build', 'java')
        if not os.path.exists(build_java.outdir):
            os.makedirs(build_java.outdir)
        build_java.testoutdir = os.path.join(build_java.outdir, 'test')
        if not os.path.exists(build_java.testoutdir):
            os.makedirs(build_java.testoutdir)
        build_java.headeroutdir = os.path.join('build', 'include')
        if not os.path.exists(build_java.headeroutdir):
            os.makedirs(build_java.headeroutdir)

        self.java_files = []
        self.javac = os.path.join(get_java_home(), 'bin', 'javac')

    def finalize_options(self):
        self.java_files = self.distribution.java_files

    def build(self, *jclasses):
        jep = [x for x in list(*jclasses) if not x.startswith('src{0}test{0}java{0}'.format(os.sep))]
        tests = [x for x in list(*jclasses) if x.startswith('src{0}test{0}java{0}'.format(os.sep))]
        spawn([self.javac, '-deprecation', '-d', build_java.outdir, '-h', build_java.headeroutdir, '-classpath', 'src'] + jep)
        spawn([self.javac, '-deprecation', '-d', build_java.testoutdir, '-classpath', '{0}{1}src'.format(build_java.outdir, os.pathsep)] + tests)
        # Copy the source files over to the build directory to make src.jar's.
        self.copySrc('jep', jep)
        self.copySrc('jep.test', tests)

    def run(self):
        if not skip_java_build(self):
            self.build(self.java_files)
        else:
            log.debug('skipping building .class files (up to date)')

    def copySrc(self, app, files):
        for src in files:
            dest = os.path.join(build_java.outdir, '{0}.{1}'.format(app, src))
            print('copying {0} to {1}'.format(src, dest))
            if not os.path.exists(os.path.dirname(dest)):
                os.makedirs(os.path.dirname(dest))
            shutil.copy(src, dest)


class build_jar(Command):
    outdir = None

    user_options = [
        ('jar=', None,
         'use jar (default: {0}/bin/jar)'.format(get_java_home())),
    ]

    def initialize_options(self):
        build_jar.outdir = os.path.join('build', 'java')
        if not os.path.exists(build_jar.outdir):
            os.makedirs(build_jar.outdir)

        self.version = []
        self.java_files = []
        self.extra_jar_files = []
        self.jar = os.path.join(get_java_home(), 'bin', 'jar')

    def finalize_options(self):
        self.extra_jar_files = self.distribution.extra_jar_files
        self.version = self.distribution.metadata.get_version()

    def build(self):
        for src in self.extra_jar_files:
            dest = os.path.join(*['build/java/jep'])
            dest_dir = os.path.dirname(dest)
            print('copying {0} to {1}'.format(src, dest))
            if not os.path.exists(dest_dir):
                os.makedirs(dest_dir)
            shutil.copy(src, dest)

        spawn([self.jar, '-cf', 'build/java/jep-{0}-sources.jar'.format(self.version), '-C', 'build/java/jep.src/main/java', 'jep'])
        spawn([self.jar, '-cfe', 'build/java/jep-{0}.jar'.format(self.version), 'jep.Run', '-C', 'build/java', 'jep'])
        spawn([self.jar, '-cf', 'build/java/jep-{0}-test-sources.jar'.format(self.version), '-C', 'build/java/jep.test.src/test/java', 'jep'])
        spawn([self.jar, '-cfe', 'build/java/jep-{0}-test.jar'.format(self.version), 'test.jep.Test', '-C', 'build/java/test', 'jep'])

    def run(self):
        if not skip_java_build(self):
            self.build()
        else:
            log.debug('skipping packing jar files (up to date)')
