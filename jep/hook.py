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
            clazz = forName('{0}.{1}'.format(self.__name__, name))
            setattr(self, name, clazz)
            return clazz


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

        split = fullname.split('.')
        if split[-1][0].islower():
            # it's a package/module
            mod = module(fullname)
            mod.__dict__.update({
                '__loader__': self,
                '__path__': [],
                '__file__': '<java>',
                '__package__': None,
            })
            sys.modules[fullname] = mod

            if self.classlist.supportsPackageImport():
                # get the list of classes in package and add them as attributes
                # to the module
                classlist = self.classlist.getClassNames(fullname)
                if classlist:
                    for name in classlist:
                        try:
                            setattr(mod, name.split('.')[-1], forName(name))
                        except Exception:
                            pass
        else:
            # It's a Java class, in general we will only reach here if
            # self.classlist.supportsPackageImport() is False (ie the class
            # has not already been imported and set on the module).
            parentModName = '.'.join(split[0:-1])
            parentMod = sys.modules[parentModName]
            return parentMod.__getattr__(split[-1])
        return mod


def setupImporter(classlist):
    alreadySetup = False
    for importer in sys.meta_path:
        if isinstance(importer, JepImporter):
            alreadySetup = True
            break
    if not alreadySetup:
        sys.meta_path.append(JepImporter(classlist))
