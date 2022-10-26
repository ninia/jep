/*
   jep - Java Embedded Python

   Copyright (c) 2004-2022 JEP AUTHORS.

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
#include "jep_Jep.h"


#ifdef WIN32
# include "winconfig.h"

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    return TRUE;
}
#endif


// -------------------------------------------------- jni functions


JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    return JNI_VERSION_1_2;
}


JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *vm, void *reserved)
{
    pyembed_shutdown(vm);
}


/*
 * Class:     jep_Jep
 * Method:    init
 * Signature: (Ljava/lang/ClassLoader;Z)J
 */
JNIEXPORT jlong JNICALL Java_jep_Jep_init
(JNIEnv *env, jobject obj, jobject cl, jboolean hasSharedModules,
 jboolean usesubinterpreter)
{
    return pyembed_thread_init(env, cl, obj, hasSharedModules, usesubinterpreter);
}


/*
 * Class:     jep_Jep
 * Method:    run
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_run
(JNIEnv *env, jobject obj, jlong tstate, jstring str)
{
    const char *filename;

    filename = jstring2char(env, str);
    pyembed_run(env, (intptr_t) tstate, (char *) filename);
    release_utf_char(env, str, filename);
}


/*
 * Class:     jep_Jep
 * Method:    invoke
 * Signature: (JLjava/lang/String;[Ljava/lang/Object;Ljava/util/Map;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_jep_Jep_invoke
(JNIEnv *env,
 jobject obj,
 jlong tstate,
 jstring name,
 jobjectArray args,
 jobject kwargs)
{
    const char *cname;
    jobject ret;

    cname = jstring2char(env, name);
    ret = pyembed_invoke_method(env, (intptr_t) tstate, cname, args, kwargs);
    release_utf_char(env, name, cname);

    return ret;
}


/*
 * Class:     jep_Jep
 * Method:    compileString
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_jep_Jep_compileString
(JNIEnv *env, jobject obj, jlong tstate, jstring jstr)
{
    const char *str;
    jint ret;

    str = jstring2char(env, jstr);
    ret = (jint) pyembed_compile_string(env, (intptr_t) tstate, (char *) str);
    release_utf_char(env, jstr, str);
    return ret;
}


/*
 * Class:     jep_Jep
 * Method:    eval
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_eval
(JNIEnv *env, jobject obj, jlong tstate, jstring jstr)
{
    const char *str;

    str = jstring2char(env, jstr);
    pyembed_eval(env, (intptr_t) tstate, (char *) str);
    release_utf_char(env, jstr, str);
}


/*
 * Class:     jep_Jep
 * Method:    eval
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_exec
(JNIEnv *env, jobject obj, jlong tstate, jstring jstr)
{
    const char *str;

    str = jstring2char(env, jstr);
    pyembed_exec(env, (intptr_t) tstate, (char *) str);
    release_utf_char(env, jstr, str);
}


/*
 * Class:     jep_Jep
 * Method:    getValue
 * Signature: (JLjava/lang/String;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_jep_Jep_getValue
(JNIEnv *env, jobject obj, jlong tstate, jstring jstr, jclass clazz)
{
    const char *str;
    jobject ret;

    str = jstring2char(env, jstr);
    ret = pyembed_getvalue(env, (intptr_t) tstate, (char *) str, clazz);
    release_utf_char(env, jstr, str);
    return ret;
}

/*
 * Class:     jep_Jep
 * Method:    close
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_close
(JNIEnv *env, jobject obj, jlong tstate)
{
    pyembed_thread_close(env, (intptr_t) tstate);
}

// -------------------------------------------------- set() methods

/*
 * Class:     jep_Jep
 * Method:    set
 * Signature: (JLjava/lang/String;Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_jep_Jep_set__JLjava_lang_String_2Ljava_lang_Object_2
(JNIEnv *env, jobject obj, jlong tstate, jstring jname, jobject jval)
{
    const char *name;

    name = jstring2char(env, jname);
    pyembed_setparameter_object(env, (intptr_t) tstate, 0, name, jval);
    release_utf_char(env, jname, name);
}
