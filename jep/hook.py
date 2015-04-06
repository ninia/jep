from _jep import forName
import sys
from types import ModuleType


class module(ModuleType):
    """Lazy load classes not found at runtime.

    Introspecting Java packages is difficult, there is not a good
    way to get a list of all classes for a package. By providing
    a __getattr__ implementation for modules, this class can
    try to find classes manually.

    Due to this Java limitation, some classes will not appear in dir()
    but will import correctly.
    """

    def __getattr__(self, name):
        try:
            return super(module, self).__getattribute__(name)
        except AttributeError as ae:
            try:
                clazz = forName('{0}.{1}'.format(self.__name__, name))
                setattr(self, name, clazz)
                return clazz
            except Exception:
                # should raise AttributeError, not JepException
                raise ae


class JepImporter(object):
    def __init__(self, classlist=None):
        if classlist:
            self.classlist = classlist
        else:
            self.classlist = forName('jep.ClassList').getInstance()

    def find_module(self, fullname, path=None):
        if self.classlist.contains(fullname):
            return self  # found a java package with this name
        return None

    def load_module(self, fullname):
        if fullname in sys.modules:
            return sys.modules[fullname]

        mod = module(fullname)
        mod.__dict__.update({
            '__loader__': self,
            '__path__': [],
            '__file__': '<java>',
        })
        sys.modules[fullname] = mod

        if self.classlist.supportsPackageImport():
            # list of classes in package
            classlist = self.classlist.get(fullname)
            if classlist:
                for name in classlist:
                    try:
                        setattr(mod, name.split('.')[-1], forName(name))
                    except Exception:
                        pass
        return mod


def setupImporter(classlist):
    sys.meta_path = [importer for importer in sys.meta_path if isinstance(importer, JepImporter)]
    sys.meta_path.append(JepImporter(classlist))

