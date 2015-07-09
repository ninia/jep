from distutils.command.install_lib import install_lib
from distutils import sysconfig
import os
from commands.util import is_osx, is_windows
from commands.python import get_python_lib_dir
from distutils.spawn import spawn


class jep_install(install_lib):
    
    def install(self):
        build_ext = self.get_finalized_command('build_ext')
        jep_lib = build_ext.get_outputs()[0]
        version = self.distribution.metadata.get_version()

        if os.path.isdir(self.build_dir):
            jep_dir = os.path.join(self.install_dir, 'jep')
            if not os.path.exists(jep_dir):
               os.makedirs(jep_dir)

            # let's put the native lib in site-packages/jep where it belongs
            lib_copied = self.copy_file(
                                        jep_lib,
                                        os.path.join(jep_dir, os.path.basename(jep_lib)))

            # let's copy the jar file too
            jar_name = 'jep-{0}.jar'.format(version)
            jar_copied = self.copy_file(
                                        os.path.join('build', 'java', jar_name),
                                        os.path.join(jep_dir, jar_name))
        
            # let's copy the jep package to site-packages
            py_copied = self.copy_tree(
                                  os.path.join(self.build_dir, 'jep'),
                                  jep_dir)

            # now let's give it a link that works for Java System.loadLibrary("jep")
            self.link_native_lib(jep_dir, os.path.basename(jep_lib))
        else:
            self.warn("'%s' does not exist -- no Python modules to install" %
                      self.build_dir)
            return


    def link_native_lib(self, jep_dir, jep_lib):
        # we'll put the jep_dir as -Djava.library.path in the jep script
        if is_windows():
            jep_dll = os.path.join(jep_dir, 'jep.dll')
            # Remove the old DLL if it exists to avoid a file move error.
            if os.path.exists(jep_dll):
                os.remove(jep_dll)
            # Do not use 'spawn' as that will run as a non-administrative user
            # that may no longer have access to the destination directory.
            self.move_file(os.path.join(jep_dir, jep_lib), jep_dll)

        elif is_osx():
            # Apple says to put the file at /Library/Java/Extensions/libjep.jnilib,
            # which is good for a permanent install but not so good when using
            # virtualenv or testing
            spawn(['ln',
                   '-sf',
                   '{0}'.format(jep_lib),
                   '{0}'.format(os.path.join(jep_dir, 'libjep.jnilib')),])

        else:
            # otherwise, distutils outputs 'jep.so' which needs to be linked
            # to libjep.so. The JVM will not find the library without.
            spawn(['ln',
                   '-sf',
                   '{0}'.format(jep_lib),
                   '{0}'.format(os.path.join(jep_dir, 'libjep.so')),
                   ])
 
