/*
   jep - Java Embedded Python

   Copyright (c) 2017-2019 JEP AUTHORS.

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

static jmethodID newProxyInstance       = 0;
static jmethodID newDirectProxyInstance = 0;
static jmethodID getPyObject            = 0;

jobject jep_Proxy_newProxyInstance(JNIEnv* env, jobject jep, jlong ltarget,
                                   jobjectArray interfaces)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (newProxyInstance
            || (newProxyInstance = (*env)->GetStaticMethodID(env, JEP_PROXY_TYPE,
                                   "newProxyInstance",
                                   "(Ljep/Jep;J[Ljava/lang/String;)Ljava/lang/Object;"))) {
        result = (*env)->CallStaticObjectMethod(env, JEP_PROXY_TYPE, newProxyInstance,
                                                jep, ltarget, interfaces);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject jep_Proxy_newDirectProxyInstance(JNIEnv* env, jobject jep,
        jlong ltarget,
        jclass targetInterface)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (newDirectProxyInstance
            || (newDirectProxyInstance = (*env)->GetStaticMethodID(env, JEP_PROXY_TYPE,
                                         "newDirectProxyInstance",
                                         "(Ljep/Jep;JLjava/lang/Class;)Ljava/lang/Object;"))) {
        result = (*env)->CallStaticObjectMethod(env, JEP_PROXY_TYPE,
                                                newDirectProxyInstance,
                                                jep, ltarget, targetInterface);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject jep_Proxy_getPyObject(JNIEnv* env, jobject object)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (getPyObject
            || (getPyObject = (*env)->GetStaticMethodID(env, JEP_PROXY_TYPE,
                              "getPyObject",
                              "(Ljava/lang/Object;)Ljep/python/PyObject;"))) {
        result = (*env)->CallStaticObjectMethod(env, JEP_PROXY_TYPE, getPyObject, object);
    }
    Py_END_ALLOW_THREADS
    return result;
}
