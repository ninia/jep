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

static jmethodID init_J_J_Jep = 0;


jobject jep_python_PyCallable_new_J_J_Jep(JNIEnv* env, jlong tstate, jlong pyObject, jobject jep)
{
    jobject result = NULL;
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(init_J_J_Jep, env, JPYCALLABLE_TYPE, "<init>", "(JJLjep/Jep;)V")) {
        result = (*env)->NewObject(env, JPYCALLABLE_TYPE, init_J_J_Jep, tstate, pyObject, jep);
    }
    Py_END_ALLOW_THREADS

    return result;
}

