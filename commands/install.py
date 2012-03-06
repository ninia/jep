from distutils.command.install import install
from distutils.spawn import spawn
import os
from commands.util import is_osx

class post_install(install):
    def run(self):
        install.run(self)

        if is_osx():
            # link the jep.so output file to /Library/Java/Extensions/libjep.jnilib
            spawn(['ln',
                   '-sf',
                   '{0}'.format(os.path.join(self.install_lib, 'jep.so')),
                   '/Library/Java/Extensions/libjep.jnilib',])
