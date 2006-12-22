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
#include "jep.h"
#include "pyembed.h"


#ifdef WIN32
# include "winconfig.h"

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved) {
	return TRUE;
}
#endif


// -------------------------------------------------- jni functions


JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    pyembed_startup();
    return JNI_VERSION_1_2;
}


JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *vm, void *reserved) {
    pyembed_shutdown();
}


/*
 * Class:     jep_Jep
 * Method:    init
 * Signature: (Ljava/lang/ClassLoader;)I
 */
JNIEXPORT jlong JNICALL Java_jep_Jep_init
(JNIEnv *env, jclass clazz, jobject cl, jobject caller) {
    return pyembed_thread_init(env, cl, caller);
}


/*
 * Class:     jep_Jep
 * Method:    run
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_run
(JNIEnv *env, jobject obj, jlong tstate, jstring str) {
    const char *filename;

    filename = jstring2char(env, str);
    pyembed_run(env, tstate, (char *) filename);
    release_utf_char(env, str, filename);
}


/*
 * Class:     jep_Jep
 * Method:    compileString
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_jep_Jep_compileString
(JNIEnv *env, jobject obj, jlong tstate, jstring jstr) {
    const char *str;
    jint ret;

    str = jstring2char(env, jstr);
    ret = (jint) pyembed_compile_string(env, tstate, (char *) str);
    release_utf_char(env, jstr, str);
    return ret;
}


/*
 * Class:     jep_Jep
 * Method:    eval
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_eval
(JNIEnv *env, jobject obj, jlong tstate, jstring jstr) {
    const char *str;

    str = jstring2char(env, jstr);
    pyembed_eval(env, tstate, (char *) str);
    release_utf_char(env, jstr, str);
}


/*
 * Class:     jep_Jep
 * Method:    getValue
 * Signature: (ILjava/lang/String;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_jep_Jep_getValue
(JNIEnv *env, jobject obj, jlong tstate, jstring jstr) {
    const char *str;
    jobject ret;

    str = jstring2char(env, jstr);
    ret = pyembed_getvalue(env, tstate, (char *) str);
    release_utf_char(env, jstr, str);
    return ret;
}


/*
 * Class:     jep_Jep
 * Method:    createModule
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jep_Jep_createModule
(JNIEnv *env, jobject obj, jlong tstate, jstring jstr) {
    const char *str;
    jlong ret;

    str = jstring2char(env, jstr);
    ret = pyembed_create_module(env, tstate, (char *) str);
    release_utf_char(env, jstr, str);
    return ret;
}


/*
 * Class:     jep_Jep
 * Method:    setClassLoader
 * Signature: (ILjava/lang/ClassLoader;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_setClassLoader
(JNIEnv *env, jobject obj, jlong tstate, jobject cl) {
    pyembed_setloader(env, tstate, cl);
}


/*
 * Class:     jep_Jep
 * Method:    close
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_close
(JNIEnv *env, jobject obj, jlong tstate) {
    pyembed_thread_close(tstate);
}


// -------------------------------------------------- set() methods

/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (ILjava/lang/String;Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__JLjava_lang_String_2Ljava_lang_Object_2
(JNIEnv *env, jobject obj, jlong tstate, jstring jname, jobject jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_object(env, tstate, 0, name, jval);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (JLjava/lang/String;Ljava/lang/Class;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__JLjava_lang_String_2Ljava_lang_Class_2
(JNIEnv *env, jobject obj, jlong tstate, jstring jname, jclass jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_class(env, tstate, 0, name, jval);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (ILjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__JLjava_lang_String_2Ljava_lang_String_2
(JNIEnv *env, jobject obj, jlong tstate, jstring jname, jstring jval) {
    const char *name, *value;

    name  = jstring2char(env, jname);
    value = jstring2char(env, jval);
    pyembed_setparameter_string(env, tstate, 0, name, value);
    release_utf_char(env, jname, name);
    release_utf_char(env, jval, value);
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (ILjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__JLjava_lang_String_2I
(JNIEnv *env, jobject obj, jlong tstate, jstring jname, jint jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_int(env, tstate, 0, name, (int) jval);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (ILjava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__JLjava_lang_String_2J
(JNIEnv *env, jobject obj, jlong tstate, jstring jname, jlong jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_long(env, tstate, 0, name, (jeplong) jval);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (ILjava/lang/String;D)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__JLjava_lang_String_2D
(JNIEnv *env, jobject obj, jlong tstate, jstring jname, jdouble jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_double(env, tstate, 0, name, (double) jval);
    release_utf_char(env, jname, name);
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (ILjava/lang/String;F)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__JLjava_lang_String_2F
(JNIEnv *env, jobject obj, jlong tstate, jstring jname, jfloat jval) {
    const char *name;
    
    name = jstring2char(env, jname);
    pyembed_setparameter_float(env, tstate, 0, name, (float) jval);
    release_utf_char(env, jname, name);
}
