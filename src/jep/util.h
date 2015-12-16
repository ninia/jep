/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/* 
   jep - Java Embedded Python

   Copyright (c) JEP AUTHORS.

   This file is licenced under the the zlib/libpng License.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.
   
   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
   must not claim that you wrote the original software. If you use
   this software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
   must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.   
*/


// shut up the compiler
#include <jni.h>
#ifdef _POSIX_C_SOURCE
# undef _POSIX_C_SOURCE
#endif
#ifdef _FILE_OFFSET_BITS
# undef _FILE_OFFSET_BITS
#endif
#include <Python.h>

#ifndef _Included_util
#define _Included_util

#if PY_MAJOR_VERSION >= 3
#define Py_TPFLAGS_HAVE_ITER 0

#define PyString_FromString(str)          PyUnicode_FromString(str)
#define PyString_Check(str)               PyUnicode_Check(str)
#define PyString_FromFormat(fmt, ...)     PyUnicode_FromFormat(fmt, ##__VA_ARGS__)
// more string macros are defined for python 3 compatibility farther down...

#define PyInt_AsLong(i)                   PyLong_AsLongLong(i)
#define PyInt_AS_LONG(i)                  PyLong_AsLongLong(i)
#define PyInt_Check(i)                    PyLong_Check(i)
#define PyInt_FromLong(i)                 PyLong_FromLongLong(i)
#endif 

#ifndef USE_NUMPY
#define USE_NUMPY 1
#endif

#define JEPEXCEPTION "jep/JepException"

#define THROW_JEP(env, msg)                         \
{                                                   \
    jclass clazz = (*env)->FindClass(env,           \
                                     JEPEXCEPTION); \
    if(clazz)                                       \
        (*env)->ThrowNew(env, clazz, msg);          \
}

#define THROW_JEP_EXC(env, jepExc) { (*env)->Throw(env, jepExc); }

// does the same thing as the function version, but
// restores thread blocking first
#define PROCESS_JAVA_EXCEPTION(env)             \
{                                               \
    if((*env)->ExceptionCheck(env)) {           \
        Py_BLOCK_THREADS;                       \
        process_java_exception(env);            \
        Py_UNBLOCK_THREADS;                     \
        goto EXIT_ERROR;                        \
    }                                           \
}


#ifdef WIN32
#define FILE_SEP               '\\'
#else
#define FILE_SEP               '/'
#endif

// was added in python 2.2
#ifndef PyObject_TypeCheck
# define PyObject_TypeCheck(ob, tp) (Py_TYPE(ob) == (tp))
#endif

// added in python 2.3
#ifndef PyDoc_STR
# define PyDoc_VAR(name)         static char name[]
# define PyDoc_STR(str)          (str)
# define PyDoc_STRVAR(name, str) PyDoc_VAR(name) = PyDoc_STR(str)
#endif

// this function exists solely to support python 3.2
char* pyunicode_to_utf8(PyObject *unicode);

// 3.3 reworked unicode so we have special handling for 3.2
#if PY_MAJOR_VERSION >= 3
 #if PY_MINOR_VERSION <= 2
  #define PyString_AsString(str)            pyunicode_to_utf8(str)
  #define PyString_AS_STRING(str)           pyunicode_to_utf8(str)
  #define PyString_Size(str)                PyUnicode_GetSize(str)
  #define PyString_GET_SIZE(str)            PyUnicode_GET_SIZE(str)
 #else
  #define PyString_AsString(str)            PyUnicode_AsUTF8(str)
  #define PyString_AS_STRING(str)           PyUnicode_AsUTF8(str)
  #define PyString_Size(str)                PyUnicode_GetLength(str)
  #define PyString_GET_SIZE(str)            PyUnicode_GET_LENGTH(str)
 #endif
#endif

// call toString() on jobject, make a python string and return
// sets error conditions as needed.
// returns new reference to PyObject
PyObject* jobject_topystring(JNIEnv*, jobject, jclass);

PyObject* pystring_split_last(PyObject*, char*);

// call toString() on jobject and return result.
// NULL on error
jstring jobject_tostring(JNIEnv*, jobject, jclass);

// get a const char* string from java string.
// you *must* call release when you're finished with it.
// returns local reference.
const char* jstring2char(JNIEnv*, jstring);

// release memory allocated by jstring2char
void release_utf_char(JNIEnv*, jstring, const char*);

// convert pyerr to java exception.
// int param is printTrace, send traceback to stderr
int process_py_exception(JNIEnv*, int);

// convert java exception to pyerr.
// true (1) if an exception was processed.
int process_java_exception(JNIEnv*);

// convert java exception to ImportError.
// true (1) if an exception was processed.
int process_import_exception(JNIEnv *env);

// sets up J<BLAH>TYPE
int cache_primitive_classes(JNIEnv*);
void unref_cache_primitive_classes(JNIEnv*);
int cache_frequent_classes(JNIEnv*);
void unref_cache_frequent_classes(JNIEnv*);

int get_jtype(JNIEnv*, jclass);
int pyarg_matches_jtype(JNIEnv*, PyObject*, jclass, int);
PyObject* convert_jobject(JNIEnv*, jobject, int);
PyObject* convert_jobject_pyobject(JNIEnv*, jobject);
jvalue convert_pyarg_jvalue(JNIEnv*, PyObject*, jclass, int, int);

PyObject* tuplelist_getitem(PyObject*, PyObject*);

#if USE_NUMPY
int npy_array_check(PyObject*);
int jndarray_check(JNIEnv*, jobject);
jobject convert_pyndarray_jndarray(JNIEnv*, PyObject*);
PyObject* convert_jndarray_pyndarray(JNIEnv*, jobject);
#endif

#define JBOOLEAN_ID 0
#define JINT_ID     1
#define JLONG_ID    2
#define JOBJECT_ID  3
#define JSTRING_ID  4
#define JVOID_ID    5
#define JDOUBLE_ID  6
#define JSHORT_ID   7
#define JFLOAT_ID   8
#define JARRAY_ID   9
#define JCHAR_ID    10
#define JBYTE_ID    11
#define JCLASS_ID   12

extern jclass JINT_TYPE;
extern jclass JLONG_TYPE;
extern jclass JOBJECT_TYPE;
extern jclass JSTRING_TYPE;
extern jclass JBOOLEAN_TYPE;
extern jclass JVOID_TYPE;
extern jclass JDOUBLE_TYPE;
extern jclass JSHORT_TYPE;
extern jclass JFLOAT_TYPE;
extern jclass JCHAR_TYPE;
extern jclass JBYTE_TYPE;
extern jclass JCLASS_TYPE;

#if USE_NUMPY
extern jclass JBOOLEAN_ARRAY_TYPE;
extern jclass JBYTE_ARRAY_TYPE;
extern jclass JSHORT_ARRAY_TYPE;
extern jclass JINT_ARRAY_TYPE;
extern jclass JLONG_ARRAY_TYPE;
extern jclass JFLOAT_ARRAY_TYPE;
extern jclass JDOUBLE_ARRAY_TYPE;
#endif

// cache some frequently looked up classes
extern jclass JLIST_TYPE;
extern jclass JMAP_TYPE;
extern jclass JITERABLE_TYPE;
extern jclass JITERATOR_TYPE;
extern jclass JCOLLECTION_TYPE;
#if USE_NUMPY
extern jclass JEP_NDARRAY_TYPE;
#endif

#endif // ifndef _Included_util
