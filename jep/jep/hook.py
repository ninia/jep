from _jep import *
import sys
import imp


class JepImporter(object):
    def __init__(self):
        self.classlist = findClass('jep.ClassList').getInstance()

    def find_module(self, fullname, path=None):
        if self.classlist.get(fullname) is not None:
            return self # found a java package with this name
        return None

    def load_module(self, fullname):
        if fullname in sys.modules:
            return fullname

        mod = imp.new_module(fullname)
        mod.__loader__ = self
        sys.modules[fullname] = mod
        mod.__path__ = []
        mod.__file__ = '<java>'

        # list of classes in package
        for name in self.classlist.get(fullname):
            setattr(mod, name.split('.')[-1], findClass(name))
        return mod

sys.meta_path = [importer for importer in sys.meta_path if isinstance(importer, JepImporter)]
sys.meta_path.append(JepImporter())

