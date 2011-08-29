__VERSION__ = '3.0.dev1'
VERSION = __VERSION__
try:
    from _jep import *
    from hook import *
except:
    pass

class JavaException(Exception):
    def __init__(self, er):
        # error is "classname: message"
        clazz, message = er.split(':', 1)
        super(JavaException, self).__init__(message)
        # the java class name of exception
        self.java_name = clazz
