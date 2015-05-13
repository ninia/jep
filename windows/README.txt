This directory is only for building the Jep native library inside of Visual
Studio.  Since setup.py now supports Windows, in general you should strive to
use setup.py to build jep.dll instead of this directory.

This directory is useful for diagnosing build issues, as
it can be quite helpful to see the commands and arguments Visual Studio uses
when compiling and compare those to the commands and arguments that distutils
uses when compiling.  See
http://stackoverflow.com/questions/1211841/how-can-i-make-visual-studios-build-be-very-verbose