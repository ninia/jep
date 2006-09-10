#!/bin/bash

export JAVA_HOME='/cygdrive/c/Progra~1/Java/jdk1.6.0'
export PATH=$JAVA_HOME/bin/:$PATH

JAVAH=javah
JAVAC=javac
CLASSPATH=ext\\bsf.jar\;..\\
SUBCLASSPATH=..\\ext\\bsf.jar\;..\\..\\
JAVACOPT='-Xlint:unchecked -deprecation -Xmaxerrs 5'


failed() {
    if [ $? != 0 ]; then
        echo "Error"
        exit 1
    fi
}

run() {
    echo $*
    $*

    failed
}

pushd ../ >/dev/null

if [ "$1" == "clean" ]; then
    run rm -f *.class *.jar
else
    run $JAVAC -classpath $CLASSPATH $JAVACOPT *.java

    pushd python
    run $JAVAC -classpath $SUBCLASSPATH $JAVACOPT *.java
    run $JAVAH -o jep_object.h -classpath $SUBCLASSPATH jep.python.PyObject
    popd

    run $JAVAH -o jep.h -classpath ../ jep.Jep
    ./makejar.sh jep/ jep.jar
fi

popd >/dev/null
