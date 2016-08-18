from distutils.dist import Distribution


class JepDistribution(Distribution):

    def __init__(self, attrs=None):
        self.java_files = None
        self.javah_files = None
        self.extra_jar_files = None
        Distribution.__init__(self, attrs)
