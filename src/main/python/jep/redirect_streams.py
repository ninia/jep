#
# Copyright (c) 2016-2018 JEP AUTHORS.
#
# This file is licensed under the the zlib/libpng License.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any
# damages arising from the use of this software.
# 
# Permission is granted to anyone to use this software for any
# purpose, including commercial applications, and to alter it and
# redistribute it freely, subject to the following restrictions:
# 
#     1. The origin of this software must not be misrepresented; you
#     must not claim that you wrote the original software. If you use
#     this software in a product, an acknowledgment in the product
#     documentation would be appreciated but is not required.
# 
#     2. Altered source versions must be plainly marked as such, and
#     must not be misrepresented as being the original software.
# 
#     3. This notice may not be removed or altered from any source
#     distribution.
#

import sys

class StdOutToJava(object):
    "Redirects Python's sys.stdout to Java's System.out"

    def __init__(self):
        from java.lang import System
        self.oldout = sys.stdout
        self.printmethod = getattr(System.out, 'print')
        self.flushmethod = getattr(System.out, 'flush')

    def write(self, msg):
        self.printmethod(msg)

    def flush(self):
        self.flushmethod()


class StdErrToJava(object):
    "Redirects Python's sys.stderr to Java's System.err"

    def __init__(self):
        from java.lang import System
        self.olderr = sys.stderr
        self.printmethod = getattr(System.err, 'print')
        self.flushmethod = getattr(System.err, 'flush')

    def write(self, msg):
        self.printmethod(msg)

    def flush(self):
        self.flushmethod()


def setup():
    sys.stdout = StdOutToJava()
    sys.stderr = StdErrToJava()
