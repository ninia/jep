from distutils.command.install_lib import install_lib
from distutils import sysconfig
import os
from commands.util import is_osx, is_windows


class jep_install(install_lib):
    
    def install(self):                                   
        if is_windows():
            jep_lib = 'jep.pyd'
        else:
            jep_lib = 'jep.so'
        py_lib = sysconfig.get_config_var('LIBDIR')
        
        if os.path.isdir(self.build_dir):        
            # let's put the file in python/lib where it belongs
            lib_copied = self.copy_file(
                                        os.path.join(self.build_dir, jep_lib),
                                        os.path.join(py_lib, jep_lib))
        
            # let's copy the jep package to site-packages        
            return self.copy_tree(
                                  os.path.join(self.build_dir, 'jep'),
                                  os.path.join(self.install_dir, 'jep'))                
        else: 
            self.warn("'%s' does not exist -- no Python modules to install" %
                      self.build_dir)
            return
                    
