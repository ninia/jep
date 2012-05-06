Jep - Java Embedded Python
===========================

Jep embeds CPython in Java. It is safe to use in a heavily threaded
environment, it is quite fast and its stability is a main feature and
goal.

Some benefits of CPython over Java-based languages:

* Using the native Python interpreter may be much faster.

* Python is mature and well supported, so there's no fear the
  interpreter will suddenly change widely-used features.

* Access to the high quality Python modules, both native and
  Python-based.

* Compilers and assorted Python tools are as mature as the language.

* Python is an ideal language for your business logic. It is easy to
  learn, readable and generally immune to programming gotchas.

Patches, comments and other help is greatly appreciated. If you need
help, post to the `SourceForge mailing list <http://sourceforge.net/mailarchive/forum.php?forum_name=jepp-users>`_
or forums. Please include code snippets for the most accurate
response.

Jep is licensed zlib/libpng license to avoid linking issues.

Installation
------------

Simply run ``pip install jep``.

Dependencies
------------
* Python version >= 2.6
* JNI >= 1.4

*Building on Mac OS X*

OS X requires the `Java Developer Package and Xcode
<http://developer.apple.com/java/>`_ from Apple. They are free to download.

*Windows*

You'll need to use the same compiler that your Python is built
with. That's usually MSVC.

Note that Oracle is now building Java with a MSVCRT version that is
not easily linked with using tools that I have. Using native modules
on Windows has not worked in recent years because the compilers are
not widely available. If an OpenJDK build used MinGW, that'd be
much more likely to work.

Running scripts
---------------

The ``setup.py`` script will provide a ``jep`` shortcut to make launching Java and Python easier.

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


Running the tests
-----------------

The tests are run from setup.py:

::

    $ python setup.py test

Support
-------

For issues and source control, use github:

https://github.com/mrj0/jep/

There's also a Sourceforge mailing list that is the best way to get support for Jep:

https://sourceforge.net/mail/?group_id=109839


Mike Johnson
