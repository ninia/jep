"""
Fork of distutils' install_lib command that installs the native
jep library, the jar file, and the python files to the
install_dir + 'jep', typically site-packages/jep.
"""

from commands.util import is_osx
from commands.util import is_windows
from commands.link_util import link_native_lib
from commands.python import get_python_lib_dir
from distutils.command.install_lib import install_lib
import os


class jep_install(install_lib):

    def install(self):
        build_ext = self.get_finalized_command('build_ext')
        jep_built_path = build_ext.get_outputs()[0]
        version = self.distribution.metadata.get_version()

        if os.path.isdir(self.build_dir):
            jep_install_dir = os.path.join(self.install_dir, 'jep')
            if not os.path.exists(jep_install_dir):
                os.makedirs(jep_install_dir)

            # let's put the native lib in site-packages/jep where it belongs
            lib_copied = self.copy_file(
                jep_built_path,
                os.path.join(jep_install_dir, os.path.basename(jep_built_path)))

            # let's copy the jar file too
            jar_name = 'jep-{0}.jar'.format(version)
            jar_copied = self.copy_file(
                os.path.join('build', 'java', jar_name),
                os.path.join(jep_install_dir, jar_name))

            # let's copy the jep package to site-packages
            py_copied = self.copy_tree(
                os.path.join(self.build_dir, 'jep'),
                jep_install_dir)

            # now let's give it a link that works for Java
            # System.loadLibrary("jep")
            link_native_lib(jep_install_dir, jep_built_path)
        else:
            self.warn("'%s' does not exist -- no Python modules to install" %
                      self.build_dir)
            return
