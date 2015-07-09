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
        py_lib = get_python_lib_dir()

        # Get the "--root" directory supplied to the "install" command,
        # and use it as a prefix to LIBDIR.
        install_root = self.get_finalized_command('install').root
        if install_root is not None:
            py_lib = os.path.join(install_root + py_lib)
            if not os.path.exists(py_lib):
                os.makedirs(py_lib)
        
        if os.path.isdir(self.build_dir):        
            # let's put the file in python/lib where it belongs
            lib_copied = self.copy_file(
                                        jep_lib,
                                        os.path.join(py_lib, os.path.basename(jep_lib)))

            # let's copy the jar file too
            jar_dir = os.path.join(py_lib, 'jep')
            if not os.path.exists(jar_dir):
                os.makedirs(jar_dir)
            jar_copied = self.copy_file(
                                        os.path.join('build', 'java', 'jep-{0}.jar'.format(version)),
                                        os.path.join(jar_dir, 'jep-{0}.jar'.format(version)))
        
            # let's copy the jep package to site-packages        
            py_copied = self.copy_tree(
                                  os.path.join(self.build_dir, 'jep'),
                                  os.path.join(self.install_dir, 'jep'))

            self.link_native_lib(py_lib) 
        else: 
            self.warn("'%s' does not exist -- no Python modules to install" %
                      self.build_dir)
            return
                   

    def link_native_lib(self, py_lib):
        # now let's give it a link that works for Java System.loadLibrary("jep")
        if is_windows():
            spawn(['mv',
                   '{0}'.format(os.path.join(py_lib, 'jep.pyd')),
                   '{0}'.format(os.path.join(py_lib, 'jep.dll')),
                   ])
            
        elif is_osx():
            # link the jep.so output file to /Library/Java/Extensions/libjep.jnilib
            spawn(['ln',
                   '-sf',
                   '{0}'.format(os.path.join(py_lib, 'jep.so')),                   
                   '/Library/Java/Extensions/libjep.jnilib',])

        else:
            # otherwise, distutils outputs 'jep.so' which needs to be linked
            # to libjep.so. The JVM will not find the library without.
            spawn(['ln',
                   '-sf',
                   '{0}'.format('jep.so'),
                   '{0}'.format(os.path.join(py_lib, 'libjep.so')),
                   ])
