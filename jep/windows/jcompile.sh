#!/bin/bash

TOPDIR=`pwd`/..
JAVAH=javah
JAVAC=javac
CLASSPATH=..\\..\\ext\\bsf.jar\;..\\
SUBCLASSPATH=..\\..\\..\\ext\\bsf.jar\;..\\..\\
JAVACOPT='-Xlint:unchecked -deprecation -Xmaxerrs 5'
JWIN_DIR=`pwd`

#testing variables
JAVA_TEST=Test.java
CACHE=./jcompile.cache
CCACHE=./jcompile.cache.javac

LOG=config.log

SCRIPT_IMPORTS="import javax.script.Bindings;
import javax.script.ScriptContext;
import javax.script.ScriptEngine;
import javax.script.ScriptEngineFactory;
import javax.script.ScriptException;
import javax.script.SimpleBindings;"

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

rlog() {
    if [ ! -z "$JCOMPILE_DEBUG" ]; then
        echo $*
    fi

    echo $* >> $LOG
    $* >> $LOG 2>&1
    return $?
}

cache_new() {
    rm -f $CACHE
}

# $1 should be variable name
# $2 should be value
cache() {
    # bash screws it up if we do this on one line. *shrugs*
    echo -n "$1=" >> $CACHE
    echo $2 >> $CACHE
}

# $1 should be name of cache variable
# $2 should be imports to test
test_javac() {
    eval cache_var=\$$1
    if [ ! -z "$cache_var" ]; then
        echo "testing $i (cached) ... $cache_var"
        cache $1 $cache_var
        ret=$cache_var
        return
    fi
    echo -n "testing $1 ... "

    echo "$2" > $JAVA_TEST
    cat << \EOF >> $JAVA_TEST
public class Test {
}
EOF

    ret=no
    rlog $JAVAC $JAVACOPT Test.java
    if [ $? -eq 0 ]; then
        ret=yes
    fi

    rlog rm -f Test.java
    rlog rm -f Test.class

    cache $1 $ret
    echo $ret
    return
}

rm -f $LOG
echo "`date`" > $LOG
echo "using javac `which javac`" >> $LOG

# invalidate cache if javac changes
mycc=`javac -version 2>&1 | head -1 | awk '{print $NF}'`
if [ -r $CCACHE ]; then
    rlog source $CCACHE
fi

# bash screws this up. dunno why
echo "CACHE_CC=$mycc" > $CCACHE

if [ "$mycc" = "$CACHE_CC" ] && [ -r $CACHE ]; then
    echo "Loading $CACHE ..."
    rlog source $CACHE
fi

# ----------------------------------------
# configure

if [ "$1" != "clean" ]; then
    # make a new cache
    cache_new

    # test if we have javax.script
    test_javac HAS_SCRIPTING "$SCRIPT_IMPORTS"
    HAS_SCRIPTING=$ret
fi

run pushd ../src/jep/ >/dev/null

if [ "$1" == "clean" ]; then
    run rm -f *.class *.jar

    run pushd python
    run rm -f *.class *.jar
    run popd

    run pushd $JWIN_DIR
    run rm -f $CACHE $CCACHE
    run popd
else
    jep_files=`ls -1 --color=no *.java`

    # don't attempt to compile javax.script support if we don't have it
    if [ "$HAS_SCRIPTING" = "no" ]; then
        jep_files=`echo "$jep_files" | grep -v ScriptEngine`
    fi

    run $JAVAC -classpath $CLASSPATH $JAVACOPT $jep_files

    pushd python
    run $JAVAC -classpath $SUBCLASSPATH $JAVACOPT *.java
    run $JAVAH -o jep_object.h -classpath $SUBCLASSPATH jep.python.PyObject
    popd

    run $JAVAH -o jep.h -classpath ../ jep.Jep
    run $JAVAH -o invocationhandler.h -classpath ../ jep.InvocationHandler

    pushd $TOPDIR
    run ./makejar.sh jep/ jep.jar
    popd

    # copy jep.jar/.dll to ext dir for jrunscript support
    run cp -f $TOPDIR/jep.jar $JAVA_HOME/jre/lib/ext/
    run cp -f $TOPDIR/windows/Active/jep.dll /cygdrive/c/WINDOWS/system32/jep.dll
fi

run popd >/dev/null

