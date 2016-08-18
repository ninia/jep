from distutils import sysconfig
from commands.util import is_osx
from commands.util import is_windows
from commands.util import is_bsd
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
    if not is_windows() and not is_bsd():
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


def get_libpython():
    """
    Searches for the Python library, e.g. libpython<version>.so.
    Primarily used for setting up LD_PRELOAD.
    """
    lib_python = os.path.join(sysconfig.get_config_var('LIBDIR'),
                              sysconfig.get_config_var('LDLIBRARY'))
    if os.path.exists(lib_python):
        return lib_python
    else:
        # x64 systems will tend to also have a MULTIARCH folder
        lib_python = os.path.join(sysconfig.get_config_var('LIBDIR'),
                                  sysconfig.get_config_var('MULTIARCH'),
                                  sysconfig.get_config_var('LDLIBRARY'))
        if os.path.exists(lib_python):
            return lib_python
