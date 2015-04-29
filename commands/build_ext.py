"""
Fork of distutils' build_ext command to handle using a
patched version of the msvc9compiler.
"""

import sys, os, re
from distutils.command.build_ext import build_ext as old_build_ext
from distutils.errors import *
from distutils.sysconfig import customize_compiler
from distutils.util import get_platform

class build_ext (old_build_ext):

    def run(self):
        # 'self.extensions', as supplied by setup.py, is a list of
        # Extension instances.  See the documentation for Extension (in
        # distutils.extension) for details.
        #
        # For backwards compatibility with Distutils 0.8.2 and earlier, we
        # also allow the 'extensions' list to be a list of tuples:
        #    (ext_name, build_info)
        # where build_info is a dictionary containing everything that
        # Extension instances do except the name, with a few things being
        # differently named.  We convert these 2-tuples to Extension
        # instances as needed.

        if not self.extensions:
            return

        # If we were asked to build any C/C++ libraries, make sure that the
        # directory where we put them is in the library search path for
        # linking extensions.
        if self.distribution.has_c_libraries():
            build_clib = self.get_finalized_command('build_clib')
            self.libraries.extend(build_clib.get_library_names() or [])
            self.library_dirs.append(build_clib.build_clib)

        # Setup the CCompiler object that we'll use to do all the
        # compiling and linking
        self.compiler = new_compiler(compiler=self.compiler,
                                     verbose=self.verbose,
                                     dry_run=self.dry_run,
                                     force=self.force)
        customize_compiler(self.compiler)
        # If we are cross-compiling, init the compiler now (if we are not
        # cross-compiling, init would not hurt, but people may rely on
        # late initialization of compiler even if they shouldn't...)
        if os.name == 'nt' and self.plat_name != get_platform():
            self.compiler.initialize(self.plat_name)

        # And make sure that any compile/link-related options (which might
        # come from the command-line or from the setup script) are set in
        # that CCompiler object -- that way, they automatically apply to
        # all compiling and linking done here.
        if self.include_dirs is not None:
            self.compiler.set_include_dirs(self.include_dirs)
        if self.define is not None:
            # 'define' option is a list of (name,value) tuples
            for (name, value) in self.define:
                self.compiler.define_macro(name, value)
        if self.undef is not None:
            for macro in self.undef:
                self.compiler.undefine_macro(macro)
        if self.libraries is not None:
            self.compiler.set_libraries(self.libraries)
        if self.library_dirs is not None:
            self.compiler.set_library_dirs(self.library_dirs)
        if self.rpath is not None:
            self.compiler.set_runtime_library_dirs(self.rpath)
        if self.link_objects is not None:
            self.compiler.set_link_objects(self.link_objects)

        # Now actually compile and link everything.
        self.build_extensions()

# class build_ext

# Map a sys.platform/os.name ('posix', 'nt') to the default compiler
# type for that platform. Keys are interpreted as re match
# patterns. Order is important; platform mappings are preferred over
# OS names.
_default_compilers = (

    # Platform string mappings

    # on a cygwin built python we can use gcc like an ordinary UNIXish
    # compiler
    ('cygwin.*', 'unix'),
    ('os2emx', 'emx'),

    # OS name mappings
    ('posix', 'unix'),
    ('nt', 'msvc'),

    )

def get_default_compiler(osname=None, platform=None):
    """ Determine the default compiler to use for the given platform.

        osname should be one of the standard Python OS names (i.e. the
        ones returned by os.name) and platform the common value
        returned by sys.platform for the platform in question.

        The default values are os.name and sys.platform in case the
        parameters are not given.

    """
    if osname is None:
        osname = os.name
    if platform is None:
        platform = sys.platform
    for pattern, compiler in _default_compilers:
        if re.match(pattern, platform) is not None or \
           re.match(pattern, osname) is not None:
            return compiler
    # Default to Unix compiler
    return 'unix'

# Map compiler types to (module_name, class_name) pairs -- ie. where to
# find the code that implements an interface to this compiler.
compiler_class = { 'unix':    ('distutils.unixccompiler', 'UnixCCompiler',
                               "standard UNIX-style compiler"),
                   'msvc':    ('commands.msvc9compiler', 'MSVCCompiler',
                               "Microsoft Visual C++"),
                   'cygwin':  ('distutils.cygwinccompiler', 'CygwinCCompiler',
                               "Cygwin port of GNU C Compiler for Win32"),
                   'mingw32': ('distutils.cygwinccompiler', 'Mingw32CCompiler',
                               "Mingw32 port of GNU C Compiler for Win32"),
                   'bcpp':    ('distutils.bcppcompiler', 'BCPPCompiler',
                               "Borland C++ Compiler"),
                   'emx':     ('distutils.emxccompiler', 'EMXCCompiler',
                               "EMX port of GNU C Compiler for OS/2"),
                 }

def show_compilers():
    """Print list of available compilers (used by the "--help-compiler"
    options to "build", "build_ext", "build_clib").
    """
    # XXX this "knows" that the compiler option it's describing is
    # "--compiler", which just happens to be the case for the three
    # commands that use it.
    from distutils.fancy_getopt import FancyGetopt
    compilers = []
    for compiler in compiler_class.keys():
        compilers.append(("compiler="+compiler, None,
                          compiler_class[compiler][2]))
    compilers.sort()
    pretty_printer = FancyGetopt(compilers)
    pretty_printer.print_help("List of available compilers:")


