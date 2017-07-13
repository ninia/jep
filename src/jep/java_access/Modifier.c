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

static jmethodID isPublic   = 0;
static jmethodID isStatic   = 0;
static jmethodID isAbstract = 0;

jboolean java_lang_reflect_Modifier_isPublic(JNIEnv* env, jint mod)
{
    jboolean result = JNI_FALSE;
    if (isPublic
            || (isPublic = (*env)->GetStaticMethodID(env, JMODIFIER_TYPE, "isPublic",
                           "(I)Z"))) {
        result = (*env)->CallStaticBooleanMethod(env, JMODIFIER_TYPE, isPublic, mod);
    }
    return result;
}

jboolean java_lang_reflect_Modifier_isStatic(JNIEnv* env, jint mod)
{
    jboolean result = JNI_FALSE;
    if (isStatic
            || (isStatic = (*env)->GetStaticMethodID(env, JMODIFIER_TYPE, "isStatic",
                           "(I)Z"))) {
        result = (*env)->CallStaticBooleanMethod(env, JMODIFIER_TYPE, isStatic, mod);
    }
    return result;
}

jboolean java_lang_reflect_Modifier_isAbstract(JNIEnv* env, jint mod)
{
    jboolean result = JNI_FALSE;
    if (isAbstract
            || (isAbstract = (*env)->GetStaticMethodID(env, JMODIFIER_TYPE, "isAbstract",
                           "(I)Z"))) {
        result = (*env)->CallStaticBooleanMethod(env, JMODIFIER_TYPE, isAbstract, mod);
    }
    return result;
}
