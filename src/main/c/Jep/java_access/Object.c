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

static jmethodID equals   = 0;
static jmethodID hashCode = 0;
static jmethodID toString = 0;

jboolean java_lang_Object_equals(JNIEnv* env, jobject this, jobject obj)
{
    jboolean result = JNI_FALSE;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(equals, env, JOBJECT_TYPE, "equals", "(Ljava/lang/Object;)Z")) {
        result = (*env)->CallBooleanMethod(env, this, equals, obj);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jint java_lang_Object_hashCode(JNIEnv* env, jobject this)
{
    jint result = 0;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(hashCode, env, JOBJECT_TYPE, "hashCode", "()I")) {
        result = (*env)->CallIntMethod(env, this, hashCode);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jstring java_lang_Object_toString(JNIEnv* env, jobject this)
{
    jstring result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(toString, env, JOBJECT_TYPE, "toString",
                   "()Ljava/lang/String;")) {
        result = (jstring) (*env)->CallObjectMethod(env, this, toString);
    }
    Py_END_ALLOW_THREADS
    return result;
}
