#!/usr/bin/env python
import codecs

import os

import sysconfig

from setuptools import setup, Extension

from commands import jep_build
from commands.clean import really_clean

from commands.dist import JepDistribution
from commands.install_lib import jep_install
from commands.java import build_java, get_java_home, get_java_include,\
    get_java_linker_args, build_jar, get_java_lib_folders, get_java_libraries, setup_java
from commands.javadoc import javadoc
from commands.python import get_libpython, get_python_libs, get_python_linker_args
from commands.scripts import build_scripts
from commands.test import test
from commands.util import is_windows
from commands.build_ext import build_ext

VERSION = None  # shut up pycharm
with open('src/main/python/jep/version.py') as f:
   exec(f.read())

numpy_include = []
numpy_found = 0
try:
   import numpy
   numpy1_include_path = os.path.join(numpy.__path__[0], 'core', 'include')
   numpy2_include_path = os.path.join(numpy.__path__[0], '_core', 'include')
   if os.path.exists(numpy1_include_path):
      print('numpy include found at', numpy1_include_path)
      numpy_found = 1
      numpy_include = [numpy1_include_path]
   elif os.path.exists(numpy2_include_path):
      print('numpy include found at', numpy2_include_path)
      numpy_found = 1
      numpy_include = [numpy2_include_path]
   else:
      print('numpy include not found')
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
    ldlib = get_libpython()
    if ldlib:
        # a libpython was found, so use the basename of the discovered path
        ldlib = os.path.basename(ldlib)
    else:
        # no libpython was found, so use LDLIBRARY blindly
        ldlib = sysconfig.get_config_var('LDLIBRARY')
    if ldlib:
        defines.append(('PYTHON_LDLIBRARY', '"' + ldlib + '"'))
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
          url='https://github.com/ninia/jep',
          packages=['jep'],
          package_dir={'': 'src/main/python'},
          scripts=['src/main/scripts/jep'],
          keywords='java',
          license='zlib/libpng',
          classifiers=[
                       'License :: OSI Approved :: zlib/libpng License',
                       'Development Status :: 5 - Production/Stable',
                       'Intended Audience :: Developers',
                       'Topic :: Software Development',
                       'Programming Language :: Java',
                       'Programming Language :: Python',
                       'Programming Language :: Python :: 3.10',
                       'Programming Language :: Python :: 3.11',
                       'Programming Language :: Python :: 3.12',
                       'Programming Language :: Python :: 3.13',
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
                  include_dirs=get_java_include() + ['src/main/c/Include', 'build/include',] + numpy_include,
              )
          ],

          # my hacks to compile java files
          java_files=get_files('.java'),
          extra_jar_files=['src/main/resources/jep/classlist_8.txt',
                           'src/main/resources/jep/classlist_9.txt',
                           'src/main/resources/jep/classlist_10.txt',
                           'src/main/resources/jep/classlist_11.txt'],
          javah_files=['jep.Jep',
                       'jep.MainInterpreter',
                       'jep.python.InvocationHandler',
                       'jep.python.PyObject',
                       'jep.python.PyCallable',
                       'jep.python.PyPointer'],
          distclass=JepDistribution,
          cmdclass={
              'setup_java': setup_java,
              'build_java': build_java,
              'javadoc': javadoc,
              'build_jar': build_jar,
              'build': jep_build,
              'build_ext' : build_ext,
              'build_scripts': build_scripts,
              'install_lib': jep_install,
              'clean': really_clean,
              'test': test,
          },
          zip_safe=False
    )

