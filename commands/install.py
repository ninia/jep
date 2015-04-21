from distutils.command.install_data import install_data
from distutils import sysconfig
from distutils.spawn import spawn
import os
from commands.util import is_osx, is_windows


class post_install(install_data):
    def run(self):        
        install_data.run(self)        

        py_lib = py_lib = sysconfig.get_config_var('LIBDIR')

        # now let's give it a link that works for Java System.loadLibrary("jep")
        if is_windows():
            # windows actually supports symbolic links now?
            spawn(['mklink',
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
            
