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

static jmethodID getModifiers = 0;
static jmethodID getName      = 0;

jint java_lang_reflect_Member_getModifiers(JNIEnv* env, jobject this)
{
    jint result = 0;
    if (JNI_METHOD(getModifiers, env, JMEMBER_TYPE, "getModifiers", "()I")) {
        result = (*env)->CallIntMethod(env, this, getModifiers);
    }
    return result;
}

jstring java_lang_reflect_Member_getName(JNIEnv* env, jobject this)
{
    jstring result = NULL;
    if (JNI_METHOD(getName, env, JMEMBER_TYPE, "getName", "()Ljava/lang/String;")) {
        result = (jstring) (*env)->CallObjectMethod(env, this, getName);
    }
    return result;
}
