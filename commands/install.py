from distutils.command.install_data import install_data
from distutils.spawn import spawn
import os
from commands.util import is_osx

class post_install(install_data):
    def run(self):
        install_data.run(self)

        install = self.get_finalized_command('install')

        if is_osx():
            # link the jep.so output file to /Library/Java/Extensions/libjep.jnilib
            spawn(['ln',
                   '-sf',
                   '{0}'.format(os.path.join(install.install_lib, 'jep.so')),
                   '/Library/Java/Extensions/libjep.jnilib',])

        else:
            # otherwise, distutils outputs 'jep.so' which needs to be linked
            # to libjep.so. The JVM will not find the library without.
            spawn(['ln',
                   '-sf',
                   '{0}'.format(os.path.join(install.install_lib, 'jep.so')),
                   '{0}'.format(os.path.join(install.install_lib, 'libjep.so')),
                   ])
            
