/*
   jep - Java Embedded Python

   Copyright (c) 2004-2025 JEP AUTHORS.

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

#include "jep_platform.h"

#ifndef _Included_pyembed
#define _Included_pyembed


#define DICT_KEY "jep"

struct __JepThread {
    PyObject      *globals;
    PyThreadState *tstate;
    JNIEnv        *env;
    jobject        classloader;
    jobject        caller;        /* Jep instance that called us. */
};
typedef struct __JepThread JepThread;

struct __JepModuleState {
    PyObject *pyJTypeCache;

    PyTypeObject *PyJType_Type;
    PyTypeObject *PyJField_Type;
    PyTypeObject *PyJMethod_Type;
    PyTypeObject *PyJConstructor_Type;
    PyTypeObject *PyJMultiMethod_Type;

    PyTypeObject *PyJObject_Type;
    PyTypeObject *PyJClass_Type;
    PyTypeObject *PyJArray_Type;

    PyTypeObject *PyJMonitor_Type;
    PyTypeObject *PyJArrayIter_Type;
};
typedef struct __JepModuleState JepModuleState;

void pyembed_startup(JNIEnv*, jboolean, jobjectArray, jint, jint, jstring, jint,
                     jint, jstring, jint, jint, jint, jint, jint);
void pyembed_shutdown(JavaVM*);
void pyembed_shared_import(JNIEnv*, jstring);

intptr_t pyembed_thread_init_sub(JNIEnv*, jobject, jobject, jboolean, jboolean, jint, jint, jint, jint, jint, jint, jint);
intptr_t pyembed_thread_init_shared(JNIEnv*, jobject, jobject, intptr_t, jboolean);
void pyembed_thread_close(JNIEnv*, intptr_t, jboolean);

void pyembed_close(void);
void pyembed_run(JNIEnv*, intptr_t, char*);
jobject pyembed_invoke_method(JNIEnv*, intptr_t, const char*, jobjectArray,
                              jobject);
jobject pyembed_invoke_method_as(JNIEnv*, intptr_t, const char*, jobjectArray,
                                 jobject, jclass);
jobject pyembed_invoke(JNIEnv*, PyObject*, jobjectArray, jobject);
jobject pyembed_invoke_as(JNIEnv*, PyObject*, jobjectArray, jobject, jclass);
void pyembed_eval(JNIEnv*, intptr_t, char*);
int pyembed_compile_string(JNIEnv*, intptr_t, char*);
void pyembed_exec(JNIEnv*, intptr_t, char*);
jobject pyembed_getvalue(JNIEnv*, intptr_t, char*, jclass);

JNIEnv* pyembed_get_env(void);
JepThread* pyembed_get_jepthread(void);
PyObject* pyembed_get_jep_module(void);
JepModuleState* pyembed_get_module_state(void);

// -------------------------------------------------- set() methods

void pyembed_setparameter_object(JNIEnv*, intptr_t, intptr_t, const char*,
                                 jobject);
#endif
