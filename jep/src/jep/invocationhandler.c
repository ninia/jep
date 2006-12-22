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
 jintArray _types,
 jint returnType) {

    PyThreadState *prevThread;
    PyThreadState *_save;
    JepThread     *jepThread;
    jobject        ret;
    int            iarg, arglen;
    jint          *types;       /* pinned primitive array */
    jboolean       isCopy;
    const char    *cname;
    PyObject      *target;
    PyObject      *pyargs;      /* a tuple */
    PyObject      *callable;
    PyObject      *pyret;

    target   = (PyObject *) (intptr_t) _target;
    types    = NULL;
    ret      = NULL;
    callable = NULL;
    pyret    = NULL;

    jepThread = (JepThread *) (intptr_t) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return 0;
    }

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);

    // pin primitive array so we can get to it
    types = (*env)->GetIntArrayElements(env, _types, &isCopy);

    // first thing to do, convert java arguments to a python tuple
    arglen = (*env)->GetArrayLength(env, args);
    pyargs = PyTuple_New(arglen);
    for(iarg = 0; iarg < arglen; iarg++) {
        jobject   val;
        int       typeid;
        PyObject *pyval;

        val = (*env)->GetObjectArrayElement(env, args, iarg);
        if((*env)->ExceptionCheck(env)) /* careful, NULL is okay */
            goto EXIT;

        typeid = (int) types[iarg];

        // now we know the type, convert and add to pyargs.  we know
        // typeid is never going to be a primitive, don't bother
        // checking those. they'll end up as a pyjobject.

        switch(typeid) {
        case -1:
            // null
            Py_INCREF(Py_None);
            pyval = Py_None;
            break;

        case JARRAY_ID:
            pyval = (PyObject *) pyjarray_new(env, val);
            break;

        case JOBJECT_ID:
            pyval = (PyObject *) pyjobject_new(env, val);
            break;

        case JSTRING_ID: {
            const char *str;

            str = jstring2char(env, val);
            pyval = PyString_FromString(str);
            release_utf_char(env, val, str);
            break;
        }

        case JCLASS_ID:
            pyval = (PyObject *) pyjobject_new_class(env, val);
            break;

        default:
            THROW_JEP(env, "invoke: Unhandled typeid");
            goto EXIT;
            
        } // switch(typeid)

        PyTuple_SET_ITEM(pyargs, iarg, pyval); /* steals */
        if(val)
            (*env)->DeleteLocalRef(env, val);
    } // for(iarg = 0; iarg < arglen; iarg++)

    // now get the callable object
    cname = jstring2char(env, jname);
    // python docs say this returns a new ref. they lie like dogs.
    callable = PyObject_GetAttrString(target, (char *) cname);
    release_utf_char(env, jname, cname);

    if(process_py_exception(env, 0) || !callable)
        goto EXIT;

    pyret = PyObject_CallObject(callable, pyargs);
    if(process_py_exception(env, 0) || !pyret)
        goto EXIT;

    // handles errors
    ret = pyembed_box_py(env, pyret);

EXIT:
    if(pyargs)
        Py_DECREF(pyargs);
    if(pyret)
        Py_DECREF(pyret);

    if(types) {
        (*env)->ReleaseIntArrayElements(env,
                                        _types,
                                        types,
                                        JNI_ABORT);

        (*env)->DeleteLocalRef(env, _types);
    }
        

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();

    return ret;
}
