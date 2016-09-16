.. image:: https://img.shields.io/pypi/pyversions/Jep.svg
    :target: https://pypi.python.org/pypi/jep

.. image:: https://img.shields.io/pypi/l/Jep.svg
    :target: https://pypi.python.org/pypi/jep

.. image:: https://img.shields.io/pypi/v/Jep.svg
    :target: https://pypie.python.org/pypi/jep
	
.. image:: https://img.shields.io/badge/docs-wiki-orange.svg
    :target: https://github.com/mrj0/jep/wiki


Jep - Java Embedded Python
===========================

Jep embeds CPython in Java through JNI and is safe to use in a heavily
threaded environment. 

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
* Python 2.6, 2.7, 3.2, 3.3, 3.4, or 3.5
* Java >= 1.6
* Numpy (optional) >= 1.5 (numpy >= 1.7 recommended)

Notable features
----------------
* Interactive Jep console much like Python's interactive console
* Supports multiple, simultaneous, mostly sandboxed sub-interpreters
* Numpy support for Java primitive arrays

Help
----
* Documentation: https://github.com/mrj0/jep/wiki
* Mailing List: https://groups.google.com/d/forum/jep-project
* Known Issues: https://github.com/mrj0/jep/issues
* Project Page: https://github.com/mrj0/jep

We welcome comments, contributions, bug reports, wiki documentation, etc.

*Jep Team*
