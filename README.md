Jep - Java Embedded Python
===========================

[![Python: 3.5, 3.6, 3.7, 3.8, 3.9](https://img.shields.io/pypi/pyversions/Jep.svg)](https://pypi.python.org/pypi/jep)
[![License: zlib/libpng](https://img.shields.io/pypi/l/Jep.svg)](https://pypi.python.org/pypi/jep)
[![Pypi: v4.0.0](https://img.shields.io/pypi/v/Jep.svg)](https://pypie.python.org/pypi/jep)
[![Docs: wiki](https://img.shields.io/badge/docs-wiki-orange.svg)](https://github.com/ninia/jep/wiki)
[![Docs: JavaDoc](https://img.shields.io/badge/docs-javadoc-orange.svg)](javadoc)

Jep embeds CPython in Java through JNI.

Some benefits of embedding CPython in a JVM:

* Using the native Python interpreter may be much faster than
  alternatives.

* Python is mature, well supported, and well documented.

* Access to high quality Python modules, both native CPython
  extensions and Python-based.

* Compilers and assorted Python tools are as mature as the language.

* Python is an interpreted language, enabling scripting of established
  Java code without requiring recompilation.

* Both Java and Python are cross platform, enabling deployment to 
  different operating systems.


Installation
------------
Simply run ``pip install jep`` or download the source and run ``python setup.py build install``.
Building and installing require the JDK, Python, and optionally numpy to be installed beforehand.

Dependencies
------------
* Python 3.5, 3.6, 3.7, 3.8 or 3.9
* Java >= 1.8
* NumPy >= 1.7 (optional)

Notable features
----------------
* Interactive Jep console much like Python's interactive console
* Supports multiple, simultaneous, mostly sandboxed sub-interpreters or shared interpreters
* Numpy support for Java primitive arrays

Help
----
* [Documentation](https://github.com/ninia/jep/wiki)
* [JavaDoc](javadoc)
* [Known Issues](https://github.com/ninia/jep/issues)
* [Project Page](https://github.com/ninia/jep)

We welcome comments, contributions, bug reports, wiki documentation, etc.

*Jep Team*
