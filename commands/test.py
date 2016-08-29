from __future__ import print_function
from distutils.cmd import Command
from distutils import sysconfig
from commands.util import configure_error
from commands.util import is_osx
from commands.util import is_windows
from commands.link_util import link_native_lib
from commands.python import get_libpython
import os
import sys


class test(Command):
    description = "test build"

    user_options = [
        ('build-base=', 'b', "base directory for build library"),
        ('build-java=', 'j', "base directory for java classes"),
    ]

    def initialize_options(self):
        self.build_base = 'build'
        self.java_build = os.path.join('build', 'java')

    def finalize_options(self):
        pass

    def run(self):
        # make sure we're testing the latest
        self.run_command('build')

        # setup java classpath
        version = self.distribution.metadata.get_version()
        classpath = os.path.join(self.java_build, 'jep-' + version + '.jar')
        classpath += os.pathsep + os.path.join(self.java_build, 'jep.test-' + version + '.jar')
        classpath += os.pathsep + 'tests/lib/sqlitejdbc-v056.jar'
        classpath += os.pathsep + 'tests/lib/fakenetty.jar'

        # setup environment variables
        environment = {}
        if not is_osx() and not is_windows():
            environment['LD_LIBRARY_PATH'] = sysconfig.get_config_var('LIBDIR')

            # set the LD_PRELOAD environment variable if we can locate the
            # libpython<version>.so library.
            lib_python = get_libpython()
            if lib_python:
                environment['LD_PRELOAD'] = lib_python

        # http://bugs.python.org/issue20614
        if is_windows():
            environment['SYSTEMROOT'] = os.environ['SYSTEMROOT']

        # if multiple versions of python are installed, this helps ensure the right
        # version is used
        executable = sys.executable
        if executable:
            environment['PATH'] = os.path.dirname(executable) + os.pathsep + os.environ['PATH']

        # find the jep library and makes sure it's named correctly
        build_ext = self.get_finalized_command('build_ext')
        jep_lib = build_ext.get_outputs()[0]
        built_dir = os.path.dirname(jep_lib)
        link_native_lib(built_dir, jep_lib)

        # actually kick off the tests
        import subprocess
        args = ['java',
                '-classpath', '{0}'.format(classpath),
                '-Djava.library.path={0}'.format(built_dir),
                'jep.Run', 'tests/runtests.py']
        p = subprocess.Popen(args, env=environment)
        p.wait()
