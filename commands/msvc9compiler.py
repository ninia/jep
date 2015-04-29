"""
Fork of distutils' msvc9compiler to bypass the removal of MSVC manifest
from the output file and skip export of the python default init<module>
methond.
"""

from distutils.msvc9compiler import MSVCCompiler as old_MSVCCompiler

class MSVCCompiler(old_MSVCCompiler) :

    # Remove export symbols as we will not be loading this in Python
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
    
    # Retain the manifest as Java will load the DLL 
    def _remove_visual_c_ref(self, manifest_file):
        return manifest_file