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


/*
 * Class:     jep_python_PyObject
 * Method:    decref
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_decref
(JNIEnv *env, jobject jobj, jlong ptr) {
    PyObject *o = (PyObject *) (intptr_t) jobj;

    if(ptr == 0) {
        THROW_JEP(env, "jep_object: Invalid object");
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
    PyObject *o = (PyObject *) (intptr_t) jobj;

    if(ptr == 0) {
        THROW_JEP(env, "jep_object: Invalid object");
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
    pyembed_setparameter_object(env, (intptr_t) tstate, (intptr_t) module, name, jval);
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
    pyembed_setparameter_string(env, (intptr_t) tstate, (intptr_t) module, name, value);
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
    pyembed_setparameter_int(env, (intptr_t) tstate, (intptr_t) module, name, (int) jval);
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
    pyembed_setparameter_long(env, (intptr_t) tstate, (intptr_t) module, name, (PY_LONG_LONG) jval);
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
    pyembed_setparameter_double(env, (intptr_t) tstate, (intptr_t) module, name, (double) jval);
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
    pyembed_setparameter_float(env, (intptr_t) tstate, (intptr_t) module, name, (float) jval);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;[Z)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2_3Z
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jbooleanArray jarr) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_array(env, (intptr_t) tstate, (intptr_t) module, name, (jobjectArray) jarr);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;[I)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2_3I
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jintArray jarr) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_array(env, (intptr_t) tstate, (intptr_t) module, name, (jobjectArray) jarr);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;[S)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2_3S
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jshortArray jarr) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_array(env, (intptr_t) tstate, (intptr_t) module, name, (jobjectArray) jarr);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;[B)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2_3B
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jbyteArray jarr) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_array(env, (intptr_t) tstate, (intptr_t) module, name, (jobjectArray) jarr);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;[J)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2_3J
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jlongArray jarr) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_array(env, (intptr_t) tstate, (intptr_t) module, name, (jobjectArray) jarr);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;[D)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2_3D
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jdoubleArray jarr) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_array(env, (intptr_t) tstate, (intptr_t) module, name, (jobjectArray) jarr);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    set
 * Signature: (JJLjava/lang/String;[F)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_set__JJLjava_lang_String_2_3F
(JNIEnv *env, jobject obj, jlong tstate, jlong module,
 jstring jname, jfloatArray jarr) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_array(env, (intptr_t) tstate, (intptr_t) module, name, (jobjectArray) jarr);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_python_PyObject
 * Method:    createModule
 * Signature: (JJLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jep_python_PyObject_createModule
(JNIEnv *env, jobject obj, jlong tstate, jlong module, jstring jstr) {
    const char *str;
    jlong ret;

    str = jstring2char(env, jstr);
    ret = pyembed_create_module_on(env, (intptr_t) tstate, (intptr_t) module, (char *) str);
    release_utf_char(env, jstr, str);
    return ret;
}


/*
 * Class:     jep_python_PyObject
 * Method:    getValue
 * Signature: (JJLjava/lang/String;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_jep_python_PyObject_getValue
(JNIEnv *env, jobject obj, jlong tstate, jlong onModule, jstring jstr) {
    const char *str;
    jobject ret;

    str = jstring2char(env, jstr);
    ret = pyembed_getvalue_on(env, (intptr_t) tstate, (intptr_t) onModule, (char *) str);
    release_utf_char(env, jstr, str);
    return ret;
}
