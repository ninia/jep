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

static jmethodID unmodifiableList = 0;

jobject java_util_Collections_unmodifiableList(JNIEnv* env, jobject list)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (unmodifiableList
            || (unmodifiableList = (*env)->GetStaticMethodID(env, JCOLLECTIONS_TYPE,
                                   "unmodifiableList", "(Ljava/util/List;)Ljava/util/List;"))) {
        result = (*env)->CallStaticObjectMethod(env, JCOLLECTIONS_TYPE,
                                                unmodifiableList, list);
    }
    Py_END_ALLOW_THREADS
    return result;
}
