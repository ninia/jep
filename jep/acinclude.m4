#serial 3

dnl find libjvm, if needed
AC_DEFUN([AC_JAVA_LIBJVM],
[
  AC_MSG_CHECKING([for libjvm])

  java_libs=
  java_ldflags=

  case $host_os in
    darwin*)
      java_ldflags="-Xlinker -framework -Xlinker JavaVM"
      AC_MSG_RESULT([$java_ldflags])
      ;;
    *)
      AC_MSG_RESULT([not needed])
      ;;
  esac

  AC_SUBST(JAVA_LIBS, $java_libs)
  AC_SUBST(JAVA_LDFLAGS, $java_ldflags)
])

dnl From Jim Meyering

dnl Define HAVE_STRUCT_UTIMBUF if `struct utimbuf' is declared --
dnl usually in <utime.h>.
dnl Some systems have utime.h but don't declare the struct anywhere.

AC_DEFUN([jm_CHECK_TYPE_STRUCT_UTIMBUF],
[
  AC_CHECK_HEADERS(utime.h)
  AC_REQUIRE([AC_HEADER_TIME])
  AC_CACHE_CHECK([for struct utimbuf], fu_cv_sys_struct_utimbuf,
    [AC_TRY_COMPILE(
      [
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_UTIME_H
# include <utime.h>
#endif
      ],
      [static struct utimbuf x; x.actime = x.modtime;],
      fu_cv_sys_struct_utimbuf=yes,
      fu_cv_sys_struct_utimbuf=no)
    ])

  if test $fu_cv_sys_struct_utimbuf = yes; then
    AC_DEFINE_UNQUOTED(HAVE_STRUCT_UTIMBUF, 1,
[Define if struct utimbuf is declared -- usually in <utime.h>.
   Some systems have utime.h but don't declare the struct anywhere. ])
  fi
])


AC_DEFUN([AC_PROG_JAVAH],[
AC_MSG_CHECKING([for jni.h])
AC_REQUIRE([AC_PROG_CPP])
JAVAH=$JAVA_HOME/bin/javah

ac_save_CPPFLAGS="$CPPFLAGS"
sdk_inc="${JAVA_HOME}/include"

# older autotools doesn't set this
if test x$lt_cv_path_SED = "x"; then
    lt_cv_path_SED=sed
fi

ac_machdep=`echo $build_os | sed 's/[@<:@]-0-9[@:>@].*//' | sed 's/cygwin/win32/g'`

if test x"$ac_machdep" != x; then
    CPPFLAGS="$ac_save_CPPFLAGS -I$sdk_inc -I$sdk_inc/$ac_machdep"
fi

AC_TRY_COMPILE([#include <jni.h>], [],
    ac_save_CPPFLAGS="$CPPFLAGS",
    AC_MSG_ERROR([unable to include <jni.h>]))

AC_SUBST([JAVAH], $JAVAH)
AC_MSG_RESULT([yes])
])

dnl shutup the compiler, long double is from Python.h
AC_DEFUN([AC_CHECK_LONG_DOUBLE], [
AC_REQUIRE([AC_PROG_CPP])
AC_MSG_CHECKING([how to disable long double warning])
ac_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS -Wno-long-double"
AC_TRY_COMPILE([],[],
    ac_long=0,
    ac_long=1
)
if test "$ac_long" = "0"; then
    AC_MSG_RESULT([-Wno-long-double])
else
    AC_MSG_RESULT([ignored])
    CPPFLAGS="$ac_save_CPPFLAGS"
fi
])

dnl --------------------------------------------------
dnl check features
AC_DEFUN([AC_ARG_DEALLOC], [
AC_ARG_ENABLE([dealloc],
    [  --enable-dealloc        enable object deallocation (default yes)],
    [use_dealloc=$enableval],
    [use_dealloc=yes])

if test $use_dealloc = yes; then
    AC_DEFINE(USE_DEALLOC, 1, [Define if you want to deallocate objects.])
fi
])

dnl mrj
dnl check if we have javax.script, enable/disable support
AC_DEFUN([AC_JAVAX_SCRIPTING],
[
AC_CACHE_CHECK([for javax.script], ac_cv_prog_javax_script_works, [
JAVA_TEST=Test.java
CLASS_TEST=Test.class
cat << \EOF > $JAVA_TEST
/* [#]line __oline__ "configure" */
import javax.script.Bindings;
import javax.script.ScriptContext;
import javax.script.ScriptEngine;
import javax.script.ScriptEngineFactory;
import javax.script.ScriptException;
import javax.script.SimpleBindings;

public class Test {
}
EOF

classes=
ac_cv_prog_javax_script_works=no

if AC_TRY_COMMAND($JAVAC $JAVACFLAGS $JAVA_TEST) >/dev/null 2>&1; then
  ac_cv_prog_javax_script_works=yes
  classes="src/jep/JepScriptEngine.class src/jep/JepScriptEngineFactory.class"
fi

AC_SUBST(JAVAX_SCRIPT_CLASSES, $classes)

#AC_MSG_RESULT($ac_cv_prog_javax_script_works)
rm -f $JAVA_TEST $CLASS_TEST
])
AC_PROVIDE([$0])dnl
])

dnl enable/disable automagic exception mapping
AC_DEFUN([AC_ARG_EXCEPT], [
AC_ARG_ENABLE([map-exceptions],
    [  --enable-map-exceptions enable automagic exception mapping (default yes)],
    [use_except=$enableval],
    [use_except=yes])

if test $use_except = yes; then
    AC_DEFINE(USE_MAPPED_EXCEPTIONS, 1, [Define if you want to map exceptions.])
fi
])

dnl check JAVA_HOME, change path
AC_DEFUN([AC_CHECK_JAVA_HOME], [
AC_MSG_CHECKING([if JAVA_HOME is set])

if test "${JAVA_HOME}X" = "X"; then
    AC_MSG_ERROR([environment variable JAVA_HOME not set.])
else
    PATH="$JAVA_HOME/bin:$PATH"
    AC_MSG_RESULT([yes])
fi
])

dnl mrj, check python version
AC_DEFUN([AC_CHECK_PYTHON_VERSION], [
AC_MSG_CHECKING([python version >= 2])
AC_TRY_COMPILE([
#include "Python.h"
#if PY_MAJOR_VERSION < 2
#  error Python version 2.2 or greater is required.
#endif
#if PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION < 2
#  error Python version 2.2 or greater is required.
#endif
], [],
    AC_MSG_RESULT([yes]),
    AC_MSG_ERROR([python 2.2 or greater is required]))
])

AC_DEFUN([AC_CHECK_PYTHON_THREAD], [
AC_MSG_CHECKING([python has threads])
AC_TRY_COMPILE([
#include "Python.h"
#if !WITH_THREAD
#  error threads required
#endif
], [],
    AC_MSG_RESULT([yes]),
    AC_MSG_ERROR([python must be compiled with thread support]))
])

dnl mrj, find javac
AC_DEFUN([AC_PROG_JAVAC],[
AC_PATH_PROG(JAVAC,javac)
])


dnl http://www.gnu.org/software/ac-archive/htmldoc/ac_prog_javac_works.html
AC_DEFUN([AC_PROG_JAVAC_WORKS],[
AC_CACHE_CHECK([if $JAVAC works], ac_cv_prog_javac_works, [
JAVA_TEST=Test.java
CLASS_TEST=Test.class
cat << \EOF > $JAVA_TEST
/* [#]line __oline__ "configure" */
public class Test {
}
EOF
if AC_TRY_COMMAND($JAVAC $JAVACFLAGS $JAVA_TEST) >/dev/null 2>&1; then
  ac_cv_prog_javac_works=yes
else
  AC_MSG_ERROR([The Java compiler $JAVAC failed (see config.log, check the CLASSPATH?)])
  echo "configure: failed program was:" >&AC_FD_CC
  cat $JAVA_TEST >&AC_FD_CC
fi
rm -f $JAVA_TEST $CLASS_TEST
])
AC_PROVIDE([$0])dnl
])

m4_include([python.m4])


# NOTE: This macro has been submitted for inclusion into   #
#  GNU Autoconf as AC_PROG_SED.  When it is available in   #
#  a released version of Autoconf we should remove this    #
#  macro and use it instead.                               #
# LT_AC_PROG_SED
# --------------
# Check for a fully-functional sed program, that truncates
# as few characters as possible.  Prefer GNU sed if found.
AC_DEFUN([LT_AC_PROG_SED],
[AC_MSG_CHECKING([for a sed that does not truncate output])
AC_CACHE_VAL(lt_cv_path_SED,
[# Loop through the user's path and test for sed and gsed.
# Then use that list of sed's as ones to test for truncation.
as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
for as_dir in $PATH
do
  IFS=$as_save_IFS
  test -z "$as_dir" && as_dir=.
  for lt_ac_prog in sed gsed; do
    for ac_exec_ext in '' $ac_executable_extensions; do
      if $as_executable_p "$as_dir/$lt_ac_prog$ac_exec_ext"; then
        lt_ac_sed_list="$lt_ac_sed_list $as_dir/$lt_ac_prog$ac_exec_ext"
      fi
    done
  done
done
lt_ac_max=0
lt_ac_count=0
# Add /usr/xpg4/bin/sed as it is typically found on Solaris
# along with /bin/sed that truncates output.
for lt_ac_sed in $lt_ac_sed_list /usr/xpg4/bin/sed; do
  test ! -f $lt_ac_sed && break
  cat /dev/null > conftest.in
  lt_ac_count=0
  echo $ECHO_N "0123456789$ECHO_C" >conftest.in
  # Check for GNU sed and select it if it is found.
  if "$lt_ac_sed" --version 2>&1 < /dev/null | grep 'GNU' > /dev/null; then
    lt_cv_path_SED=$lt_ac_sed
    break
  fi
  while true; do
    cat conftest.in conftest.in >conftest.tmp
    mv conftest.tmp conftest.in
    cp conftest.in conftest.nl
    echo >>conftest.nl
    $lt_ac_sed -e 's/a$//' < conftest.nl >conftest.out || break
    cmp -s conftest.out conftest.nl || break
    # 10000 chars as input seems more than enough
    test $lt_ac_count -gt 10 && break
    lt_ac_count=`expr $lt_ac_count + 1`
    if test $lt_ac_count -gt $lt_ac_max; then
      lt_ac_max=$lt_ac_count
      lt_cv_path_SED=$lt_ac_sed
    fi
  done
done
SED=$lt_cv_path_SED
])
AC_MSG_RESULT([$SED])
])
