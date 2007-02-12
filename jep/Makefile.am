SUBDIRS = src .

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
	console.py \
	memtest.py \
	testswing.py \
	src/jep/jep.h \
	makejar.sh \
	jep.jar \
	ext/bsf.jar \
	src/manifest \
	windows/jep.dsp \
	windows/setup.nsi \
	windows/jcompile.sh \
	javadoc/ \
	src/jep/winconfig.h \
	src/META-INF \
	src/jep/python/jep_object.h \
	src/jep/python/PyObject.java \
	src/jep/python/PyModule.java \
	src/jep/python/PyClass.java \
	src/jep/ClassList.java \
	src/jep/JepScriptEngineFactory.java \
	src/jep/JepScriptEngine.java \
	src/jep/Proxy.java \
	src/jep/InvocationHandler.java \
	src/jep/Util.java \
	src/jep/invocationhandler.h

dist-hook:
	rm -rf `find $(distdir) -name .svn`

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#  install the man pages
man_MANS=jep.1

CLEANFILES = *.class *.jar src/jep/jep.h src/jep/python/jep_object.h

JAVACOPT = -deprecation -classpath src/

BUILT_SOURCES = src/jep/jep.h \
	src/jep/python/jep_object.h \
	src/jep/invocationhandler.h

all: jep.jar

src/jep/python/jep_object.h: src/jep/python/PyObject.class
	$(JAVAH) -o src/jep/python/jep_object.h -classpath src/ jep.python.PyObject
# make sure the timestamp gets updated because javac might not
	touch src/jep/python/jep_object.h

src/jep/python/PyObject.class: src/jep/python/PyObject.java
	$(JAVAC) ${JAVACOPT} src/jep/python/PyObject.java

src/jep/python/PyModule.class: src/jep/python/PyModule.java
	$(JAVAC) ${JAVACOPT} src/jep/python/PyModule.java

src/jep/python/PyClass.class: src/jep/python/PyClass.java
	$(JAVAC) ${JAVACOPT} src/jep/python/PyClass.java

src/jep/jep.h: src/jep/Jep.class
	$(JAVAH) -o src/jep/jep.h -classpath src/ jep.Jep
# make sure the timestamp gets updated because javac might not
	touch src/jep/jep.h

src/jep/Jep.class: src/jep/Jep.java
	$(JAVAC) ${JAVACOPT} src/jep/Jep.java

src/jep/Run.class: src/jep/Run.java src/jep/Jep.java
	$(JAVAC) ${JAVACOPT} src/jep/Run.java

src/jep/BSFJepEngine.class: src/jep/BSFJepEngine.java src/jep/Jep.java
	$(JAVAC) ${JAVACOPT}:./ext/bsf.jar src/jep/BSFJepEngine.java

src/jep/Test.class: src/jep/Test.java
	$(JAVAC) ${JAVACOPT} src/jep/Test.java

src/jep/JepException.class: src/jep/JepException.java
	$(JAVAC) ${JAVACOPT} src/jep/JepException.java

src/jep/ClassList.class: src/jep/ClassList.java
	$(JAVAC) ${JAVACOPT} src/jep/ClassList.java

src/jep/JepScriptEngine.class: src/jep/JepScriptEngine.java
	$(JAVAC) ${JAVACOPT} src/jep/JepScriptEngine.java

src/jep/JepScriptEngineFactory.class: src/jep/JepScriptEngineFactory.java
	$(JAVAC) ${JAVACOPT} src/jep/JepScriptEngineFactory.java

src/jep/InvocationHandler.class: src/jep/InvocationHandler.java
	$(JAVAC) ${JAVACOPT} src/jep/InvocationHandler.java

src/jep/invocationhandler.h: src/jep/InvocationHandler.class
	$(JAVAH) -o src/jep/invocationhandler.h -classpath src/ jep.InvocationHandler
# make sure the timestamp gets updated because javac might not
	touch src/jep/invocationhandler.h

src/jep/Proxy.class: src/jep/Proxy.java
	$(JAVAC) ${JAVACOPT} src/jep/Proxy.java

src/jep/Util.class: src/jep/Util.java
	$(JAVAC) ${JAVACOPT} src/jep/Util.java

jep.jar:  \
	src/jep/python/PyObject.class \
	src/jep/python/PyModule.class \
	src/jep/python/PyClass.class \
	src/jep/Jep.class \
	src/jep/JepException.class \
	src/jep/Test.class \
	src/jep/Run.class \
	src/jep/InvocationHandler.class \
	src/jep/Proxy.class \
	src/jep/Util.class \
	src/jep/ClassList.class $(JAVAX_SCRIPT_CLASSES)

	./makejar.sh jep/ jep.jar

javadoc: jep.jar
	${JAVA_HOME}/bin/javadoc -version -windowtitle 'Java Embedded Python' -author -breakiterator -classpath jep.jar:ext/bsf.jar -sourcepath src/ jep -d javadoc/

