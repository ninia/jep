#!/bin/sh

#makejar.sh
#utility script to cd up one directory and make a new jar
#mrj 9-18-02

#try to be more portable, make specific to jep package
if test -z $JAVA_HOME; then
    echo "JAVA_HOME not set!"
    exit 1
fi

JAR=${JAVA_HOME}/bin/jar
# set jarfile one directory up because we're going to cd into src
JARFILE=../$2
OPTS=-u0f

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

if test -z $1 -o -z $2; then
    echo "
Usage ./makejar.sh [path/to/target/] [jarname.jar]
Example: ./makejar.sh com/trinitycapital/DB/ DB.jar
"
    exit 1
fi

run cd src

run test -f manifest

#add existing META-INF
if test -d META-INF; then
    run $JAR -cfm $JARFILE manifest META-INF/services/javax.script.ScriptEngineFactory
else
    echo "Couldn't find META-INF, pwd is `pwd`"
    exit 1
fi

files=`find $1 -name "*.class"`

if test -f ${1}jarfiles; then
    echo "Using jarfiles...."
    for f in `cat ${1}jarfiles`; do
        # without newlines, probably overly paranoid
        files="$files $f"
    done
fi

run $JAR $OPTS $JARFILE $files

run cd jep

