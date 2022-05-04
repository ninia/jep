from commands.util import is_osx
from commands.util import is_windows
from commands.util import is_bsd
import os
import sysconfig


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
    Used by setup.py to set PYTHON_LDLIBRARY, and by scripts to set up LD_PRELOAD.
    """
    libdir = sysconfig.get_config_var('LIBDIR')
    ldlibrary = sysconfig.get_config_var('LDLIBRARY')
    if libdir is None or ldlibrary is None:
        return None

    lib_python = os.path.join(libdir, ldlibrary)
    if os.path.exists(lib_python):
        return lib_python

    # x64 systems will tend to also have a MULTIARCH folder
    multiarch = sysconfig.get_config_var('MULTIARCH')
    if multiarch is not None:
        lib_python = os.path.join(libdir, multiarch, ldlibrary)
        if os.path.exists(lib_python):
            return lib_python

    # HACK: Non-existent static library is a known issue with conda-forge python;
    # see: https://github.com/conda-forge/python-feedstock/issues/565
    # Let's also look for a shared library in this case.
    if ldlibrary.endswith('.a'):
        ldshared = ldlibrary[:-1] + 'so'
        lib_python = os.path.join(libdir, ldshared)
        if os.path.exists(lib_python):
            return lib_python
        if multiarch is not None:
            lib_python = os.path.join(libdir, multiarch, ldshared)
            if os.path.exists(lib_python):
                return lib_python

    # give up
    return None
