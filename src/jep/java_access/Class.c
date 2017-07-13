/*
   jep - Java Embedded Python

   Copyright (c) 2017 JEP AUTHORS.

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

static jmethodID getComponentType   = 0;
static jmethodID getConstructors    = 0;
static jmethodID getDeclaredClasses = 0;
static jmethodID getFields          = 0;
static jmethodID getMethods         = 0;
static jmethodID getModifiers       = 0;
static jmethodID getName            = 0;
static jmethodID getSimpleName      = 0;
static jmethodID isArray            = 0;
static jmethodID newInstance        = 0;
static jmethodID isInterface        = 0;

jclass java_lang_Class_getComponentType(JNIEnv* env, jclass this)
{
    jclass result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(getComponentType, env, JCLASS_TYPE, "getComponentType",
                   "()Ljava/lang/Class;")) {
        result = (jclass) (*env)->CallObjectMethod(env, this, getComponentType);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobjectArray java_lang_Class_getConstructors(JNIEnv* env, jclass this)
{
    jobjectArray result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(getConstructors, env, JCLASS_TYPE, "getConstructors",
                   "()[Ljava/lang/reflect/Constructor;")) {
        result = (jobjectArray) (*env)->CallObjectMethod(env, this, getConstructors);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobjectArray java_lang_Class_getDeclaredClasses(JNIEnv* env, jclass this)
{
    jobjectArray result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(getDeclaredClasses, env, JCLASS_TYPE, "getDeclaredClasses",
                   "()[Ljava/lang/Class;")) {
        result = (jobjectArray) (*env)->CallObjectMethod(env, this, getDeclaredClasses);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobjectArray java_lang_Class_getFields(JNIEnv* env, jclass this)
{
    jobjectArray result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(getFields, env, JCLASS_TYPE, "getFields",
                   "()[Ljava/lang/reflect/Field;")) {
        result = (jobjectArray) (*env)->CallObjectMethod(env, this, getFields);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobjectArray java_lang_Class_getMethods(JNIEnv* env, jclass this)
{
    jobjectArray result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(getMethods, env, JCLASS_TYPE, "getMethods",
                   "()[Ljava/lang/reflect/Method;")) {
        result = (jobjectArray) (*env)->CallObjectMethod(env, this, getMethods);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jint java_lang_Class_getModifiers(JNIEnv* env, jclass this)
{
    jint result = 0;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(getModifiers, env, JCLASS_TYPE, "getModifiers", "()I")) {
        result = (*env)->CallIntMethod(env, this, getModifiers);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jstring java_lang_Class_getName(JNIEnv* env, jclass this)
{
    jstring result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(getName, env, JCLASS_TYPE, "getName", "()Ljava/lang/String;")) {
        result = (jstring) (*env)->CallObjectMethod(env, this, getName);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jstring java_lang_Class_getSimpleName(JNIEnv* env, jclass this)
{
    jstring result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(getSimpleName, env, JCLASS_TYPE, "getSimpleName",
                   "()Ljava/lang/String;")) {
        result = (jstring) (*env)->CallObjectMethod(env, this, getSimpleName);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jboolean java_lang_Class_isArray(JNIEnv* env, jclass this)
{
    jboolean result = JNI_FALSE;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(isArray, env, JCLASS_TYPE, "isArray", "()Z")) {
        result = (*env)->CallBooleanMethod(env, this, isArray);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject java_lang_Class_newInstance(JNIEnv* env, jclass this)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(newInstance, env, JCLASS_TYPE, "newInstance",
                   "()Ljava/lang/Object;")) {
        result = (*env)->CallObjectMethod(env, this, newInstance);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jboolean java_lang_Class_isInterface(JNIEnv* env, jclass this)
{
    jboolean result = JNI_FALSE;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(isInterface, env, JCLASS_TYPE, "isInterface", "()Z")) {
        result = (*env)->CallBooleanMethod(env, this, isInterface);
    }
    Py_END_ALLOW_THREADS
    return result;
}
