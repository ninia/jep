/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/* 
   jep - Java Embedded Python

   Copyright (C) 2004 Mike Johnson

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
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

#define JEPEXCEPTION "jep/JepException"

#define THROW_JEP(env, msg)                         \
{                                                   \
    jclass clazz = (*env)->FindClass(env,           \
                                     JEPEXCEPTION); \
    if(clazz)                                       \
        (*env)->ThrowNew(env, clazz, msg);          \
}

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
typedef __int64 jeplong;
#else
typedef long long jeplong;
#endif

// was added in python 2.2
#ifndef PyObject_TypeCheck
# define PyObject_TypeCheck(ob, tp) ((ob)->ob_type == (tp))
#endif

// added in python 2.3
#ifndef PyDoc_STR
# define PyDoc_VAR(name)         static char name[]
# define PyDoc_STR(str)          (str)
# define PyDoc_STRVAR(name, str) PyDoc_VAR(name) = PyDoc_STR(str)
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

int get_jtype(JNIEnv*, jobject, jclass);
int pyarg_matches_jtype(JNIEnv*, PyObject*, jclass, int);
PyObject* convert_jobject(JNIEnv*, jobject, int);
jvalue convert_pyarg_jvalue(JNIEnv*, PyObject*, jclass, int, int);

PyObject* tuplelist_getitem(PyObject*, PyObject*);

int register_exceptions(JNIEnv*, jobject, jobject, jobjectArray);

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

#endif // ifndef _Included_util
