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

#include "invocationhandler.h"

#include "util.h"
#include "pyembed.h"


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
 jint returnType) {

    PyThreadState *prevThread;
    JepThread     *jepThread;
    jobject        ret;
    const char    *cname;
    PyObject      *target;
    PyObject      *callable;

    target   = (PyObject *) (intptr_t) _target;
    ret      = NULL;
    callable = NULL;

    jepThread = (JepThread *) (intptr_t) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return NULL;
    }

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);

    // now get the callable object
    cname = jstring2char(env, jname);
    // python docs say this returns a new ref. they lie like dogs.
    callable = PyObject_GetAttrString(target, (char *) cname);
    release_utf_char(env, jname, cname);

    if(process_py_exception(env, 0) || !callable)
        goto EXIT;

    ret = pyembed_invoke(env, callable, args, types);

EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();

    return ret;
}
