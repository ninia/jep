from _jep import forName
import sys
from types import ModuleType


class module(ModuleType):
    """Lazy load classes not found at runtime.

    Introspecting Java packages is difficult, there is not a good
    way to get a list of all classes for a package. By providing
    a __getattr__ implementation for modules, this class can
    try to find classes manually.

    Based on the ClassEnquirer used, some classes may not appear in dir()
    but will import correctly.
    """

    def __getattr__(self, name):
        try:
            return super(module, self).__getattribute__(name)
        except AttributeError as ae:
            subpkgs = self.__classEnquirer__.getSubPackages(self.__name__)
            if subpkgs and name in subpkgs:
                fullname = self.__name__ + '.' + name
                mod = makeModule(fullname, self.__loader__,
                                 self.__classEnquirer__)
                return mod
            elif name == '__all__':
                return self.__dir__()
            else:
                # assume it is a class and attempt the import
                clazz = forName('{0}.{1}'.format(self.__name__, name))
                setattr(self, name, clazz)
                return clazz

    def __dir__(self):
        result = []
        subpkgs = self.__classEnquirer__.getSubPackages(self.__name__)
        if subpkgs:
            for s in subpkgs:
                result.append(s)
        classnames = self.__classEnquirer__.getClassNames(self.__name__)
        if classnames:
            for c in classnames:
                result.append(c.split('.')[-1])
        return result


def makeModule(fullname, loader, classEnquirer):
    mod = module(fullname)
    mod.__dict__.update({
        '__loader__': loader,
        '__path__': [],
        '__file__': '<java>',
        '__package__': None,
        '__classEnquirer__': classEnquirer,
    })
    sys.modules[fullname] = mod
    return mod


class JepJavaImporter(object):

    def __init__(self, classEnquirer=None):
        if classEnquirer:
            self.classEnquirer = classEnquirer
        else:
            self.classEnquirer = forName('jep.ClassList').getInstance()

    def find_module(self, fullname, path=None):
        if self.classEnquirer.isJavaPackage(fullname):
            return self  # found a Java package with this name
        return None

    def load_module(self, fullname):
        if fullname in sys.modules:
            return sys.modules[fullname]

        mod = makeModule(fullname, self, self.classEnquirer)
        return mod


def setupImporter(classEnquirer):
    alreadySetup = False
    for importer in sys.meta_path:
        if isinstance(importer, JepJavaImporter):
            alreadySetup = True
            break
    if not alreadySetup:
        sys.meta_path.insert(0,JepJavaImporter(classEnquirer))
