from distutils import sysconfig
from commands.util import is_osx, is_windows
import os

def get_python_libs():
    """
    Get the shared library names for embedding jep.

    See python-config
    """
    v = sysconfig.get_config_var('VERSION')
    ldv = sysconfig.get_config_var('LDVERSION')
    if ldv:
        v = ldv
    libs = ['python' + v]
    if not is_windows():
        libs.append('dl')
    return libs

def get_python_linker_args():
    if is_windows():
        return []
    return ['-L{0}'.format(sysconfig.get_config_var('LIBDIR'))]

def get_python_lib_dir():
    if is_windows():
        return os.path.join(os.environ.get('PYTHONHOME'), 'DLLs')

    return sysconfig.get_config_var('LIBDIR')

