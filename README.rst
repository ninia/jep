.. image:: https://img.shields.io/pypi/pyversions/Jep.svg
    :target: https://pypi.python.org/pypi/jep

.. image:: https://img.shields.io/pypi/l/Jep.svg
    :target: https://pypi.python.org/pypi/jep

.. image:: https://img.shields.io/pypi/v/Jep.svg
    :target: https://pypi.python.org/pypi/jep
	
.. image:: https://img.shields.io/badge/docs-wiki-orange.svg
    :target: https://github.com/ninia/jep/wiki

.. image:: https://img.shields.io/badge/docs-javadoc-orange.svg
    :target: https://ninia.github.io/jep/javadoc

Jep - Java Embedded Python
===========================

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
Simply run ``pip install jep`` or download the source and run ``pip install .``.
Building and installing require the JDK, Python, and optionally numpy to be installed beforehand.

Dependencies
------------
* Python >= 3.5
* Java >= 1.8
* NumPy >= 1.7 (optional)

Notable features
----------------
* Interactive Jep console much like Python's interactive console
* Supports multiple, simultaneous, mostly sandboxed sub-interpreters or shared interpreters
* Numpy support for Java primitive arrays

Help
----
* `Documentation <https://github.com/ninia/jep/wiki>`_
* `JavaDoc <https://ninia.github.io/jep/javadoc>`_
* `Mailing List (deprecated) <https://groups.google.com/d/forum/jep-project>`_
* `Known Issues and Help <https://github.com/ninia/jep/issues>`_
* `Contribution Guidelines <https://github.com/ninia/jep/blob/master/.github/CONTRIBUTING.md>`_
* `Project Page <https://github.com/ninia/jep>`_

We welcome comments, contributions, bug reports, wiki documentation, etc.

If you need help, please first search for existing solutions online, in the
the issues, and on the wiki. If you still need help, please open a GitHub issue
and we will try and help you. Please remember to close the issue once it has
been resolved.


*Jep Team*
