try:
    from _jep import *
except ImportError:
    raise ImportError("Jep is not supported in standalone Python, it must be embedded in Java.")
from .version import __VERSION__, VERSION
from .java_import_hook import *
from .shared_modules_hook import *
