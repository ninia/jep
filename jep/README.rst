	jepp - Java Embedded Python



   Copyright (c) 2004, 2005, 2006 Mike Johnson.

   This file is licenced under the the zlib/libpng License.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.
   
   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:
   
       1. The origin of this software must not be misrepresented; you
       must not claim that you wrote the original software. If you use
       this software in a product, an acknowledgment in the product
       documentation would be appreciated but is not required.
   
       2. Altered source versions must be plainly marked as such, and
       must not be misrepresented as being the original software.
   
       3. This notice may not be removed or altered from any source
       distribution.

Comments welcome.

	- Mike Johnson <mrjohnson0@users.sourceforge.net>


    DEPENDENCIES
        - Python version >= 2.2
        - JNI >= 1.2 (untested, preferably 1.4)


    BUILDING ON LINUX/UNIX
        Simply ./configure && make && make install.


    BUILDING ON MAC OS X
        You'll need to:
            - set JAVA_HOME to /Library/Java/Home *not* /usr.
            - compile Python or download it using Fink.
            - have installed the developer kits from Apple
            - make a symlink from libjep.dynlib to /Library/Java/Extentions, like:
                sudo ln -s `pwd`/.libs/libjep.dylib \
                    /Library/Java/Extensions/libjep.jnilib

                *** Make sure to use '.jnilib' for the symlink. ***

              (This will get added to the install script Very Soon Now.)


    AUTOMAKE 1.6
        If you're using automake > 1.4, please run the `autogen.sh`
script before running configure. This includes Mac OS X.

    
    WINDOWS
        You'll need MSVC 6 and Python 2.3. I've compiled successfully
against the ActiveState version. The project file expects Java in
C:\j2sdk1.4.2_04 and Python in C:\Python2.3. If these aren't correct
paths, you should edit the project settings.
        Otherwise, the build is pretty normal. Notice there's no debug
version. Feel free to add one if you like.
        Also, you'll likely need to add -Djava.library.path='path to
Release folder' for testing. Or copy and register the DLL in the
System directory.


    RUNNING ON *NIX - Preparing the libraries

        Due to some (common) difficulties with Java and C projects
that dlopen libraries, you'll need to set LD_PRELOAD environment
variable. That's in addition to setting LD_LIBRARY_PATH if you've
installed libjep into a directory not cached by ld.so.
        For example, my tomcat startup.sh script starts with this:

            #!/bin/sh
            # java memory setting
            export JAVA_OPTS='-Xmx64m'

            # force system to load python
            export LD_PRELOAD=/usr/lib/libpython2.3.so.1.0

            # this is where my libjep.so is.
            export LD_LIBRARY_PATH=/share/jepp/jep/.libs

        Adding some heap memory is a good idea, too.
        The libpython used here is whatever you've compiled jep
against. If you don't know, try this command:

            $ ldd .libs/libjep.so.1.0.0
                < ... blah ... >
            	libpython2.3.so.1.0 => /usr/lib/libpython2.3.so.1.0 (0x40023000)

        That's the libpython you want to set in
LD_PRELOAD. Unfortunately, this means you'll have to change this if
you upgrade python. If it's the wrong library, it will most likely
cause a crash. If it's not set you'll get an exception similar to:
"Python Encountered: exceptions.ImportError:
/usr/lib/python2.3/lib-dynload/datetime.so: undefined symbol:
PyObject_GenericGetAttr".


    RUNNING - Down to business

        First, fire off the test.py script. We want to make sure
you're fully setup. To start with, export LD_LIBRARY_PATH and
LD_PRELOAD as in the section above. Then, start with:

            java -classpath ../ jep.Test 0

        A lot of early bugs in Jep didn't appear until the code is
stressed a little. The above 0 argument is the number of additional
threads to create. They all run through the test.py script in sub
intepreters. Go ahead, throw some threads at it.

            $ time java -classpath ../ jep.Test 3
                real	0m0.344s
                user	0m0.260s
                sys	0m0.050s

            $ time java -classpath ../ jep.Test 50
                real	0m1.944s
                user	0m1.850s
                sys	0m0.100s

        The test script isn't designed for speed but it runs quickly
enough.
