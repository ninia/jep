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

static jmethodID auto_close = 0;

void java_lang_AutoCloseable_close(JNIEnv* env, jobject this)
{
    Py_BEGIN_ALLOW_THREADS
    if (JNI_METHOD(auto_close, env, JAUTOCLOSEABLE_TYPE, "close", "()V")) {
        (*env)->CallVoidMethod(env, this, auto_close);
    }
    Py_END_ALLOW_THREADS
}
