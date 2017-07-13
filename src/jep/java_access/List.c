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

static jmethodID add         = 0;
static jmethodID addAll      = 0;
static jmethodID clear       = 0;
static jmethodID get         = 0;
static jmethodID list_remove = 0;
static jmethodID set         = 0;
static jmethodID subList     = 0;

jboolean java_util_List_add(JNIEnv* env, jobject this, jobject e)
{
    jboolean result = JNI_FALSE;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(add, env, JLIST_TYPE, "add", "(Ljava/lang/Object;)Z")) {
        result = (*env)->CallBooleanMethod(env, this, add, e);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jboolean java_util_List_addAll(JNIEnv* env, jobject this, jobject c)
{
    jboolean result = JNI_FALSE;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(addAll, env, JLIST_TYPE, "addAll",
                   "(Ljava/util/Collection;)Z")) {
        result = (*env)->CallBooleanMethod(env, this, addAll, c);
    }
    Py_END_ALLOW_THREADS
    return result;
}

void java_util_List_clear(JNIEnv* env, jobject this)
{
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(get, env, JLIST_TYPE, "clear", "()V")) {
        (*env)->CallVoidMethod(env, this, clear);
    }
    Py_END_ALLOW_THREADS
}

jobject java_util_List_get(JNIEnv* env, jobject this, jint index)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(get, env, JLIST_TYPE, "get", "(I)Ljava/lang/Object;")) {
        result = (*env)->CallObjectMethod(env, this, get, index);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject java_util_List_remove(JNIEnv* env, jobject this, jint index)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(list_remove, env, JLIST_TYPE, "remove",
                   "(I)Ljava/lang/Object;")) {
        result = (*env)->CallObjectMethod(env, this, list_remove, index);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject java_util_List_set(JNIEnv* env, jobject this, jint index,
                           jobject element)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(set, env, JLIST_TYPE, "set",
                   "(ILjava/lang/Object;)Ljava/lang/Object;")) {
        result = (*env)->CallObjectMethod(env, this, set, index, element);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject java_util_List_subList(JNIEnv* env, jobject this, jint fromIndex,
                               jint toIndex)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(subList, env, JLIST_TYPE, "subList", "(II)Ljava/util/List;")) {
        result = (*env)->CallObjectMethod(env, this, subList, fromIndex, toIndex);
    }
    Py_END_ALLOW_THREADS
    return result;
}
