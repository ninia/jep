AM_CPPFLAGS = $(all_includes)

lib_LTLIBRARIES = libjep.la
libjep_la_LDFLAGS = -version-info 1:0:0 -no-undefined
libjep_la_SOURCES = jep.c jep.1 system.h aclocal.m4 \
	pyembed.c \
	util.c \
	pyjmethod.c \
	pyjobject.c \
	pyjclass.c \
	pyjfield.c \
	pyjarray.c

METASOURCES = AUTO

#  uncomment the following if jep requires the math library
#jep_LDADD=-lm

EXTRA_DIST=python.m4 \
	autogen.sh \
	pyembed.h \
	util.h \
	pyjmethod.h \
	pyjobject.h \
	pyjarray.h \
	pyjclass.h \
	pyjfield.h \
	Jep.java \
	Run.java \
	BSFJepEngine.java \
	JepException.java \
	Test.java \
	test.py \
	jep.h \
	makejar.sh \
	jep.jar \
	ext/bsf.jar \
	windows/jep.dsp \
	windows/setup.nsi \
	javadoc/

dist-hook:
	rm -rf `find $(distdir)/javadoc -name CVS`

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#  install the man pages
man_MANS=jep.1

BUILT_SOURCES = jep.jar

CLEANFILES = *.class *.jar

JAVACOPT=-deprecation -classpath ../

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

jep.jar: Jep.class Test.class JepException.class Run.class
	./makejar.sh jep/ jep.jar

javadoc: jep.jar
	${JAVA_HOME}/bin/javadoc -version -windowtitle 'Java Embedded Python' -author -breakiterator -classpath ../:ext/bsf.jar jep -d javadoc/
