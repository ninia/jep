"""
Fork of distutils' msvc9compiler to bypass the removal of MSVC manifest
from the output file and skip export of the python default init<module>
methond.
"""

from distutils.msvc9compiler import MSVCCompiler as old_MSVCCompiler

class MSVCCompiler(old_MSVCCompiler) :

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
        export_symbols=None
        old_MSVCCompiler.link(self,
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
    
    # MSVCCompiler will strip any manifest additions from a non-executable
    # that contain MSVC runtime information. This is so that when the pyd 
    # is loaded it will use Pythons MSVC runtime version. Since we are loading
    # the resulting library from Java we must retain the manifest so Java can 
    # load the DLL. This returns the unmodified manifest file.
    def _remove_visual_c_ref(self, manifest_file):
        return manifest_file