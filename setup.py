#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import codecs

import os

from distutils.core import setup, Extension
# if you want to build wheels, use setuptools instead of distutils
# otherwise stick with distutils to avoid extra dependencies
#from setuptools import setup, Extension

from commands import jep_build
from commands.clean import really_clean

from commands.dist import JepDistribution
from commands.install_lib import jep_install
from commands.java import build_java, build_javah, get_java_home, get_java_include,\
    get_java_linker_args, build_jar, get_java_lib_folders, get_java_libraries, setup_java
from commands.python import get_python_libs, get_python_linker_args
from commands.scripts import build_scripts
from commands.test import test
from commands.util import is_windows
from commands.build_ext import build_ext

VERSION = None  # shut up pycharm
with open('jep/version.py') as f:
   exec(f.read())

numpy_include = []
numpy_found = 0
try:
   import numpy
   include_path = os.path.join(numpy.__path__[0], 'core', 'include')
   if os.path.exists(include_path):
      print('numpy include found at', include_path)
      numpy_found = 1
      numpy_include = [include_path]
except ImportError:
   print('numpy not found, running without numpy support')


def get_files(pattern):
    ret = []
    for root, dirs, files in os.walk('src'):
        for f in files:
            if f.endswith(pattern):
                ret.append(os.path.join(root, f))
    return ret


def read_file(name):
    return codecs.open(os.path.join(os.path.dirname(__file__), name), encoding='utf-8').read()


if __name__ == '__main__':
    get_java_home()

    defines=[
              ('PACKAGE', 'jep'),
              ('USE_DEALLOC', 1),
              ('JEP_NUMPY_ENABLED', numpy_found),
              ('VERSION', '"{0}"'.format(VERSION)),
          ]
    if is_windows():
        defines.append(('WIN32', 1))
        #Disable warnings about Secure CRT Functions in util.c and pyembed.c.
        defines.append(('_CRT_SECURE_NO_WARNINGS', 1))

    setup(name='jep',
          version=VERSION,
          description='Jep embeds CPython in Java',
          long_description=read_file('README.rst'),
          author='Jep Developers',
          author_email='jep-project@googlegroups.com',
          url='https://github.com/mrj0/jep',
          packages=['jep'],
          scripts=['src/scripts/jep'],
          keywords='java',
          license='zlib/libpng',
          classifiers=[
                       'License :: OSI Approved :: zlib/libpng License',
                       'Development Status :: 5 - Production/Stable',
                       'Intended Audience :: Developers',
                       'Topic :: Software Development',
                       'Programming Language :: Java',
                       'Programming Language :: Python',
                       'Programming Language :: Python :: 2',
                       'Programming Language :: Python :: 2.6',
                       'Programming Language :: Python :: 2.7',
                       'Programming Language :: Python :: 3',
                       'Programming Language :: Python :: 3.2',
                       'Programming Language :: Python :: 3.3',
                       'Programming Language :: Python :: 3.4',
                       'Programming Language :: Python :: 3.5',
                       'Programming Language :: Python :: Implementation :: CPython',
                      ],
          ext_modules=[
              Extension(
                  name='jep',
                  sources=get_files('.c'),
                  define_macros=defines,
                  libraries=get_java_libraries() + get_python_libs(),
                  library_dirs=get_java_lib_folders(),
                  extra_link_args=get_java_linker_args() + get_python_linker_args(),
                  include_dirs=get_java_include() + ['src/jep', 'build/include'] + numpy_include,
              )
          ],

          # my hacks to compile java files
          java_files=get_files('.java'),
          extra_jar_files=['src/jep/classlist_6.txt',
                           'src/jep/classlist_7.txt',
                           'src/jep/classlist_8.txt'],
          javah_files=[   # tuple containing class and the header file to output
              ('jep.Jep', 'jep.h'),
              ('jep.python.PyObject', 'jep_object.h'),
              ('jep.InvocationHandler', 'invocationhandler.h'),
          ],
          distclass=JepDistribution,
          cmdclass={
              'setup_java': setup_java,
              'build_java': build_java,
              'build_javah': build_javah,
              'build_jar': build_jar,
              'build': jep_build,
              'build_ext' : build_ext,
              'build_scripts': build_scripts,
              'install_lib': jep_install,
              'clean': really_clean,
              'test': test,
          },
    )

