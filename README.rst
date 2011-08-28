Jep - Java Embedded Python
===========================

Jep embeds CPython in Java. It is safe to use in a heavily threaded
environment, it is quite fast and its stability is a main feature and
goal.

Some benefits of CPython over Java-based languages:

* Using the native Python interpreter may mean a massive speed improvement over Java-based languages.
* Python is mature so authors needn't fear the interpreter will suddenly change widely-used features.
* Access to the high quality Python modules, both native and Python-based.
* Compilers and assorted Python tools are as mature as the language.
* Python is an ideal language for your business logic. It is easy to learn, eminently readable and generally immune to programming gotchas.

Patches, comments and other help is greatly appreciated. If you need
help, post to the SourceForge mailing list or forums. Please include
code snippets for the most accurate response.

Jep is licensed zlib/libpng license to avoid linking issues.

Dependencies
------------
* Python version >= 2.6
* JNI >= 1.2 (untested, preferably 1.4)

Building on Linux/UNIX
----------------------
* Simply ./configure && make && make install.

Building on Mac OS X
--------------------
You'll need to:

* set JAVA_HOME to /Library/Java/Home *not* /usr.
* have installed the developer kits from Apple
* make a symlink from libjep.dynlib to /Library/Java/Extentions, like:

``sudo ln -s `pwd`/.libs/libjep.dylib /Library/Java/Extensions/libjep.jnilib``

*** Make sure to use '.jnilib' for the symlink. ***

Windows
-------
You'll need to use the same compiler that your Python is built with. That's usually MSVC. I've successfully used the ActiveState version.

There are project files for MSVC but you must manually update the paths to your Java and Python installations.

Running on *Nix
---------------
Due to some (common) difficulties with Java and C projects
that dlopen libraries, you'll need to set LD_PRELOAD environment
variable. That's in addition to setting LD_LIBRARY_PATH if you've
installed libjep into a directory not cached by ld.so.

For example, my tomcat startup.sh script starts with this:

::

    #!/bin/sh
    # force system to load python
    export LD_PRELOAD=/usr/lib/libpython2.7.so
    
    # this is where my libjep.so is.
    export LD_LIBRARY_PATH=/usr/local/lib

Adding some heap memory is a good idea, too.

The libpython used here is whatever you've compiled jep against. If
you don't know, try this command:

::

    $ ldd /usr/local/lib/libjep.so | grep python
        /usr/lib/libpython2.7.so (0x00007f74adfbd000)

That's the libpython you want to set in LD_PRELOAD. Unfortunately,
this means you'll have to change this if you upgrade python. If it's
the wrong library, it will most likely cause a crash. If it's not set
you'll get an exception similar to: `Python Encountered:
exceptions.ImportError: /usr/lib/python2.7/lib-dynload/datetime.so:
undefined symbol: PyObject_GenericGetAttr`.

Running
-------

First, fire off the test.py script. We want to make sure you're fully
setup. To start with, export LD_LIBRARY_PATH and LD_PRELOAD as in the
section above. Then, start with:

::

    $ java -cp jep.jar jep.Test 0

A lot of early bugs in Jep didn't appear until the code is stressed a
little. The above 0 argument is the number of additional threads to
create. They all run through the test.py script in sub intepreters. Go
ahead, throw some threads at it.

::

    $ java -cp jep.jar jep.Test 30

Also, try the console script:

::

    $ java -jar jep.jar console.py
    >>> from java.util import HashMap
    >>> map = HashMap()
    >>> map.put('test', 'asdf')

You can run arbitrary scripts, too:

::

    $ cat hello.py 
    print 'Hello, world'
    $ java -jar jep.jar hello.py 
    Hello, world

Support
-------

For issues and source control, use github:

https://github.com/mrj0/jep/

There's also a Sourceforge mailing list that is the best way to get support for Jep:

https://sourceforge.net/mail/?group_id=109839


Mike Johnson
