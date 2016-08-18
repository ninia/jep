"""
Helper functions for linking and finding libraries.
"""

from commands.util import is_osx
from commands.util import is_windows
from distutils import sysconfig
from distutils.spawn import spawn
import os
import os.path
import shutil


def link_native_lib(output_dir, jep_lib_path):
    """
    Links or moves the native jep library (jep.so, jep.dll) to where it
    can be found by Java.

    Args:
            output_dir: the directory the linked/moved file should be in
                        should match the value passed as -Djava.library.path
            jep_lib_path: the path to the jep library
                          e.g. build/lib.linux-x86_64-2.7/jep.so
    """
    jep_lib = os.path.basename(jep_lib_path)
    if is_windows():
        jep_dll = os.path.join(output_dir, 'jep.dll')
        # Remove the old DLL if it exists to avoid a file move error.
        if os.path.exists(jep_dll):
            os.remove(jep_dll)
        # Do not use 'spawn' as that will run as a non-administrative user
        # that may no longer have access to the destination directory.
        shutil.copy(os.path.join(output_dir, jep_lib), jep_dll)

    elif is_osx():
        # Apple says to put the file at /Library/Java/Extensions/libjep.jnilib,
        # which is good for a permanent install but not so good when using
        # virtualenv or testing.
        spawn(['ln',
               '-sf',
               '{0}'.format(jep_lib),
               '{0}'.format(os.path.join(output_dir, 'libjep.jnilib')), ])

    else:
        # Otherwise, distutils outputs 'jep.so' which needs to be linked
        # to 'libjep.so'. Otherwise the JVM will not find the library.
        spawn(['ln',
               '-sf',
               '{0}'.format(jep_lib),
               '{0}'.format(os.path.join(output_dir, 'libjep.so')),
               ])
