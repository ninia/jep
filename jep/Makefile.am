AM_CPPFLAGS = $(all_includes)

lib_LTLIBRARIES = libjep.la
libjep_la_LDFLAGS = -version-info 1:0:0 -no-undefined
libjep_la_SOURCES = jep.c jep.1 system.h aclocal.m4 \
	pyembed.c \
	util.c \
	pyjmethod.c \
	pyjobject.c \
	pyjclass.c \
	pyjfield.c

METASOURCES = AUTO

#  uncomment the following if jep requires the math library
#jep_LDADD=-lm

EXTRA_DIST=jep.lsm.in jep.spec.in jep.texinfo python.m4

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#  build and install the .info pages
info_TEXINFOS = jep.texinfo
libjep_la_TEXINFOS = gpl.texinfo

#  install the man pages
man_MANS=jep.1

BUILT_SOURCES = Jep.class Test.class JepException.class jep.jar

CLEANFILES = *.class jep.h *.jar
Jep.class: Jep.java
	$(JAVAC) -classpath ../ Jep.java
	$(JAVAH) -o jep.h -classpath ../ jep.Jep

Test.class: Test.java
	$(JAVAC) -classpath ../ Test.java

JepException.class: JepException.java
	$(JAVAC) -classpath ../ JepException.java

jep.jar: Jep.class Test.class JepException.class
	./makejar.sh jep/ jep.jar
