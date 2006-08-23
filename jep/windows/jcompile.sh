#!/bin/bash

export JAVA_HOME='/cygdrive/c/Progra~1/Java/jdk1.5.0_08'
export PATH=$JAVA_HOME/bin/:$PATH

echo $PATH

JAVAH=javah
JAVAC=javac
CLASSPATH=../
JAVACOPT='-deprecation -classpath .\ext\bsf.jar'

run() {
    echo $*
    $*

    if [ $? != 0 ]; then
        echo "Error"
        exit 1
    fi
}

pushd ../ >/dev/null

if [ "$1" == "clean" ]; then
    run rm -f *.class *.jar
else
    run $JAVAC $JAVACOPT *.java
    run $JAVAH -o jep.h -classpath ../ jep.Jep
    ./makejar.sh jep/ jep.jar
fi

popd >/dev/null
