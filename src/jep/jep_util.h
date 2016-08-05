/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/*
   jep - Java Embedded Python

   Copyright (c) JEP AUTHORS.

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

/*
 * Contains convenient utility functions for Jep.
 */

#include "jep_platform.h"

#ifndef _Included_jep_util
#define _Included_jep_util

/*
 * A wrapper around the JNI method GetMethodID which will cache the jmethodID
 * to avoid repeated lookups. The first argument should be a variable for
 * saving the jmethodID and the remaining arguments match the signature of
 * GetMethodID. This macro "returns" 1 if the method is already cached or if
 * the lookup succeeds and 0 if the lookup fails.
 */
#define JNI_METHOD(var, env, type, name, sig)\
    (var || (var = (*env)->GetMethodID(env, type, name, sig)))

// this function exists solely to support python 3.2
char* pyunicode_to_utf8(PyObject *unicode);

// 3.3 reworked unicode so we have special handling for 3.2
#if PY_MAJOR_VERSION >= 3
    #if PY_MINOR_VERSION <= 2
        #define PyString_AsString(str)            pyunicode_to_utf8(str)
        #define PyString_AS_STRING(str)           pyunicode_to_utf8(str)
        #define PyString_Size(str)                PyUnicode_GetSize(str)
        #define PyString_GET_SIZE(str)            PyUnicode_GET_SIZE(str)
    #endif
#endif

// call toString() on jobject, make a python string and return
// sets error conditions as needed.
// returns new reference to PyObject
PyObject* jobject_topystring(JNIEnv*, jobject);

// call toString() on jobject and return result.
// NULL on error
jstring jobject_tostring(JNIEnv*, jobject);

// get a const char* string from java string.
// you *must* call release when you're finished with it.
// returns local reference.
const char* jstring2char(JNIEnv*, jstring);

// release memory allocated by jstring2char
void release_utf_char(JNIEnv*, jstring, const char*);

// sets up J<BLAH>TYPE
int cache_primitive_classes(JNIEnv*);
void unref_cache_primitive_classes(JNIEnv*);
int cache_frequent_classes(JNIEnv*);
void unref_cache_frequent_classes(JNIEnv*);

int get_jtype(JNIEnv*, jclass);
int pyarg_matches_jtype(JNIEnv*, PyObject*, jclass, int);
PyObject* convert_jobject(JNIEnv*, jobject, int);
PyObject* convert_jobject_pyobject(JNIEnv*, jobject);
jvalue convert_pyarg_jvalue(JNIEnv*, PyObject*, jclass, int, int);

PyObject* tuplelist_getitem(PyObject*, PyObject*);


#define JBOOLEAN_ID 0
#define JINT_ID     1
#define JLONG_ID    2
#define JOBJECT_ID  3
#define JSTRING_ID  4
#define JVOID_ID    5
#define JDOUBLE_ID  6
#define JSHORT_ID   7
#define JFLOAT_ID   8
#define JARRAY_ID   9
#define JCHAR_ID    10
#define JBYTE_ID    11
#define JCLASS_ID   12

extern jclass JINT_TYPE;
extern jclass JLONG_TYPE;
extern jclass JOBJECT_TYPE;
extern jclass JSTRING_TYPE;
extern jclass JBOOLEAN_TYPE;
extern jclass JVOID_TYPE;
extern jclass JDOUBLE_TYPE;
extern jclass JSHORT_TYPE;
extern jclass JFLOAT_TYPE;
extern jclass JCHAR_TYPE;
extern jclass JBYTE_TYPE;
extern jclass JCLASS_TYPE;

// cache primitive array types
extern jclass JBOOLEAN_ARRAY_TYPE;
extern jclass JBYTE_ARRAY_TYPE;
extern jclass JCHAR_ARRAY_TYPE;
extern jclass JSHORT_ARRAY_TYPE;
extern jclass JINT_ARRAY_TYPE;
extern jclass JLONG_ARRAY_TYPE;
extern jclass JFLOAT_ARRAY_TYPE;
extern jclass JDOUBLE_ARRAY_TYPE;

// cache some frequently looked up interfaces
extern jclass JLIST_TYPE;
extern jclass JMAP_TYPE;
extern jclass JITERABLE_TYPE;
extern jclass JITERATOR_TYPE;
extern jclass JCOLLECTION_TYPE;
extern jclass JCOMPARABLE_TYPE;

// cache some frequently looked up classes
extern jclass JBOOL_OBJ_TYPE;
extern jclass JBYTE_OBJ_TYPE;
extern jclass JSHORT_OBJ_TYPE;
extern jclass JINT_OBJ_TYPE;
extern jclass JLONG_OBJ_TYPE;
extern jclass JFLOAT_OBJ_TYPE;
extern jclass JDOUBLE_OBJ_TYPE;
extern jclass JCHAR_OBJ_TYPE;

extern jclass JMETHOD_TYPE;
extern jclass JFIELD_TYPE;
extern jclass JNUMBER_TYPE;
extern jclass JTHROWABLE_TYPE;
extern jclass JMODIFIER_TYPE;
extern jclass JARRAYLIST_TYPE;
extern jclass JHASHMAP_TYPE;
extern jclass JCOLLECTIONS_TYPE;

// cache frequently used method
extern jmethodID JCLASS_GET_NAME;
#endif // ifndef _Included_jep_util
