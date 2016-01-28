/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/*
   jep - Java Embedded Python

   Copyright (c) 2016 JEP AUTHORS.

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

#include "Jep.h"

#include "invocationhandler.h"


// note that this function is called by java, not python. this is
// different than most of our calls. so i don't use the util functions
// for exception processing. if there's a java exception, just return
// NULL.

/*
 * Class:     jep_InvocationHandler
 * Method:    invoke
 * Signature: (Ljava/lang/String;JJ[Ljava/lang/Object;[Ljava/lang/Class;Ljava/lang/Class;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_jep_InvocationHandler_invoke
(JNIEnv *env,
 jclass clazz,
 jstring jname,
 jlong _jepThread,
 jlong _target,
 jobjectArray args,
 jintArray types,
 jint returnType)
{

    JepThread     *jepThread;
    jobject        ret;
    const char    *cname;
    PyObject      *target;
    PyObject      *callable;

    target   = (PyObject *) (intptr_t) _target;
    ret      = NULL;
    callable = NULL;

    jepThread = (JepThread *) (intptr_t) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return NULL;
    }

    PyEval_AcquireThread(jepThread->tstate);

    // now get the callable object
    cname = jstring2char(env, jname);
    // python docs say this returns a new ref. they lie like dogs.
    callable = PyObject_GetAttrString(target, (char *) cname);
    release_utf_char(env, jname, cname);

    if (process_py_exception(env, 0) || !callable) {
        goto EXIT;
    }

    ret = pyembed_invoke(env, callable, args, types);

EXIT:
    PyEval_ReleaseThread(jepThread->tstate);

    return ret;
}
