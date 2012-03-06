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

class build_scripts(Command):
    description = "\"build\" scripts (copy and fixup #! line)"

    user_options = [
        ('build-dir=', 'd', "directory to \"build\" (copy) to"),
        ('force', 'f', "forcibly build everything (ignore file timestamps"),
        ('executable=', 'e', "specify final destination interpreter path"),
        ]

    boolean_options = ['force']


    def initialize_options (self):
        self.build_dir = None
        self.scripts = None
        self.force = None
        self.executable = None
        self.outfiles = None

    def finalize_options (self):
        self.set_undefined_options('build',
                                   ('build_scripts', 'build_dir'),
                                   ('force', 'force'),
                                   ('executable', 'executable'))
        self.scripts = self.distribution.scripts

    def get_source_files(self):
        return self.scripts

    def run (self):
        if not self.scripts:
            return
        self.copy_scripts()


    def copy_scripts (self):
        """Copy each script listed in 'self.scripts'; if it's marked as a
        Python script in the Unix way (first line matches 'first_line_re',
        ie. starts with "\#!" and contains "python"), then adjust the first
        line to refer to the current Python interpreter as we copy.
        """
        _sysconfig = __import__('sysconfig')
        self.mkpath(self.build_dir)
        outfiles = []

        install = self.get_finalized_command('install')
        context = dict(
            prefix=install.prefix,
            install_base=install.install_base,
            install_platbase=install.install_platbase,
            install_lib=install.install_lib,
        )

        for script in self.scripts:
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
                    oldmode = os.stat(file)[ST_MODE] & 07777
                    newmode = (oldmode | 0555) & 07777
                    if newmode != oldmode:
                        log.info("changing mode of %s from %o to %o",
                                 file, oldmode, newmode)
                        os.chmod(file, newmode)

    # copy_scripts ()

# class build_scripts

#class install_script_template(install_scripts):
#    def write_script(self, script_name, contents, mode="t", *ignored):
#        """Write an executable file to the scripts directory"""
#
#        install = self.get_finalized_command('install')
#        context = dict(
#            prefix=install.prefix,
#            install_base=install.install_base,
#            install_platbase=install.install_platbase,
#            install_lib=install.install_lib,
#        )
#
#        log.info("Installing %s script to %s", script_name, self.install_dir)
#        target = os.path.join(self.install_dir, script_name)
#        self.outfiles.append(target)
#
#        if not self.dry_run:
#            ensure_directory(target)
#            f = open(target, "w" + mode)
#            f.write(contents.format(context))
#            f.close()
#            chmod(target, 0755)
