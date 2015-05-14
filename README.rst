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

License
-------
Jep is licensed **zlib/libpng** license to avoid linking issues.

Installation
------------
Simply run ``pip install jep``.

Dependencies
------------
* Python >= 2.6
* Java >= 1.6
* Numpy (optional) >= 1.5 (numpy >= 1.7 recommended) 

Building
--------
Simply run ``python setup.py build``. Building requires the JDK, Python, and
optionally numpy to already be installed.  The build will produce the jep jar,
a test jar, a src jar, and the native library compiled for your platform.

*Build support*

Due to the variety of configurations of operating systems, Java versions,
Python versions, and Numpy versions, we cannot test all configurations. The
build has been tested on Linux, Windows, and OS X.

*OS X*

The OS X build requires Xcode.  In recent versions of OS X, running the build
will automatically prompt you to download Xcode if it is not found.

*Windows*

CPython extensions generally need to be built with the same compiler that
built Python. That's usually MSVC.  MSVC needs to be installed prior to
building Jep, or you can attempt your own build with MinGW.

Installing from a build
-----------------------
Simply run ``python setup.py install`` after the build has successfully
completed.  The install will move the shared library to the appropriate
location for Python libraries and create a jep script.

Running the tests
-----------------
Simply run ``python setup.py test``.

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

Running on \*nix
-----------------
Due to some (common) difficulties with Java and C projects
that dlopen libraries, you may need to set LD_PRELOAD environment
variable. That's in addition to setting LD_LIBRARY_PATH if you've
installed libjep into a directory not cached by ld.so.

See the contents of the installed ``jep`` script for an example how to do this.
The script should have the correct values for your interpreter and virtualenv
(if present).


Common Errors
-------------
If you see Unsatisfied Link Errors, that implies Java cannot find the shared
library that you built for your platform.  You can fix this by setting
LD_LIBRARY_PATH or the JVM argument java.library.path.

If you see fatal python errors when first using Jep, that implies
the PATH, LD_PRELOAD, or LD_LIBRARY_PATH environment variables are incorrect or
inconsistent.  This is often seen if multiple versions of python are installed
on a system.


Support
-------
For issues and source control, use github:

https://github.com/mrj0/jep/

There's also a Sourceforge mailing list that is the best way to get support
for Jep:

https://sourceforge.net/mail/?group_id=109839

Note the project page and mailing list will be moving in the future.  A wiki
will be forthcoming.  We will be consolidating as much as possible on github.

Contributions
-------------
We welcome comments, contributions, bug reports, wiki documentation, etc.
Please use the mailing list and/or github's tools.


*Jep Team*
