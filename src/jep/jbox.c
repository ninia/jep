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

static jmethodID booleanConstructor = 0;
static jmethodID byteConstructor    = 0;
static jmethodID shortConstructor   = 0;
static jmethodID intConstructor     = 0;
static jmethodID longConstructor    = 0;
static jmethodID floatConstructor   = 0;
static jmethodID doubleConstructor  = 0;
static jmethodID charConstructor    = 0;

jobject JBox_Boolean(JNIEnv* env, jboolean z)
{
    if (!JNI_METHOD(booleanConstructor, env, JBOOL_OBJ_TYPE, "<init>", "(Z)V")) {
        process_java_exception(env);
        return NULL;
    }
    return (*env)->NewObject(env, JBOOL_OBJ_TYPE, booleanConstructor, z);
}

jobject JBox_Byte(JNIEnv* env, jbyte b)
{
    if (!JNI_METHOD(byteConstructor, env, JBYTE_OBJ_TYPE, "<init>", "(B)V")) {
        process_java_exception(env);
        return NULL;
    }
    return (*env)->NewObject(env, JBYTE_OBJ_TYPE, byteConstructor, b);
}

jobject JBox_Short(JNIEnv* env, jshort s)
{
    if (!JNI_METHOD(shortConstructor, env, JSHORT_OBJ_TYPE, "<init>", "(S)V")) {
        process_java_exception(env);
        return NULL;
    }
    return (*env)->NewObject(env, JSHORT_OBJ_TYPE, shortConstructor, s);
}

jobject JBox_Int(JNIEnv* env, jint i)
{
    if (!JNI_METHOD(intConstructor, env, JINT_OBJ_TYPE, "<init>", "(I)V")) {
        process_java_exception(env);
        return NULL;
    }
    return (*env)->NewObject(env, JINT_OBJ_TYPE, intConstructor, i);
}

jobject JBox_Long(JNIEnv* env, jlong j)
{
    if (!JNI_METHOD(longConstructor, env, JLONG_OBJ_TYPE, "<init>", "(J)V")) {
        process_java_exception(env);
        return NULL;
    }
    return (*env)->NewObject(env, JLONG_OBJ_TYPE, longConstructor, j);
}

jobject JBox_Float(JNIEnv* env, jfloat f)
{
    if (!JNI_METHOD(floatConstructor, env, JFLOAT_OBJ_TYPE, "<init>", "(F)V")) {
        process_java_exception(env);
        return NULL;
    }
    return (*env)->NewObject(env, JFLOAT_OBJ_TYPE, floatConstructor, f);
}

jobject JBox_Double(JNIEnv* env, jdouble d)
{
    if (!JNI_METHOD(doubleConstructor, env, JDOUBLE_OBJ_TYPE, "<init>", "(D)V")) {
        process_java_exception(env);
        return NULL;
    }
    return (*env)->NewObject(env, JDOUBLE_OBJ_TYPE, doubleConstructor, d);
}

jobject JBox_Char(JNIEnv* env, jchar c)
{
    if (!JNI_METHOD(charConstructor, env, JCHAR_OBJ_TYPE, "<init>", "(C)V")) {
        process_java_exception(env);
        return NULL;
    }
    return (*env)->NewObject(env, JCHAR_OBJ_TYPE, charConstructor, c);
}

