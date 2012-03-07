#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import codecs
import platform

import os
from distutils.core import setup, Extension
from commands import jep_build
from commands.clean import really_clean

from commands.dist import JepDistribution
from commands.install import post_install
from commands.java import build_java, build_javah, get_java_home, get_java_include,\
    get_java_lib, get_java_linker_args, build_jar, get_java_lib_folders, get_java_libraries
from commands.python import get_python_libs, get_python_linker_args
from commands.scripts import build_scripts
from commands.test import test

VERSION = None # shut up pycharm
execfile('jep/version.py')

def get_files(pattern):
    ret = []
    for root, dirs, files in os.walk('src'):
        for file in files:
            if file.endswith(pattern):
                ret.append(os.path.join(root, file))
    return ret

def read_file(name):
    return codecs.open(os.path.join(os.path.dirname(__file__), name), encoding='utf-8').read()

if __name__ == '__main__':
    get_java_home()

    setup(name='jep',
          version=VERSION,
          description='Jep embeds CPython in Java',
          long_description=read_file('README.rst'),
          author='Mike Johnson',
          author_email='mike@mrj0.com',
          url='http://www.mrj0.com/projects/jep/',
          packages=['jep'],
          scripts=['src/scripts/jep'],
          keywords='java',
          license='zlib/libpng',
          classifiers=['License :: OSI Approved :: zlib/libpng License'],
          ext_modules=[
              Extension(
                  name='jep',
                  sources=get_files('.c'),
                  define_macros=[
                      ('PACKAGE', 'jep'),
                      ('USE_DEALLOC', 1),
                     # ('USE_MAPPED_EXCEPTIONS', 1),
                      ('VERSION', '"{0}"'.format(VERSION)),
                  ],
                  libraries=get_java_libraries() + get_python_libs(),
                  library_dirs=get_java_lib_folders(),
                  extra_link_args=get_java_linker_args() + get_python_linker_args(),
                  include_dirs=get_java_include() + ['src/jep', 'build/include'],
              )
          ],

          # my hacks to compile java files
          java_files=get_files('.java'),
          javah_files=[ # tuple containing class and the header file to output
              ('jep.Jep', 'jep.h'),
	          ('jep.python.PyObject', 'jep_object.h'),
	          ('jep.InvocationHandler', 'invocationhandler.h'),
          ],
          distclass=JepDistribution,
          cmdclass={
              'build_java': build_java,
              'build_javah': build_javah,
              'build_jar': build_jar,
              'build': jep_build,
              'build_scripts': build_scripts,
              'install_data': post_install,
              'clean': really_clean,
              'test': test,
          },

          # a list of other files to install
          data_files=[
              ('lib/jep', ['build/java/jep.jar']),
          ]
    )

