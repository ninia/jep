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
OPTS=-c0f

if test -z $1 -o -z $2; then
    echo "
Usage ./makejar.sh [path/to/target/] [jarname.jar]
Example: ./makejar.sh com/trinitycapital/DB/ DB.jar
"
    exit 1
fi

echo "cd .."
cd ..

files=`find $1 -name "*.class" -maxdepth 1`

if test -e ${1}jarfiles; then
    echo "Using jarfiles...."
    for f in `cat $1/jarfiles`; do
        # without newlines, probably overly paranoid
        files="$files $f"
    done
fi

echo $JAR $OPTS ${1}${2} $files
$JAR $OPTS ${1}${2} $files
ret=$?

if test "$ret" == "0" -a -e ${1}manifest; then
    echo "Adding manifest information...."
    $JAR -umf ${1}manifest ${1}${2}
fi

echo "cd jep"
cd jep

exit $ret
