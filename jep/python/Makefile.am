
noinst_LIBRARIES = libjeppython.a

libjeppython_a_SOURCES = jep_object.c

INCLUDES = -I@top_srcdir@

CLEANFILES = *.class *.jar

JAVACOPT=-deprecation -classpath ../../

all: PyObject.class PyModule.class

PyObject.class: PyObject.java
	$(JAVAC) ${JAVACOPT} PyObject.java
	$(JAVAH) -o jep_object.h -classpath ../../ jep.python.PyObject

PyModule.class: PyModule.java
	$(JAVAC) ${JAVACOPT} PyModule.java

