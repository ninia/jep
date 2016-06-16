from __future__ import print_function

import sys
print("Py_NoSiteFlag", sys.flags.no_site)
print("Py_IgnoreEnvironmentFlag", sys.flags.ignore_environment)
print("Py_NoUserSiteDirectory", sys.flags.no_user_site)
print("Py_VerboseFlag", sys.flags.verbose)
print("Py_OptimizeFlag", sys.flags.optimize)
print("Py_DontWriteBytecodeFlag", sys.flags.dont_write_bytecode)
print("Py_HashRandomizationFlag", sys.flags.hash_randomization)