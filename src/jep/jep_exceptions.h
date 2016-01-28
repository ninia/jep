/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/*
   jep - Java Embedded Python

   Copyright (c) JEP AUTHORS.

   This file is licensed under the the zlib/libpng License.

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

/*
 * Contains functions to support exception handling in Jep.  Supports
 * both exceptions thrown by the Python interpreter and by the JVM.
 */

#include "jep_platform.h"

#ifndef _Included_jep_exceptions
#define _Included_jep_exceptions


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



// convert pyerr to java exception.
// int param is printTrace, send traceback to stderr
int process_py_exception(JNIEnv*, int);

// convert java exception to pyerr.
// true (1) if an exception was processed.
int process_java_exception(JNIEnv*);

// convert java exception to ImportError.
// true (1) if an exception was processed.
int process_import_exception(JNIEnv *env);


// exception cached types
extern jclass CLASSNOTFOUND_EXC_TYPE;
extern jclass INDEX_EXC_TYPE;
extern jclass IO_EXC_TYPE;
extern jclass CLASSCAST_EXC_TYPE;
extern jclass ILLEGALARG_EXC_TYPE;
extern jclass ARITHMETIC_EXC_TYPE;
extern jclass OUTOFMEMORY_EXC_TYPE;
extern jclass ASSERTION_EXC_TYPE;

#endif // ifndef _Included_jep_exceptions
