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
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_init
(JNIEnv *env, jobject obj, jstring _hash) {
    const char *hash;
    
    hash = jstring2char(env, _hash);

    pyembed_thread_init(env, hash);

    release_utf_char(env, _hash, hash);
}


/*
 * Class:     jep_Jep
 * Method:    run
 * Signature: (Ljava/lang/String;Ljava/lang/ClassLoader;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_run
(JNIEnv *env, jobject obj, jstring _hash, jobject cl, jstring str) {
    const char *filename, *hash;

    filename = jstring2char(env, str);
    hash     = jstring2char(env, _hash);

    pyembed_run(env, hash, (char *) filename, cl);

    release_utf_char(env, str, filename);
    release_utf_char(env, _hash, hash);
}


/*
 * Class:     jep_Jep
 * Method:    eval
 * Signature: (Ljava/lang/String;Ljava/lang/ClassLoader;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_eval
(JNIEnv *env, jobject obj, jstring _hash, jobject cl, jstring jstr) {
    const char *str, *hash;

    str = jstring2char(env, jstr);
    hash     = jstring2char(env, _hash);

    pyembed_eval(env, hash, (char *) str, cl);

    release_utf_char(env, jstr, str);
    release_utf_char(env, _hash, hash);
}


/*
 * Class:     jep_Jep
 * Method:    close
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_close
(JNIEnv *env, jobject obj, jstring _hash) {
    const char *hash = jstring2char(env, _hash);
    pyembed_thread_close(hash);
    release_utf_char(env, _hash, hash);
}


// -------------------------------------------------- set() methods

/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_Object_2
(JNIEnv *env, jobject obj, jstring _hash, jstring jname, jobject jval) {
    const char *name, *hash;
    name = jstring2char(env, jname);
    hash = jstring2char(env, _hash);

    pyembed_setparameter_object(env, hash, name, jval);

    release_utf_char(env, jname, name);
    release_utf_char(env, _hash, hash);
    return;
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2
(JNIEnv *env, jobject boj, jstring _hash, jstring jname, jstring jval) {
    const char *name, *value, *hash;

    name  = jstring2char(env, jname);
    value = jstring2char(env, jval);
    hash  = jstring2char(env, _hash);

    pyembed_setparameter_string(env, hash, name, value);

    release_utf_char(env, jname, name);
    release_utf_char(env, jval, value);
    release_utf_char(env, _hash, hash);
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__Ljava_lang_String_2Ljava_lang_String_2I
(JNIEnv *env, jobject obj, jstring _hash, jstring jname, jint jval) {
    const char *name, *hash;
    
    name = jstring2char(env, jname);
    hash = jstring2char(env, _hash);

    pyembed_setparameter_int(env, hash, name, (int) jval);

    release_utf_char(env, jname, name);
    release_utf_char(env, _hash, hash);
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__Ljava_lang_String_2Ljava_lang_String_2J
(JNIEnv *env, jobject obj, jstring _hash, jstring jname, jlong jval) {
    const char *name, *hash;
    
    name = jstring2char(env, jname);
    hash = jstring2char(env, _hash);
    
    pyembed_setparameter_long(env, hash, name, (jeplong) jval);

    release_utf_char(env, jname, name);
    release_utf_char(env, _hash, hash);
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (Ljava/lang/String;Ljava/lang/String;D)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__Ljava_lang_String_2Ljava_lang_String_2D
(JNIEnv *env, jobject obj, jstring _hash, jstring jname, jdouble jval) {
    const char *name, *hash;
    
    name = jstring2char(env, jname);
    hash = jstring2char(env, _hash);
    
    pyembed_setparameter_double(env, hash, name, (double) jval);
    
    release_utf_char(env, jname, name);
    release_utf_char(env, _hash, hash);
}


/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (Ljava/lang/String;Ljava/lang/String;F)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__Ljava_lang_String_2Ljava_lang_String_2F
(JNIEnv *env, jobject obj, jstring _hash, jstring jname, jfloat jval) {
    const char *name, *hash;
    
    name = jstring2char(env, jname);
    hash = jstring2char(env, _hash);
    
    pyembed_setparameter_float(env, hash, name, (float) jval);
    
    release_utf_char(env, jname, name);
    release_utf_char(env, _hash, hash);
}