def new_compiler(plat=None, compiler=None, verbose=0, dry_run=0, force=0):
    """Generate an instance of some CCompiler subclass for the supplied
    platform/compiler combination.  'plat' defaults to 'os.name'
    (eg. 'posix', 'nt'), and 'compiler' defaults to the default compiler
    for that platform.  Currently only 'posix' and 'nt' are supported, and
    the default compilers are "traditional Unix interface" (UnixCCompiler
    class) and Visual C++ (MSVCCompiler class).  Note that it's perfectly
    possible to ask for a Unix compiler object under Windows, and a
    Microsoft compiler object under Unix -- if you supply a value for
    'compiler', 'plat' is ignored.
    """
    if plat is None:
        plat = os.name

    try:
        if compiler is None:
            compiler = get_default_compiler(plat)

        (module_name, class_name, long_description) = compiler_class[compiler]
    except KeyError:
        msg = "don't know how to compile C/C++ code on platform '%s'" % plat
        if compiler is not None:
            msg = msg + " with '%s' compiler" % compiler
        raise DistutilsPlatformError, msg

    try:
        __import__ (module_name)
        module = sys.modules[module_name]
        klass = vars(module)[class_name]
    except ImportError:
        raise DistutilsModuleError, \
              "can't compile C/C++ code: unable to load module '%s'" % \
              module_name
    except KeyError:
        raise DistutilsModuleError, \
              ("can't compile C/C++ code: unable to find class '%s' " +
               "in module '%s'") % (class_name, module_name)

    # XXX The None is necessary to preserve backwards compatibility
    # with classes that expect verbose to be the first positional
    # argument.
    return klass(None, dry_run, force)


def gen_preprocess_options(macros, include_dirs):
    """Generate C pre-processor options (-D, -U, -I) as used by at least
    two types of compilers: the typical Unix compiler and Visual C++.
    'macros' is the usual thing, a list of 1- or 2-tuples, where (name,)
    means undefine (-U) macro 'name', and (name,value) means define (-D)
    macro 'name' to 'value'.  'include_dirs' is just a list of directory
    names to be added to the header file search path (-I).  Returns a list
    of command-line options suitable for either Unix compilers or Visual
    C++.
    """
    # XXX it would be nice (mainly aesthetic, and so we don't generate
    # stupid-looking command lines) to go over 'macros' and eliminate
    # redundant definitions/undefinitions (ie. ensure that only the
    # latest mention of a particular macro winds up on the command
    # line).  I don't think it's essential, though, since most (all?)
    # Unix C compilers only pay attention to the latest -D or -U
    # mention of a macro on their command line.  Similar situation for
    # 'include_dirs'.  I'm punting on both for now.  Anyways, weeding out
    # redundancies like this should probably be the province of
    # CCompiler, since the data structures used are inherited from it
    # and therefore common to all CCompiler classes.

    pp_opts = []
    for macro in macros:

        if not (isinstance(macro, tuple) and
                1 <= len (macro) <= 2):
            raise TypeError, \
                  ("bad macro definition '%s': " +
                   "each element of 'macros' list must be a 1- or 2-tuple") % \
                  macro

        if len (macro) == 1:        # undefine this macro
            pp_opts.append ("-U%s" % macro[0])
        elif len (macro) == 2:
            if macro[1] is None:    # define with no explicit value
                pp_opts.append ("-D%s" % macro[0])
            else:
                # XXX *don't* need to be clever about quoting the
                # macro value here, because we're going to avoid the
                # shell at all costs when we spawn the command!
                pp_opts.append ("-D%s=%s" % macro)

    for dir in include_dirs:
        pp_opts.append ("-I%s" % dir)

    return pp_opts


def gen_lib_options(compiler, library_dirs, runtime_library_dirs, libraries):
    """Generate linker options for searching library directories and
    linking with specific libraries.

    'libraries' and 'library_dirs' are, respectively, lists of library names
    (not filenames!) and search directories.  Returns a list of command-line
    options suitable for use with some compiler (depending on the two format
    strings passed in).
    """
    lib_opts = []

    for dir in library_dirs:
        lib_opts.append(compiler.library_dir_option(dir))

    for dir in runtime_library_dirs:
        opt = compiler.runtime_library_dir_option(dir)
        if isinstance(opt, list):
            lib_opts.extend(opt)
        else:
            lib_opts.append(opt)

    # XXX it's important that we *not* remove redundant library mentions!
    # sometimes you really do have to say "-lfoo -lbar -lfoo" in order to
    # resolve all symbols.  I just hope we never have to say "-lfoo obj.o
    # -lbar" to get things to work -- that's certainly a possibility, but a
    # pretty nasty way to arrange your C code.

    for lib in libraries:
        lib_dir, lib_name = os.path.split(lib)
        if lib_dir != '':
            lib_file = compiler.find_library_file([lib_dir], lib_name)
            if lib_file is not None:
                lib_opts.append(lib_file)
            else:
                compiler.warn("no library file corresponding to "
                              "'%s' found (skipping)" % lib)
        else:
            lib_opts.append(compiler.library_option(lib))

    return lib_opts
