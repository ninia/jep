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

static jmethodID containsKey = 0;
static jmethodID get         = 0;
static jmethodID keySet      = 0;
static jmethodID put         = 0;
static jmethodID map_remove  = 0;
static jmethodID size        = 0;

jboolean java_util_Map_containsKey(JNIEnv* env, jobject this, jobject key)
{
    jboolean result = JNI_FALSE;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(containsKey, env, JMAP_TYPE, "containsKey",
                   "(Ljava/lang/Object;)Z")) {
        result = (*env)->CallBooleanMethod(env, this, containsKey, key);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject java_util_Map_get(JNIEnv* env, jobject this, jobject key)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(get, env, JMAP_TYPE, "get",
                   "(Ljava/lang/Object;)Ljava/lang/Object;")) {
        result = (*env)->CallObjectMethod(env, this, get, key);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject java_util_Map_keySet(JNIEnv* env, jobject this)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(keySet, env, JMAP_TYPE, "keySet", "()Ljava/util/Set;")) {
        result = (*env)->CallObjectMethod(env, this, keySet);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject java_util_Map_put(JNIEnv* env, jobject this, jobject key, jobject value)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(put, env, JMAP_TYPE, "put",
                   "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;")) {
        result = (*env)->CallObjectMethod(env, this, put, key, value);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject java_util_Map_remove(JNIEnv* env, jobject this, jobject key)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(map_remove, env, JMAP_TYPE, "remove",
                   "(Ljava/lang/Object;)Ljava/lang/Object;")) {
        result = (*env)->CallObjectMethod(env, this, map_remove, key);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jint java_util_Map_size(JNIEnv* env, jobject this)
{
    jint result = 0;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(size, env, JMAP_TYPE, "size", "()I")) {
        result = (*env)->CallIntMethod(env, this, size);
    }
    Py_END_ALLOW_THREADS
    return result;
}
