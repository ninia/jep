"""
Fork of distutils' msvc9compiler to compile Jep on Windows.  This fork fixes the
following problems:
  1. Skip exporting the default init<module> method since Jep already has one
"""

from distutils._msvccompiler import MSVCCompiler as python_MSVCCompiler


class MSVCCompiler(python_MSVCCompiler):

    # Python requires a PyMODINIT_FUNC init<module> method entry point
    # for each symbol exported in a pyd file. This caused a duplicate
    # method warning during compile as pyembed already has an initjep.
    # If no exports are specified a default is added in the calling
    # method, this override removes the default addition.
    def link(self,
             target_desc,
             objects,
             output_filename,
             output_dir=None,
             libraries=None,
             library_dirs=None,
             runtime_library_dirs=None,
             export_symbols=None,
             debug=0,
             extra_preargs=None,
             extra_postargs=None,
             build_temp=None,
             target_lang=None):
        export_symbols = None
        python_MSVCCompiler.link(self,
                              target_desc,
                              objects,
                              output_filename,
                              output_dir,
                              libraries,
                              library_dirs,
                              runtime_library_dirs,
                              export_symbols,
                              debug,
                              extra_preargs,
                              extra_postargs,
                              build_temp,
                              target_lang)


    # Escape any backslash in the Windows path as a trailing backslash
    # will escape a end quote in the link command.
    def library_dir_option(self, dir):
        dir = dir.replace('\\', '\\\\')
        return python_MSVCCompiler.library_dir_option(self, dir)
