# There was a bug where if you called iter(PyJIterator) the object would
# be garbage collected early and the memory would become corrupted.  This
# tests the fix for that problem.

import sys
from java.util import ArrayList

itr = ArrayList().iterator()
iter(itr)
ArrayList()
itr2 = iter(itr)  # this line could crash before fix
sys.exit(0)
