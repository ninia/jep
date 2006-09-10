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

#include "util.h"
#include "pyembed.h"


/*
 * Class:     jep_python_PyObject
 * Method:    decref
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_decref
(JNIEnv *env, jobject jobj, jlong ptr) {
    intptr_t t;
    PyObject *o = NULL;

    // silence compiler warning
    t = (intptr_t) o;
    o = (PyObject *) t;

    if(ptr == 0) {
        THROW_JEP(env, "Invalid object");
    }
    else
        Py_DECREF(o);
}


/*
 * Class:     jep_python_PyObject
 * Method:    incref
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_incref
(JNIEnv *env, jobject jobj, jlong ptr) {
    intptr_t t;
    PyObject *o = NULL;

    // silence compiler warning
    t = (intptr_t) o;
    o = (PyObject *) t;

    if(ptr == 0) {
        THROW_JEP(env, "Invalid object");
    }
    else
        Py_INCREF(o);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2Ljava_lang_Object_2
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jobject jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_object(env, tstate, module, name, jval);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2Ljava_lang_String_2
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jstring jval) {
    const char *name, *value;

    name  = jstring2char(env, jname);
    value = jstring2char(env, jval);
    pyembed_setparameter_string(env, tstate, module, name, value);
    release_utf_char(env, jname, name);
    release_utf_char(env, jval, value);
}

/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2I
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jint jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_int(env, tstate, module, name, (int) jval);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2J
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jlong jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_long(env, tstate, module, name, (jeplong) jval);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;D)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2D
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jdouble jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_double(env, tstate, module, name, (double) jval);
    release_utf_char(env, jname, name);
}

/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;F)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2F
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jfloat jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_float(env, tstate, module, name, (float) jval);
    release_utf_char(env, jname, name);
}
