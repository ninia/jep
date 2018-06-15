/*
   jep - Java Embedded Python

   Copyright (c) 2017-2018 JEP AUTHORS.

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


jobject jep_Proxy_newProxyInstance(JNIEnv* env, jlong tstate, jlong ltarget,
                                   jobject jep, jobject loader,
                                   jobjectArray interfaces)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (newProxyInstance || (newProxyInstance = (*env)->GetStaticMethodID(env, JPROXY_TYPE, "newProxyInstance",
                   "(JJLjep/Jep;Ljava/lang/ClassLoader;[Ljava/lang/String;)Ljava/lang/Object;"))) {
        result = (*env)->CallStaticObjectMethod(env, JPROXY_TYPE, newProxyInstance,
                                          tstate, ltarget, jep, loader, interfaces);
    }
    Py_END_ALLOW_THREADS
    return result;
}

jobject jep_Proxy_newDirectProxyInstance(JNIEnv* env, jlong tstate,
                                         jlong ltarget, jobject jep,
                                         jobject loader, jclass targetInterface)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (newDirectProxyInstance || (newDirectProxyInstance = (*env)->GetStaticMethodID(env, JPROXY_TYPE, "newDirectProxyInstance",
                   "(JJLjep/Jep;Ljava/lang/ClassLoader;Ljava/lang/Class;)Ljava/lang/Object;"))) {
        result = (*env)->CallStaticObjectMethod(env, JPROXY_TYPE, newDirectProxyInstance,
                                          tstate, ltarget, jep, loader, targetInterface);
    }
    Py_END_ALLOW_THREADS
    return result;
}

