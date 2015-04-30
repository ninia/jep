"""
Fork of distutils' build_ext command to handle using a
patched version of the msvc9compiler.
"""

from distutils.command.build_ext import build_ext as old_build_ext

class build_ext (old_build_ext):

    def run(self):
        from commands.util import is_windows
        if is_windows():
            import sys
            import distutils
            import distutils.ccompiler
            from commands import msvc9compiler
            
            # See commands.msvc9compiler method comments for information on this override. 
            distutils.ccompiler.compiler_class['msvc']= ('jepmsvccompiler', 'MSVCCompiler', "Microsoft Visual C++")
            sys.modules['distutils.jepmsvccompiler'] = msvc9compiler
        old_build_ext.run(self)

# class build_ext