"""
Fork of distutils' build_scripts command to handle creating
shell scripts with proper path info in the python prefix.
"""

import os
from stat import ST_MODE
from distutils.core import Command
from distutils.dep_util import newer
from distutils.util import convert_path
from distutils import log
from distutils import sysconfig
from commands.util import is_osx, is_windows


class build_scripts(Command):
    description = "\"build\" scripts (copy and fixup #! line)"

    user_options = [
        ('build-dir=', 'd', "directory to \"build\" (copy) to"),
        ('force', 'f', "forcibly build everything (ignore file timestamps"),
        ('executable=', 'e', "specify final destination interpreter path"),
    ]

    boolean_options = ['force']

    def initialize_options(self):
        self.build_dir = None
        self.scripts = None
        self.force = None
        self.executable = None
        self.outfiles = None

    def finalize_options(self):
        self.set_undefined_options('build',
                                   ('build_scripts', 'build_dir'),
                                   ('force', 'force'),
                                   ('executable', 'executable'))
        self.scripts = self.distribution.scripts

    def get_source_files(self):
        return self.scripts

    def run(self):
        if not self.scripts:
            return
        self.copy_scripts()

    def copy_scripts(self):
        """Copy each script listed in 'self.scripts'; if it's marked as a
        Python script in the Unix way (first line matches 'first_line_re',
        ie. starts with "\#!" and contains "python"), then adjust the first
        line to refer to the current Python interpreter as we copy.
        """
        self.mkpath(self.build_dir)
        outfiles = []

        install = self.get_finalized_command('install')
        context = dict(
            version=self.distribution.metadata.get_version(),
            install_base=install.install_base,
            install_platbase=install.install_platbase,
            install_lib=install.install_lib,
            virtual_env=os.environ.get('VIRTUAL_ENV') or '',
            ld_library_path='',
            ld_preload='',
        )

        if not is_osx() and not is_windows():
            context['ld_library_path'] = 'LD_LIBRARY_PATH="' + \
                                           sysconfig.get_config_var('LIBDIR') + \
                                           ':{0}"; export LD_LIBRARY_PATH'.format(
                                           install.install_lib)
            
            # set the LD_PRELOAD environment variable if we can locate the
            # libpython<version>.so library.
            lib_python = os.path.join(sysconfig.get_config_var('LIBDIR'),
                                      sysconfig.get_config_var('LDLIBRARY'))
            if os.path.exists(lib_python):
                context['ld_preload'] = 'LD_PRELOAD="{0}"; export LD_PRELOAD'.format(lib_python)

            else:
                # x64 systems will tend to also have a MULTIARCH folder
                lib_python = os.path.join(sysconfig.get_config_var('LIBDIR'),
                                          sysconfig.get_config_var('MULTIARCH'),
                                          sysconfig.get_config_var('LDLIBRARY'))
                if os.path.exists(lib_python):
                    context['ld_preload'] = 'LD_PRELOAD="{0}"; export LD_PRELOAD'.format(lib_python)

        for script in self.scripts:
            if is_windows():
                script='{0}.bat'.format(script)
            script = convert_path(script)
            outfile = os.path.join(self.build_dir, os.path.basename(script))
            outfiles.append(outfile)

            if not self.force and not newer(script, outfile):
                log.debug("not copying %s (up-to-date)", script)
                continue

            # Always open the file, but ignore failures in dry-run mode --
            # that way, we'll get accurate feedback if we can read the
            # script.
            try:
                f = open(script, "r")
            except IOError:
                if not self.dry_run:
                    raise
                f = None

            log.info("copying and adjusting %s -> %s", script,
                     self.build_dir)
            if not self.dry_run:
                outf = open(outfile, "w")
                outf.write(f.read().format(**context))
                outf.close()
            if f:
                f.close()

        if os.name == 'posix':
            for file in outfiles:
                if self.dry_run:
                    log.info("changing mode of %s", file)
                else:
                    oldmode = os.stat(file)[ST_MODE] & 0o7777
                    newmode = (oldmode | 0o555) & 0o7777
                    if newmode != oldmode:
                        log.info("changing mode of %s from %o to %o",
                                 file, oldmode, newmode)
                        os.chmod(file, newmode)

    # copy_scripts ()

# class build_scripts
