#!/usr/bin/env python
from __future__ import print_function

import os
from distutils.core import setup, Extension
from commands import jep_build

from commands.dist import JepDistribution
from commands.java import build_java, build_javah, get_java_home, get_java_include

VERSION = '3.0'

def get_files(pattern):
    ret = []
    for root, dirs, files in os.walk('src'):
        for file in files:
            if file.endswith(pattern):
                ret.append(os.path.join(root, file))
    return ret

if __name__ == '__main__':
    get_java_home()

    setup(name='Jep',
          version=VERSION,
          description='Jep embeds CPython in Java',
          author='Mike Johnson',
          author_email='mike@publicstatic.net',
          url='http://www.publicstatic.net/jep/',
          scripts=['_jep.py'],
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
                      ('USE_MAPPED_EXCEPTIONS', 1),
                      ('VERSION', '"{0}"'.format(VERSION)),
                  ],
                  # libraries=['jvm'],
                  # library_dirs=[get_java_lib()],
                  include_dirs=[get_java_include(), 'src/jep'],
              )
          ],

          # my hacks to compile java files
          java_files=get_files('.java'),
          javah_files=[ # tuple containing class and the header file to output
              ('jep.Jep', 'src/jep/jep.h'),
	          ('jep.python.PyObject', 'src/jep/python/jep_object.h'),
	          ('jep.InvocationHandler', 'src/jep/invocationhandler.h'),
          ],
          distclass=JepDistribution,
          cmdclass={
              'build_java': build_java,
              'build_javah': build_javah,
              'build': jep_build,
          }
    )

