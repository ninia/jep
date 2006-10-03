AM_CPPFLAGS = $(all_includes)

SUBDIRS = python .

LDADD = python/libjeppython.a

lib_LTLIBRARIES = libjep.la
libjep_la_LDFLAGS = -version-info 2:0:0 -no-undefined $(JAVA_LDFLAGS)
libjep_la_SOURCES = jep.c system.h \
	pyembed.c \
	util.c \
	pyjmethod.c \
	pyjobject.c \
	pyjclass.c \
	pyjfield.c \
	pyjarray.c

METASOURCES = AUTO

#  uncomment the following if jep requires the math library
libjep_la_LIBADD=$(JAVA_LIBS)

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

CLEANFILES = *.class *.jar

