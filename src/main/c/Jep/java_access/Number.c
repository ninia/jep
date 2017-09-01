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

static jmethodID doubleValue = 0;
static jmethodID floatValue  = 0;
static jmethodID intValue    = 0;
static jmethodID longValue   = 0;

jdouble java_lang_Number_doubleValue(JNIEnv* env, jobject this)
{
    jdouble result = 0;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(doubleValue, env, JNUMBER_TYPE, "doubleValue", "()D")) {
        result = (*env)->CallDoubleMethod(env, this, doubleValue);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jfloat java_lang_Number_floatValue(JNIEnv* env, jobject this)
{
    jfloat result = 0;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(floatValue, env, JNUMBER_TYPE, "floatValue", "()F")) {
        result = (*env)->CallFloatMethod(env, this, floatValue);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jint java_lang_Number_intValue(JNIEnv* env, jobject this)
{
    jint result = 0;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(intValue, env, JNUMBER_TYPE, "intValue", "()I")) {
        result = (*env)->CallIntMethod(env, this, intValue);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jlong java_lang_Number_longValue(JNIEnv* env, jobject this)
{
    jlong result = 0;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(longValue, env, JNUMBER_TYPE, "longValue", "()J")) {
        result = (*env)->CallLongMethod(env, this, longValue);
    }
    Py_END_ALLOW_THREADS
    return result;
}
