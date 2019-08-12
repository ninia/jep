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

static jmethodID init_F = 0;

jobject java_lang_Float_new_F(JNIEnv* env, jfloat f)
{
    if (!JNI_METHOD(init_F, env, JFLOAT_OBJ_TYPE, "<init>", "(F)V")) {
        return NULL;
    }
    return (*env)->NewObject(env, JFLOAT_OBJ_TYPE, init_F, f);
}
