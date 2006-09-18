AM_CPPFLAGS = $(all_includes)

SUBDIRS = python

LDADD = python/libjeppython.a

lib_LTLIBRARIES = libjep.la
libjep_la_LDFLAGS = -version-info 1:1:0 -no-undefined $(JAVA_LDFLAGS)
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
JAVACOPT=-deprecation -classpath ../

all: Jep.class Test.class JepException.class Run.class ClassList.class JepScriptEngine.class JepScriptEngineFactory.class

Jep.class: Jep.java
	$(JAVAC) ${JAVACOPT} Jep.java
	$(JAVAH) -o jep.h -classpath ../ jep.Jep

Run.class: Run.java Jep.java
	$(JAVAC) ${JAVACOPT} Run.java

BSFJepEngine.class: BSFJepEngine.java Jep.java
	$(JAVAC) ${JAVACOPT}:./ext/bsf.jar BSFJepEngine.java

Test.class: Test.java
	$(JAVAC) ${JAVACOPT} Test.java

JepException.class: JepException.java
	$(JAVAC) ${JAVACOPT} JepException.java

ClassList.class: ClassList.java
	$(JAVAC) ${JAVACOPT} ClassList.java

JepScriptEngine.class: JepScriptEngine.java
	$(JAVAC) ${JAVACOPT} JepScriptEngine.java

JepScriptEngineFactory.class: JepScriptEngineFactory.java
	$(JAVAC) ${JAVACOPT} JepScriptEngineFactory.java

jclean:
	rm -f *.class *.jar
