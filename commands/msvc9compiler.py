"""
Fork of distutils' msvc9compiler to compile Jep on Windows.  This fork fixes the
following problems:
  1. Bypass the removal of MSVC manifest from the pyd/dll file, which Jep needs
  2. Skip exporting the default init<module> method since Jep already has one
  3. Add /MANIFEST arg to the linker for building with MSVC 2010 for Python 3
  4. Better vcvarsall detection to support Microsoft Visual C++ Compiler for Python 2.7
"""

import distutils
from distutils import msvc9compiler as old_msvc_module
from distutils.msvc9compiler import MSVCCompiler as old_MSVCCompiler


class MSVCCompiler(old_MSVCCompiler):

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

    # Escape any backslash in the Windows path as a trailing backslash
    # will escape a end quote in the link command.
    def library_dir_option(self, dir):
        dir = dir.replace('\\', '\\\\')
        return old_MSVCCompiler.library_dir_option(self, dir)

    # MSVC 2010 Express wants an explicit /MANIFEST argument for the linker,
    # see http://bugs.python.org/issue4431
    def manifest_setup_ldargs(self, output_filename, build_temp, ld_args):
        old_MSVCCompiler.manifest_setup_ldargs(
            self, output_filename, build_temp, ld_args)
        ld_args.append('/MANIFEST')

# More advanced detection of vcvarsall.bat.  Borrowed/modified from setuptools.
# see https://bugs.python.org/issue23246


def find_vcvarsall(version):
    Reg = distutils.msvc9compiler.Reg
    VC_BASE = r'Software\%sMicrosoft\DevDiv\VCForPython\%0.1f'
    key = VC_BASE % ('', version)
    try:
        # Per-user installs register the compiler path here
        productdir = Reg.get_value(key, "installdir")
    except KeyError:
        try:
            # All-user installs on a 64-bit system register here
            key = VC_BASE % ('Wow6432Node\\', version)
            productdir = Reg.get_value(key, "installdir")
        except KeyError:
            productdir = None

    if productdir:
        import os
        vcvarsall = os.path.join(productdir, "vcvarsall.bat")
        if os.path.isfile(vcvarsall):
            return vcvarsall
    else:
        # this implies that MSVC++ for Python2.7 is not installed, and we
        # should fall back to attempting through MSVC from Visual Studio
        return old_find_vcvarsall(version)

old_find_vcvarsall = old_msvc_module.find_vcvarsall
old_msvc_module.find_vcvarsall = find_vcvarsall
