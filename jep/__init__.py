from version import __VERSION__, VERSION
from _jep import *
from hook import *

class JavaException(Exception):
    def __init__(self, er):
        # error is "classname: message"
        clazz, message = er.split(':', 1)
        super(JavaException, self).__init__(message)
        # the java class name of exception
        self.java_name = clazz
