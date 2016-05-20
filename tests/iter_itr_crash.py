# There was a bug where if you called iter(PyJIterator) the object would
# be garbage collected early and the memory would become corrupted.  This
# tests the fix for that problem.

from __future__ import print_function
from java.util import ArrayList

itr = ArrayList().iterator()
iter(itr)
ArrayList()
iter(itr) # this line could crash before fix
print("success: no crash")
