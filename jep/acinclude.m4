#serial 3

dnl From Jim Meyering

dnl Define HAVE_STRUCT_UTIMBUF if `struct utimbuf' is declared --
dnl usually in <utime.h>.
dnl Some systems have utime.h but don't declare the struct anywhere.

AC_DEFUN(jm_CHECK_TYPE_STRUCT_UTIMBUF,
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
AC_REQUIRE([AC_PROG_CPP])
JAVAH=$JAVA_HOME/bin/javah
#AC_PATH_PROG(JAVAH,javah)
if test -e $JAVAH; then
  AC_TRY_CPP([#include <jni.h>],,[
    ac_save_CPPFLAGS="$CPPFLAGS"
changequote(, )dnl
    path_javah=`echo $JAVAH | $lt_cv_path_SED 's/javah//g'`
    ac_dir=`echo $path_javah | $lt_cv_path_SED 's,\(.*\)/[^/]*/[^/]*$,\1/include,'`
    ac_machdep=`echo $build_os | $lt_cv_path_SED 's,[-0-9].*,,' | $lt_cv_path_SED 's,cygwin,win32,'`
changequote([, ])dnl
    if test x"$ac_dir" == x; then
        CPPFLAGS="$ac_save_CPPFLAGS -I$JAVA_HOME/include"
    else
        CPPFLAGS="$ac_save_CPPFLAGS -I$ac_dir -I$ac_dir/$ac_machdep"
    fi
    AC_TRY_CPP([#include <jni.h>],
               ac_save_CPPFLAGS="$CPPFLAGS",
               AC_MSG_ERROR([unable to include <jni.h>]))
    CPPFLAGS="$ac_save_CPPFLAGS"])
    AC_SUBST([JAVAH], $JAVAH)
else
    AC_MSG_WARN([javah was $JAVAH])
    AC_MSG_ERROR([Couldn't find javah. Make sure JAVA_HOME is set correctly.])
fi])

dnl shutup the compiler, long double is from Python.h
AC_DEFUN([AC_CHECK_LONG_DOUBLE], [
AC_REQUIRE([AC_PROG_CPP])
ac_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS -Wno-long-double"
AC_TRY_CPP([int main(int argc, char **argv) {}],
    CPPFLAGS="$CPPFLAGS",
    CPPFLAGS="$ac_save_CPPFLAGS")
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

if test "${JAVA_HOME}X" == "X"; then
    AC_MSG_ERROR([environment variable JAVA_HOME not set.])
else
    PATH="$JAVA_HOME/bin:$PATH"
    AC_MSG_RESULT([yes])
fi
])

dnl mrj, check python version
AC_DEFUN([AC_CHECK_PYTHON_VERSION], [
AC_MSG_CHECKING([python version >= 2])
AC_TRY_CPP([
#include "Python.h"
#if PY_MAJOR_VERSION < 2
#  error Python version 2.2 or greater is required.
#endif
#if PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION < 2
#  error Python version 2.2 or greater is required.
#endif
],
    AC_MSG_RESULT([yes]),
    AC_MSG_ERROR([python 2.2 or greater is required]))
])

AC_DEFUN([AC_CHECK_PYTHON_THREAD], [
AC_MSG_CHECKING([python has threads])
AC_TRY_CPP([
#include "Python.h"
#if !WITH_THREAD
#  error threads required
#endif
],
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
