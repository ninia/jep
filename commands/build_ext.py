"""
Fork of distutils' build_ext command to handle using a
patched version of the msvc9compiler.
"""

import os.path
from distutils.command.build_ext import build_ext as old_build_ext


class build_ext (old_build_ext):

    def finalize_options(self):
        # if no build_lib is specified then default to build the ext inside the
        # jep directory to match the final location in an installed system.
        default_build_lib = self.build_lib is None
        old_build_ext.finalize_options(self)
        if default_build_lib:
            self.build_lib = os.path.join(self.build_lib, "jep")

    def run(self):
        from commands.util import is_windows
        if is_windows():
            import sys
            import distutils
            import distutils.ccompiler
            from commands import _msvccompiler

            # See commands.msvc9compiler method comments for information on
            # this override.
            distutils.ccompiler.compiler_class['msvc'] = (
                'jepmsvccompiler', 'MSVCCompiler', "Microsoft Visual C++")
            sys.modules['distutils.jepmsvccompiler'] = _msvccompiler
        old_build_ext.run(self)

        # copy the jep.pyd to jep.dll early to avoid confusion
        if is_windows():
            jep_lib = self.get_outputs()[0]
            dll = jep_lib.replace('pyd', 'dll')
            self.copy_file(jep_lib, dll)

# class build_ext
