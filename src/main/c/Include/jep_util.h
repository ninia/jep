/*
   jep - Java Embedded Python

   Copyright (c) 2004-2019 JEP AUTHORS.

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
jvalue convert_pyarg_jvalue(JNIEnv*, PyObject*, jclass, int, int);

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
extern jclass JBOOLEAN_TYPE;
extern jclass JVOID_TYPE;
extern jclass JDOUBLE_TYPE;
extern jclass JSHORT_TYPE;
extern jclass JFLOAT_TYPE;
extern jclass JCHAR_TYPE;
extern jclass JBYTE_TYPE;

// cache primitive array types
extern jclass JBOOLEAN_ARRAY_TYPE;
extern jclass JBYTE_ARRAY_TYPE;
extern jclass JCHAR_ARRAY_TYPE;
extern jclass JSHORT_ARRAY_TYPE;
extern jclass JINT_ARRAY_TYPE;
extern jclass JLONG_ARRAY_TYPE;
extern jclass JFLOAT_ARRAY_TYPE;
extern jclass JDOUBLE_ARRAY_TYPE;

/*
 * CLASS_TABLE contains the definition for all the classes that we cache for
 * easy access. Tyically a macro is passed to CLASS_TABLE macro and it will be
 * evaluated for all cached classes. This is based off the X macro pattern
 * except a macro is passed in rather than redifining X for each use.
 */
#define CLASS_TABLE(F) \
    F(JOBJECT_TYPE, "java/lang/Object") \
    F(JSTRING_TYPE, "java/lang/String") \
    F(JCLASS_TYPE, "java/lang/Class") \
    F(JLIST_TYPE, "java/util/List") \
    F(JMAP_TYPE, "java/util/Map") \
    F(JENTRY_TYPE, "java/util/Map$Entry") \
    F(JITERABLE_TYPE, "java/lang/Iterable") \
    F(JITERATOR_TYPE, "java/util/Iterator") \
    F(JCOLLECTION_TYPE, "java/util/Collection") \
    F(JCOMPARABLE_TYPE, "java/lang/Comparable") \
    F(JAUTOCLOSEABLE_TYPE, "java/lang/AutoCloseable") \
    F(JBOOL_OBJ_TYPE, "java/lang/Boolean") \
    F(JBYTEBUFFER_TYPE, "java/nio/ByteBuffer") \
    F(JBYTE_OBJ_TYPE, "java/lang/Byte") \
    F(JBYTEORDER_TYPE, "java/nio/ByteOrder") \
    F(JBUFFER_TYPE, "java/nio/Buffer") \
    F(JSHORT_OBJ_TYPE, "java/lang/Short") \
    F(JSHORTBUFFER_TYPE, "java/nio/ShortBuffer") \
    F(JINT_OBJ_TYPE, "java/lang/Integer") \
    F(JINTBUFFER_TYPE, "java/nio/IntBuffer") \
    F(JLONG_OBJ_TYPE, "java/lang/Long") \
    F(JLONGBUFFER_TYPE, "java/nio/LongBuffer") \
    F(JDOUBLE_OBJ_TYPE, "java/lang/Double") \
    F(JDOUBLEBUFFER_TYPE, "java/nio/DoubleBuffer") \
    F(JFLOAT_OBJ_TYPE, "java/lang/Float") \
    F(JFLOATBUFFER_TYPE, "java/nio/FloatBuffer") \
    F(JCHAR_OBJ_TYPE, "java/lang/Character") \
    F(JCHARBUFFER_TYPE, "java/nio/CharBuffer") \
    F(JNUMBER_TYPE, "java/lang/Number") \
    F(JMEMBER_TYPE, "java/lang/reflect/Member") \
    F(JMETHOD_TYPE, "java/lang/reflect/Method") \
    F(JFIELD_TYPE, "java/lang/reflect/Field") \
    F(JAVA_PROXY_TYPE, "java/lang/reflect/Proxy") \
    F(JCONSTRUCTOR_TYPE, "java/lang/reflect/Constructor") \
    F(JTHROWABLE_TYPE, "java/lang/Throwable") \
    F(JMODIFIER_TYPE, "java/lang/reflect/Modifier") \
    F(JARRAYLIST_TYPE, "java/util/ArrayList") \
    F(JHASHMAP_TYPE, "java/util/HashMap") \
    F(JCOLLECTIONS_TYPE, "java/util/Collections") \
    F(JCLASSLOADER_TYPE, "java/lang/ClassLoader") \
    F(JEP_PROXY_TYPE, "jep/Proxy") \
    F(CLASSNOTFOUND_EXC_TYPE, "java/lang/ClassNotFoundException") \
    F(INDEX_EXC_TYPE, "java/lang/IndexOutOfBoundsException") \
    F(IO_EXC_TYPE, "java/io/IOException") \
    F(CLASSCAST_EXC_TYPE, "java/lang/ClassCastException") \
    F(ILLEGALARG_EXC_TYPE, "java/lang/IllegalArgumentException") \
    F(ARITHMETIC_EXC_TYPE, "java/lang/ArithmeticException") \
    F(OUTOFMEMORY_EXC_TYPE, "java/lang/OutOfMemoryError") \
    F(ASSERTION_EXC_TYPE, "java/lang/AssertionError") \
    F(JEP_EXC_TYPE, "jep/JepException") \
    F(JPYOBJECT_TYPE, "jep/python/PyObject") \
    F(JPYCALLABLE_TYPE, "jep/python/PyCallable") \
    NUMPY_CLASS_TABLE(F)

#if JEP_NUMPY_ENABLED
#define NUMPY_CLASS_TABLE(F) \
    F(JEP_NDARRAY_TYPE, "jep/NDArray") \
    F(JEP_DNDARRAY_TYPE, "jep/DirectNDArray")
#else
#define NUMPY_CLASS_TABLE(F)
#endif

// Define an extern variable for everything in the class table
#define DEFINE_CLASS_GLOBAL(var, name) extern jclass var;
CLASS_TABLE(DEFINE_CLASS_GLOBAL)

#endif // ifndef _Included_jep_util
