.. image:: https://img.shields.io/pypi/pyversions/Jep.svg
    :target: https://pypi.python.org/pypi/jep

.. image:: https://img.shields.io/pypi/l/Jep.svg
    :target: https://pypi.python.org/pypi/jep

.. image:: https://img.shields.io/pypi/v/Jep.svg
    :target: https://pypie.python.org/pypi/jep

.. image:: https://img.shields.io/pypi/dm/Jep.svg
    :target: https://pypi.python.org/pypi/jep
	
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

* Python is an interpreted language, enabling runtime customizations
  and plugins that do not require a compilation step.

* Both Java and Python are cross platform, enabling deployment to 
  different operating systems.


Installation
------------
Simply run ``pip install jep`` or download the source and run ``python setup.py build install``.
Building and installing requires that the JDK, Python, and optionally numpy are already installed.

Dependencies
------------
* Python >= 2.6
* Java >= 1.6
* Numpy (optional) >= 1.5 (numpy >= 1.7 recommended)

Running the jep script
----------------------
The ``setup.py`` script will provide a ``jep`` or ``jep.bat`` script to make
launching Jep easier.  The jep script is very similar to running python from
a terminal/command line.  If run with an argument of a file path, it will run
the script at that path.  If run with no arguments, it will provide an
interactive console that combines the Python language with access to Java
classes on the classpath.

::

    $ jep
    >>> from java.lang import System
    >>> System.out.println('hello, world')
    hello, world
    >>>


Help
----
* Documentation: https://github.com/mrj0/jep/wiki
* Mailing List: https://sourceforge.net/mail/?group_id=109839
* Known Issues: https://github.com/mrj0/jep/issues
* Project Page: https://github.com/mrj0/jep

We welcome comments, contributions, bug reports, wiki documentation, etc.

*Jep Team*