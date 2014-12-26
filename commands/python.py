from distutils import sysconfig
from commands.util import is_osx

def get_python_libs():
    """
    Get the shared library names for embedding jep.

    See python-config
    """
    return ['python' + sysconfig.get_config_var('VERSION'), 'dl']

def get_python_linker_args():
    if is_osx():
        return []
    return ['-L{0}'.format(sysconfig.get_config_var('LIBDIR'))]
