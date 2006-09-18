SUBDIRS = src

EXTRA_DIST=python.m4 \
	jep.1 \
	autogen.sh \
	src/jep/pyembed.h \
	src/jep/util.h \
	src/jep/pyjmethod.h \
	src/jep/pyjobject.h \
	src/jep/pyjarray.h \
	src/jep/pyjclass.h \
	src/jep/pyjfield.h \
	src/jep/Jep.java \
	src/jep/Run.java \
	src/jep/BSFJepEngine.java \
	src/jep/JepException.java \
	src/jep/Test.java \
	test.py \
	src/jep/jep.h \
	makejar.sh \
	jep.jar \
	ext/bsf.jar \
	windows/jep.dsp \
	windows/setup.nsi \
	javadoc/ \
	console.py \
	memtest.py \
	src/jep/winconfig.h \
	src/META-INF \
	src/jep/python/jep_object.h \
	src/jep/python/PyObject.java \
	src/jep/python/PyModule.java \
	src/jep/ClassList.java \
	src/jep/JepScriptEngineFactory.java \
	src/jep/JepScriptEngine.java

dist-hook:
	rm -rf `find $(distdir) -name .svn`

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#  install the man pages
man_MANS=jep.1

BUILT_SOURCES = jep.jar

jep.jar:
	./makejar.sh jep/ jep.jar

javadoc: jep.jar
	${JAVA_HOME}/bin/javadoc -version -windowtitle 'Java Embedded Python' -author -breakiterator -classpath jep.jar:ext/bsf.jar -sourcepath src/ jep -d javadoc/

jclean:
	rm -f *.class *.jar
