from __future__ import print_function
from distutils.command.clean import clean
import shutil


class really_clean(clean):

    def run(self):
        print('removing ', self.build_base)
        shutil.rmtree(self.build_base, ignore_errors=True)
        print('removing ', self.build_lib)
        shutil.rmtree(self.build_lib, ignore_errors=True)
        print('removing ', self.build_scripts)
        shutil.rmtree(self.build_scripts, ignore_errors=True)
        print('removing ', self.build_temp)
        shutil.rmtree(self.build_temp, ignore_errors=True)
        print('removing ', self.bdist_base)
        shutil.rmtree(self.bdist_base, ignore_errors=True)
