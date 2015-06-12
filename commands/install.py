from distutils.command.install_data import install_data
from distutils import sysconfig
from distutils.spawn import spawn
import os
from commands.util import is_osx, is_windows
from commands.python import get_python_lib_dir


class post_install(install_data):
    def run(self):        
        py_lib = get_python_lib_dir()

        # Get the "--root" directory supplied to the "install" command,
        # and use it as a prefix to LIBDIR.
        install_root = self.get_finalized_command('install').root
        if install_root is not None:
            py_lib = os.path.join(install_root + py_lib)
            if not os.path.exists(py_lib):
                os.makedirs(py_lib)

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
            
