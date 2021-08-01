from distutils.cmd import Command
from distutils.spawn import spawn

import os.path

from commands.java import get_java_home

class javadoc(Command):
    outdir = None

    user_options = [
        ('javadoc=', None,
         'use javadoc (default: {0}/bin/javadoc)'.format(get_java_home())),
    ]

    def initialize_options(self):
        javadoc.outdir = 'javadoc'
        if not os.path.exists(javadoc.outdir):
            os.makedirs(javadoc.outdir)
        self.version = []
        self.javadoc = os.path.join(get_java_home(), 'bin', 'javadoc')

    def finalize_options(self):
        self.version = self.distribution.metadata.get_version()
        self.java_files = self.distribution.java_files

    def run(self):
        spawn([self.javadoc, '-public', '-notimestamp',
                             '-d', os.path.join(javadoc.outdir, self.version), 
                             '-sourcepath', 'src/main/java',
                             '-subpackages', 'jep' ])
