/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/*
   jep - Java Embedded Python

   Copyright (c) 2016 JEP AUTHORS.

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


/*
 * Caching of jclass and jmethodID objects for optimal performance.  These
 * are shared for all threads and should be considered constants.
 */

// primitive class types
jclass JINT_TYPE     = NULL;
jclass JLONG_TYPE    = NULL;
jclass JOBJECT_TYPE  = NULL;
jclass JSTRING_TYPE  = NULL;
jclass JBOOLEAN_TYPE = NULL;
jclass JVOID_TYPE    = NULL;
jclass JDOUBLE_TYPE  = NULL;
jclass JSHORT_TYPE   = NULL;
jclass JFLOAT_TYPE   = NULL;
jclass JCHAR_TYPE    = NULL;
jclass JBYTE_TYPE    = NULL;
jclass JCLASS_TYPE   = NULL;

// cached types for primitive arrays
jclass JBOOLEAN_ARRAY_TYPE = NULL;
jclass JBYTE_ARRAY_TYPE    = NULL;
jclass JCHAR_ARRAY_TYPE   = NULL;
jclass JSHORT_ARRAY_TYPE   = NULL;
jclass JINT_ARRAY_TYPE     = NULL;
jclass JLONG_ARRAY_TYPE    = NULL;
jclass JFLOAT_ARRAY_TYPE   = NULL;
jclass JDOUBLE_ARRAY_TYPE  = NULL;

// cached types for interfaces
jclass JLIST_TYPE       = NULL;
jclass JMAP_TYPE        = NULL;
jclass JITERABLE_TYPE   = NULL;
jclass JITERATOR_TYPE   = NULL;
jclass JCOLLECTION_TYPE = NULL;
jclass JCOMPARABLE_TYPE = NULL;

// cached types for Object equivalents of primitives
jclass JBOOL_OBJ_TYPE   = NULL;
jclass JBYTE_OBJ_TYPE   = NULL;
jclass JSHORT_OBJ_TYPE  = NULL;
jclass JINT_OBJ_TYPE    = NULL;
jclass JLONG_OBJ_TYPE   = NULL;
jclass JDOUBLE_OBJ_TYPE = NULL;
jclass JFLOAT_OBJ_TYPE  = NULL;
jclass JCHAR_OBJ_TYPE   = NULL;

// cached types for frequently used classes
jclass JNUMBER_TYPE      = NULL;
jclass JMETHOD_TYPE      = NULL;
jclass JFIELD_TYPE       = NULL;
jclass JTHROWABLE_TYPE   = NULL;
jclass JMODIFIER_TYPE    = NULL;
jclass JARRAYLIST_TYPE   = NULL;
jclass JHASHMAP_TYPE     = NULL;
jclass JCOLLECTIONS_TYPE = NULL;
#if JEP_NUMPY_ENABLED
    jclass JEP_NDARRAY_TYPE = NULL;
#endif

// exception cached types
jclass CLASSNOTFOUND_EXC_TYPE;
jclass INDEX_EXC_TYPE;
jclass IO_EXC_TYPE;
jclass CLASSCAST_EXC_TYPE;
jclass ILLEGALARG_EXC_TYPE;
jclass ARITHMETIC_EXC_TYPE;
jclass OUTOFMEMORY_EXC_TYPE;
jclass ASSERTION_EXC_TYPE;

// cached java.lang.Class.getName()
jmethodID JCLASS_GET_NAME;

// cached methodids
jmethodID objectToString        = 0;
jmethodID objectIsArray         = 0;
jmethodID classGetComponentType = 0;

// for convert_jobject
jmethodID getBooleanValue    = 0;
jmethodID getIntValue        = 0;
jmethodID getLongValue       = 0;
jmethodID getDoubleValue     = 0;
jmethodID getFloatValue      = 0;
jmethodID getCharValue       = 0;


// call toString() on jobject, make a python string and return
// sets error conditions as needed.
// returns new reference to PyObject
PyObject* jobject_topystring(JNIEnv *env, jobject obj)
{
    const char *result;
    PyObject   *pyres;
    jstring     jstr;

    jstr = jobject_tostring(env, obj);
    // it's possible, i guess. don't throw an error....
    if (process_java_exception(env) || jstr == NULL) {
        return PyString_FromString("");
    }

    result = (*env)->GetStringUTFChars(env, jstr, 0);
    pyres  = PyString_FromString(result);
    (*env)->ReleaseStringUTFChars(env, jstr, result);
    (*env)->DeleteLocalRef(env, jstr);

    // method returns new reference.
    return pyres;
}


// support for python 3.2
// unnecessary for python 2.6, 2.7, and 3.3+
char* pyunicode_to_utf8(PyObject *unicode)
{
    PyObject *bytesObj;
    char     *c;
    bytesObj = PyUnicode_AsUTF8String(unicode);
    if (bytesObj == NULL) {
        if (PyErr_Occurred()) {
            printf("Error converting PyUnicode to PyBytes\n");
            PyErr_Print();
        }
        return NULL;
    }

    c = PyBytes_AsString(bytesObj);
    Py_DECREF(bytesObj);
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
    return c;
}


// call toString() on jobject and return result.
// NULL on error
jstring jobject_tostring(JNIEnv *env, jobject obj)
{
    PyThreadState *_save;
    jstring     jstr;

    if (!env || !obj) {
        return NULL;
    }

    if (!JNI_METHOD(objectToString, env, JOBJECT_TYPE, "toString",
                    "()Ljava/lang/String;")) {
        process_java_exception(env);
        return NULL;
    }

    if (!objectToString) {
        PyErr_Format(PyExc_RuntimeError, "%s", "Couldn't get methodId.");
        return NULL;
    }

    Py_UNBLOCK_THREADS
    jstr = (jstring) (*env)->CallObjectMethod(env, obj, objectToString);
    Py_BLOCK_THREADS
    if (process_java_exception(env)) {
        return NULL;
    }

    return jstr;
}


// get a const char* string from java string.
// you *must* call release when you're finished with it.
// returns local reference.
const char* jstring2char(JNIEnv *env, jstring str)
{
    if (str == NULL) {
        return NULL;
    }
    return (*env)->GetStringUTFChars(env, str, 0);
}


// release memory allocated by jstring2char
void release_utf_char(JNIEnv *env, jstring str, const char *v)
{
    if (v != NULL && str != NULL) {
        (*env)->ReleaseStringUTFChars(env, str, v);
        (*env)->DeleteLocalRef(env, str);
    }
}


/* These macros are Only intended for use within the caching methods below. */
#define CACHE_CLASS(var, name)\
    if(var == NULL) {\
        clazz = (*env)->FindClass(env, name);\
        if((*env)->ExceptionOccurred(env))\
            return 0;\
        var = (*env)->NewGlobalRef(env, clazz);\
        (*env)->DeleteLocalRef(env, clazz);\
    }\

#define UNCACHE_CLASS(var)\
    if(var != NULL) {\
        (*env)->DeleteGlobalRef(env, var);\
        var = NULL;\
    }\

#define CACHE_PRIMITIVE_ARRAY(primitive, array, name)\
    if(primitive == NULL) {\
        if(array == NULL) {\
            clazz = (*env)->FindClass(env, name);\
            if((*env)->ExceptionOccurred(env))\
                return 0;\
            array = (*env)->NewGlobalRef(env, clazz);\
            (*env)->DeleteLocalRef(env, clazz);\
        }\
        clazz = (*env)->CallObjectMethod(env, array, classGetComponentType);\
        if ((*env)->ExceptionCheck(env)){\
            return 0;\
        }\
        primitive = (*env)->NewGlobalRef(env, clazz);\
        (*env)->DeleteLocalRef(env, clazz);\
    }\


/*
 * Caches the jclasses relevant to Java primitive types.
 *
 * In order to call methods that return primitives, we have to know their
 * return type.  That's easy, we're using the Reflection API to call
 * java.lang.reflect.Method.getReturnType().
 *
 * However, JNI requires us to match the return type to the corresponding
 * (*env)->Call<type>Method(args...) where <type> is the return type.
 * Therefore to quickly compare the return type we need the underlying
 * primitive types and not their Object types.  For example we need int,
 * not java.lang.Integer.
 *
 * We use two techniques to get at the primitive types:
 *
 *   1. Get the java.lang.Class for a primitive array, and then call
 *       Class.getComponentType().
 *
 *   2. Get the java.lang.Class for the Object and then access the static
 *       field Class.TYPE, e.g. java.lang.Integer.TYPE.
 *
 * Returns 1 if successful, 0 if failed.  Does not process Java exceptions.
 */
int cache_primitive_classes(JNIEnv *env)
{
    jclass   clazz, tmpclazz = NULL;
    jfieldID fieldId;

    if (!JNI_METHOD(classGetComponentType, env, JCLASS_TYPE, "getComponentType",
                    "()Ljava/lang/Class;")) {
        return 0;
    }

    CACHE_PRIMITIVE_ARRAY(JBOOLEAN_TYPE, JBOOLEAN_ARRAY_TYPE, "[Z");
    CACHE_PRIMITIVE_ARRAY(JBYTE_TYPE, JBYTE_ARRAY_TYPE, "[B");
    CACHE_PRIMITIVE_ARRAY(JCHAR_TYPE, JCHAR_ARRAY_TYPE, "[C");
    CACHE_PRIMITIVE_ARRAY(JSHORT_TYPE, JSHORT_ARRAY_TYPE, "[S");
    CACHE_PRIMITIVE_ARRAY(JINT_TYPE, JINT_ARRAY_TYPE, "[I");
    CACHE_PRIMITIVE_ARRAY(JLONG_TYPE, JLONG_ARRAY_TYPE, "[J");
    CACHE_PRIMITIVE_ARRAY(JFLOAT_TYPE, JFLOAT_ARRAY_TYPE, "[F");
    CACHE_PRIMITIVE_ARRAY(JDOUBLE_TYPE, JDOUBLE_ARRAY_TYPE, "[D");


    if (JVOID_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Void");
        if ((*env)->ExceptionOccurred(env)) {
            return 0;
        }

        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if ((*env)->ExceptionOccurred(env)) {
            return 0;
        }

        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                   clazz,
                   fieldId);
        if ((*env)->ExceptionOccurred(env)) {
            return 0;
        }

        JVOID_TYPE = (*env)->NewGlobalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, clazz);
    }

    return 1;
}

/*
 * Releases the global references to the cached jclasses of Java primitive
 * types that were setup in the above function.
 */
void unref_cache_primitive_classes(JNIEnv *env)
{
    UNCACHE_CLASS(JBOOLEAN_TYPE);
    UNCACHE_CLASS(JBYTE_TYPE);
    UNCACHE_CLASS(JSHORT_TYPE);
    UNCACHE_CLASS(JINT_TYPE);
    UNCACHE_CLASS(JLONG_TYPE);
    UNCACHE_CLASS(JFLOAT_TYPE);
    UNCACHE_CLASS(JDOUBLE_TYPE);

    // release the primitive array types
    UNCACHE_CLASS(JBOOLEAN_ARRAY_TYPE);
    UNCACHE_CLASS(JBYTE_ARRAY_TYPE);
    UNCACHE_CLASS(JSHORT_ARRAY_TYPE);
    UNCACHE_CLASS(JINT_ARRAY_TYPE);
    UNCACHE_CLASS(JLONG_ARRAY_TYPE);
    UNCACHE_CLASS(JFLOAT_ARRAY_TYPE);
    UNCACHE_CLASS(JDOUBLE_ARRAY_TYPE);
}


/*
 * Caches jclasses that Jep may use frequently.
 *
 * A single call of (*env)->FindClass("...") is generally considered fast, but
 * repeated calls add up.  Given how frequently Jep may make use of some
 * of the classes built-in to the JVM, we'll cache those classes for optimal
 * performance.
 *
 * Returns 1 if successful, 0 if failed.  Does not process Java exceptions.
 */
int cache_frequent_classes(JNIEnv *env)
{
    jclass clazz;

    CACHE_CLASS(JOBJECT_TYPE, "java/lang/Object");
    CACHE_CLASS(JSTRING_TYPE, "java/lang/String");
    CACHE_CLASS(JCLASS_TYPE, "java/lang/Class");

    CACHE_CLASS(JLIST_TYPE, "java/util/List");
    CACHE_CLASS(JMAP_TYPE, "java/util/Map");
    CACHE_CLASS(JITERABLE_TYPE, "java/lang/Iterable");
    CACHE_CLASS(JITERATOR_TYPE, "java/util/Iterator");
    CACHE_CLASS(JCOLLECTION_TYPE, "java/util/Collection");
    CACHE_CLASS(JCOMPARABLE_TYPE, "java/lang/Comparable");
    CACHE_CLASS(JBOOL_OBJ_TYPE, "java/lang/Boolean");
    CACHE_CLASS(JBYTE_OBJ_TYPE, "java/lang/Byte");
    CACHE_CLASS(JSHORT_OBJ_TYPE, "java/lang/Short");
    CACHE_CLASS(JINT_OBJ_TYPE, "java/lang/Integer");
    CACHE_CLASS(JLONG_OBJ_TYPE, "java/lang/Long");
    CACHE_CLASS(JDOUBLE_OBJ_TYPE, "java/lang/Double");
    CACHE_CLASS(JFLOAT_OBJ_TYPE, "java/lang/Float");
    CACHE_CLASS(JCHAR_OBJ_TYPE, "java/lang/Character");
    CACHE_CLASS(JNUMBER_TYPE, "java/lang/Number");
    CACHE_CLASS(JMETHOD_TYPE, "java/lang/reflect/Method");
    CACHE_CLASS(JFIELD_TYPE, "java/lang/reflect/Field");
    CACHE_CLASS(JTHROWABLE_TYPE, "java/lang/Throwable");
    CACHE_CLASS(JMODIFIER_TYPE, "java/lang/reflect/Modifier");
    CACHE_CLASS(JARRAYLIST_TYPE, "java/util/ArrayList");
    CACHE_CLASS(JHASHMAP_TYPE, "java/util/HashMap");
    CACHE_CLASS(JCOLLECTIONS_TYPE, "java/util/Collections");

#if JEP_NUMPY_ENABLED
    CACHE_CLASS(JEP_NDARRAY_TYPE, "jep/NDArray");
#endif

    // find and cache exception types we check for
    CACHE_CLASS(CLASSNOTFOUND_EXC_TYPE, "java/lang/ClassNotFoundException");
    CACHE_CLASS(INDEX_EXC_TYPE, "java/lang/IndexOutOfBoundsException");
    CACHE_CLASS(IO_EXC_TYPE, "java/io/IOException");
    CACHE_CLASS(CLASSCAST_EXC_TYPE, "java/lang/ClassCastException");
    CACHE_CLASS(ILLEGALARG_EXC_TYPE, "java/lang/IllegalArgumentException");
    CACHE_CLASS(ARITHMETIC_EXC_TYPE, "java/lang/ArithmeticException");
    CACHE_CLASS(OUTOFMEMORY_EXC_TYPE, "java/lang/OutOfMemoryError");
    CACHE_CLASS(ASSERTION_EXC_TYPE, "java/lang/AssertionError");

    if (!JNI_METHOD(JCLASS_GET_NAME, env, JCLASS_TYPE, "getName",
                    "()Ljava/lang/String;")) {
        return 0;
    }

    return 1;
}

/*
 * Releases the global references to the cached jclasses that Jep may use
 * frequently and were setup in the above function.
 */
void unref_cache_frequent_classes(JNIEnv *env)
{
    UNCACHE_CLASS(JOBJECT_TYPE);
    UNCACHE_CLASS(JSTRING_TYPE);
    UNCACHE_CLASS(JCLASS_TYPE);

    UNCACHE_CLASS(JLIST_TYPE);
    UNCACHE_CLASS(JMAP_TYPE);
    UNCACHE_CLASS(JITERABLE_TYPE);
    UNCACHE_CLASS(JITERATOR_TYPE);
    UNCACHE_CLASS(JCOLLECTION_TYPE);
    UNCACHE_CLASS(JCOMPARABLE_TYPE);
    UNCACHE_CLASS(JBOOL_OBJ_TYPE);
    UNCACHE_CLASS(JBYTE_OBJ_TYPE);
    UNCACHE_CLASS(JSHORT_OBJ_TYPE);
    UNCACHE_CLASS(JINT_OBJ_TYPE);
    UNCACHE_CLASS(JLONG_OBJ_TYPE);
    UNCACHE_CLASS(JDOUBLE_OBJ_TYPE);
    UNCACHE_CLASS(JFLOAT_OBJ_TYPE);
    UNCACHE_CLASS(JCHAR_OBJ_TYPE);
    UNCACHE_CLASS(JNUMBER_TYPE);
    UNCACHE_CLASS(JMETHOD_TYPE);
    UNCACHE_CLASS(JFIELD_TYPE);
    UNCACHE_CLASS(JTHROWABLE_TYPE);
    UNCACHE_CLASS(JMODIFIER_TYPE);
    UNCACHE_CLASS(JARRAYLIST_TYPE);
    UNCACHE_CLASS(JHASHMAP_TYPE);
    UNCACHE_CLASS(JCOLLECTIONS_TYPE);

#if JEP_NUMPY_ENABLED
    UNCACHE_CLASS(JEP_NDARRAY_TYPE);
#endif

    // release exception types we check for
    UNCACHE_CLASS(CLASSNOTFOUND_EXC_TYPE);
    UNCACHE_CLASS(INDEX_EXC_TYPE);
    UNCACHE_CLASS(IO_EXC_TYPE);
    UNCACHE_CLASS(CLASSCAST_EXC_TYPE);
    UNCACHE_CLASS(ILLEGALARG_EXC_TYPE);
    UNCACHE_CLASS(ARITHMETIC_EXC_TYPE);
    UNCACHE_CLASS(OUTOFMEMORY_EXC_TYPE);
    UNCACHE_CLASS(ASSERTION_EXC_TYPE);
}


// given the Class object, return the const ID.
// -1 on error or NULL.
// doesn't process errors!
int get_jtype(JNIEnv *env, jclass clazz)
{
    jboolean equals = JNI_FALSE;
    jboolean array  = JNI_FALSE;

    // have to find Class.isArray() method
    if (!JNI_METHOD(objectIsArray, env, JCLASS_TYPE, "isArray", "()Z")) {
        return -1;
    }

    // object checks
    if ((*env)->IsAssignableFrom(env, clazz, JOBJECT_TYPE)) {
        // check for string
        equals = (*env)->IsSameObject(env, clazz, JSTRING_TYPE);
        if ((*env)->ExceptionCheck(env)) {
            return -1;
        }
        if (equals) {
            return JSTRING_ID;
        }


        // check if it's an array first
        array = (*env)->CallBooleanMethod(env, clazz, objectIsArray);
        if ((*env)->ExceptionCheck(env)) {
            return -1;
        }

        if (array) {
            return JARRAY_ID;
        }

        // check for class
        if ((*env)->IsAssignableFrom(env, clazz, JCLASS_TYPE)) {
            return JCLASS_ID;
        }

        /*
         * TODO: contemplate adding List and jep.NDArray check in here
         */

        // ok it's not a string, array, or class, so let's call it object
        return JOBJECT_ID;
    }

    /*
     * check primitive types
     */

    // int
    equals = (*env)->IsSameObject(env, clazz, JINT_TYPE);
    if ((*env)->ExceptionCheck(env)) {
        return -1;
    }
    if (equals) {
        return JINT_ID;
    }

    // double
    equals = (*env)->IsSameObject(env, clazz, JDOUBLE_TYPE);
    if ((*env)->ExceptionCheck(env)) {
        return -1;
    }
    if (equals) {
        return JDOUBLE_ID;
    }

    // float
    equals = (*env)->IsSameObject(env, clazz, JFLOAT_TYPE);
    if ((*env)->ExceptionCheck(env)) {
        return -1;
    }
    if (equals) {
        return JFLOAT_ID;
    }

    // long
    equals = (*env)->IsSameObject(env, clazz, JLONG_TYPE);
    if ((*env)->ExceptionCheck(env)) {
        return -1;
    }
    if (equals) {
        return JLONG_ID;
    }

    // boolean
    equals = (*env)->IsSameObject(env, clazz, JBOOLEAN_TYPE);
    if ((*env)->ExceptionCheck(env)) {
        return -1;
    }
    if (equals) {
        return JBOOLEAN_ID;
    }

    // void
    equals = (*env)->IsSameObject(env, clazz, JVOID_TYPE);
    if ((*env)->ExceptionCheck(env)) {
        return -1;
    }
    if (equals) {
        return JVOID_ID;
    }

    // char
    equals = (*env)->IsSameObject(env, clazz, JCHAR_TYPE);
    if ((*env)->ExceptionCheck(env)) {
        return -1;
    }
    if (equals) {
        return JCHAR_ID;
    }

    // byte
    equals = (*env)->IsSameObject(env, clazz, JBYTE_TYPE);
    if ((*env)->ExceptionCheck(env)) {
        return -1;
    }
    if (equals) {
        return JBYTE_ID;
    }

    // short
    equals = (*env)->IsSameObject(env, clazz, JSHORT_TYPE);
    if ((*env)->ExceptionCheck(env)) {
        return -1;
    }
    if (equals) {
        return JSHORT_ID;
    }

    return -1;
}


/*
 * Determines how well a parameter matches the expected type. For example a
 * python integer can be passed to java as a bool, int, long, or
 * java.lang.Integer. A bool is not very descriptive so it would return a much
 * lower value than an int. A return value of 0 indicates there is no match,
 * for example a python int cannot be used as a java.util.List. Larger return
 * values indicate a better match.
 */
int pyarg_matches_jtype(JNIEnv *env,
                        PyObject *param,
                        jclass paramType,
                        int paramTypeId)
{
    if (PyBool_Check(param)) {
        switch (paramTypeId) {
        case JBOOLEAN_ID:
            return 6;
        case JBYTE_ID:
            return 5;
        case JSHORT_ID:
            return 4;
        case JINT_ID:
            return 3;
        case JLONG_ID:
            return 2;
        case JOBJECT_ID:
            return 1;
        }
    } else if (PyLong_Check(param)) {
        switch (paramTypeId) {
        case JLONG_ID:
            return 6;
        case JINT_ID:
            return 5;
        case JSHORT_ID:
            return 4;
        case JBYTE_ID:
            return 3;
        case JBOOLEAN_ID:
            return 2;
        case JOBJECT_ID:
            return 1;
        }
    } else if (PyInt_Check(param)) {
        switch (paramTypeId) {
        case JINT_ID:
            return 6;
        case JLONG_ID:
            return 5;
        case JSHORT_ID:
            return 4;
        case JBYTE_ID:
            return 3;
        case JBOOLEAN_ID:
            return 2;
        case JOBJECT_ID:
            return 1;
        }
    } else if (PyString_Check(param)) {
        switch (paramTypeId) {
        case JSTRING_ID:
            return 3;
        case JCHAR_ID:
            if (PyString_GET_SIZE(param) == 1) {
                return 2;
            }
            break;
        case JOBJECT_ID:
            if ((*env)->IsAssignableFrom(env, JSTRING_TYPE, paramType)) {
                return 1;
            }
        }
    } else if(PyUnicode_Check(param)) {
        switch (paramTypeId) {
        case JSTRING_ID:
            return 3;
            break;
        case JCHAR_ID:
            if (PyUnicode_GET_SIZE(param) == 1) {
                return 2;
            }
            break;
        case JOBJECT_ID:
            if ((*env)->IsAssignableFrom(env, JSTRING_TYPE, paramType)) {
                return 1;
            }
        }
    } else if (PyFloat_Check(param)) {
        switch (paramTypeId) {
        case JFLOAT_ID:
        case JDOUBLE_ID:
            return 2;
        case JOBJECT_ID:
            return 1;
        }
    } else if (param == Py_None) {
        switch (paramTypeId) {
        case JOBJECT_ID:
            return 4;
        case JARRAY_ID:
            return 3;
        case JSTRING_ID:
            return 2;
        case JCLASS_ID:
            return 1;
        }
    } else if (pyjarray_check(param)) {
        switch (paramTypeId) {
        case JARRAY_ID:
            if ((*env)->IsAssignableFrom(env,
                                         ((PyJArrayObject *) param)->clazz,
                                         paramType)) {
                return 2;
            }
            break;
        case JOBJECT_ID:
            return 1;
        }
    } else if (pyjclass_check(param)) {
        switch (paramTypeId) {
        case JCLASS_ID:
            return 2;
        case JOBJECT_ID:
            if ((*env)->IsAssignableFrom(env,
                                         JCLASS_TYPE,
                                         paramType)) {
                return 1;
            }
        }
    } else if (pyjobject_check(param)) {
        switch (paramTypeId) {
        case JOBJECT_ID:
            if ((*env)->IsSameObject(env,
                                     ((PyJObject *) param)->clazz,
                                     paramType)) {
                return 2;
            } else if ((*env)->IsAssignableFrom(env,
                                                ((PyJObject *) param)->clazz,
                                                paramType)) {
                return 1;
            }
        }
    }
    // no match
    return 0;
}


// convert java object to python. use this to unbox jobject
// throws java exception on error
PyObject* convert_jobject(JNIEnv *env, jobject val, int typeid)
{
    PyThreadState *_save;

    if (getIntValue == 0) {
        int success = 1;

        // get all the methodIDs here. Faster this way for Number
        // subclasses, then we'll just call the right methods below
        Py_UNBLOCK_THREADS;

        // Lookup each method only if the previous lookup succeeded
        success = success
                  && JNI_METHOD(getIntValue, env, JNUMBER_TYPE, "intValue", "()I");
        success = success
                  && JNI_METHOD(getLongValue, env, JNUMBER_TYPE, "longValue", "()J");
        success = success
                  && JNI_METHOD(getDoubleValue, env, JNUMBER_TYPE, "doubleValue", "()D");
        success = success
                  && JNI_METHOD(getFloatValue, env, JNUMBER_TYPE, "floatValue", "()F");

        Py_BLOCK_THREADS;

        if (!success) {
            return NULL;
        }
    }

    switch (typeid) {
    case -1:
        // null
        Py_RETURN_NONE;

    case JARRAY_ID:
        return (PyObject *) pyjarray_new(env, val);

    case JSTRING_ID: {
        const char *str;
        PyObject *ret;

        str = jstring2char(env, val);
        ret = PyString_FromString(str);
        release_utf_char(env, val, str);

        return ret;
    }

    case JCLASS_ID:
        return (PyObject *) pyjobject_new_class(env, val);

    case JVOID_ID:
    // pass through
    // wrap as a object... try to be diligent.

    case JOBJECT_ID:
        if ((*env)->IsInstanceOf(env, val, JNUMBER_TYPE)) {
            if ((*env)->IsInstanceOf(env, val, JBYTE_OBJ_TYPE)) {
                return convert_jobject(env, val, JBYTE_ID);
            } else if ((*env)->IsInstanceOf(env, val, JSHORT_OBJ_TYPE)) {
                return convert_jobject(env, val, JSHORT_ID);
            } else if ((*env)->IsInstanceOf(env, val, JINT_OBJ_TYPE)) {
                return convert_jobject(env, val, JINT_ID);
            } else if ((*env)->IsInstanceOf(env, val, JLONG_OBJ_TYPE)) {
                return convert_jobject(env, val, JLONG_ID);
            } else if ((*env)->IsInstanceOf(env, val, JDOUBLE_OBJ_TYPE)) {
                return convert_jobject(env, val, JDOUBLE_ID);
            } else if ((*env)->IsInstanceOf(env, val, JFLOAT_OBJ_TYPE)) {
                return convert_jobject(env, val, JFLOAT_ID);
            }
        } else if ((*env)->IsInstanceOf(env, val, JBOOL_OBJ_TYPE)) {
            return convert_jobject(env, val, JBOOLEAN_ID);
        } else if ((*env)->IsInstanceOf(env, val, JCHAR_OBJ_TYPE)) {
            return convert_jobject(env, val, JCHAR_ID);
        }
        return (PyObject *) pyjobject_new(env, val);

    case JBOOLEAN_ID: {
        jboolean b;

        if (!JNI_METHOD(getBooleanValue, env, JBOOL_OBJ_TYPE, "booleanValue", "()Z")) {
            return NULL;
        }

        b = (*env)->CallBooleanMethod(env, val, getBooleanValue);
        if ((*env)->ExceptionOccurred(env)) {
            return NULL;
        }

        if (b) {
            Py_RETURN_TRUE;
        }
        Py_RETURN_FALSE;
    }

    case JBYTE_ID:              /* pass through */
    case JSHORT_ID:             /* pass through */
    case JINT_ID: {
        jint b = (*env)->CallIntMethod(env, val, getIntValue);
        if ((*env)->ExceptionOccurred(env)) {
            return NULL;
        }

        return Py_BuildValue("i", b);
    }

    case JLONG_ID: {
        jlong b = (*env)->CallLongMethod(env, val, getLongValue);
        if ((*env)->ExceptionOccurred(env)) {
            return NULL;
        }

        return PyLong_FromLongLong(b);
    }

    case JDOUBLE_ID: {
        jdouble b = (*env)->CallDoubleMethod(env, val, getDoubleValue);
        if ((*env)->ExceptionOccurred(env)) {
            return NULL;
        }

        return PyFloat_FromDouble(b);
    }

    case JFLOAT_ID: {
        jfloat b = (*env)->CallFloatMethod(env, val, getFloatValue);
        if ((*env)->ExceptionOccurred(env)) {
            return NULL;
        }

        return PyFloat_FromDouble(b);
    }


    case JCHAR_ID: {
        jchar c;

        if (!JNI_METHOD(getCharValue, env, JCHAR_OBJ_TYPE, "charValue", "()C")) {
            return NULL;
        }

        c = (*env)->CallCharMethod(env, val, getCharValue);
        if ((*env)->ExceptionOccurred(env)) {
            return NULL;
        }

        return PyString_FromFormat("%c", (char) c);
    }

    default:
        break;
    }

    THROW_JEP(env, "util.c:convert_jobject invalid typeid.");
    return NULL;
}

/*
 * Converts java object to python.  Use this to unbox jobject.
 * Wraps convert_jobject() method above to provide for situations
 * where you don't know the jobject's type beforehand.
 */
PyObject* convert_jobject_pyobject(JNIEnv *env, jobject val)
{
    int typeId         = -1;

    if (val != NULL) {
        jclass retClass = (*env)->GetObjectClass(env, val);
        typeId = get_jtype(env, retClass);
        (*env)->DeleteLocalRef(env, retClass);
    }

    return convert_jobject(env, val, typeId);
}


// for parsing args.
// takes a python object and sets the right jvalue member for the given java type.
// returns uninitialized on error and raises a python exception.
jvalue convert_pyarg_jvalue(JNIEnv *env, PyObject *param, jclass paramType,
                            int paramTypeId, int pos)
{
    jvalue ret = PyObject_As_jvalue(env, param, paramType);
    if (PyErr_Occurred()) {
        PyObject *ptype, *pvalue, *ptrace, *pvalue_string;
        PyErr_Fetch(&ptype, &pvalue, &ptrace);
        if (pvalue == NULL) {
            pvalue_string = PyObject_Str(ptype);
        } else {
            pvalue_string = PyObject_Str(pvalue);
        }
        PyErr_Format(PyExc_TypeError, "Error converting parameter %d: %s", pos + 1,
                     PyString_AsString(pvalue_string));
        Py_DECREF(pvalue_string);
        Py_DECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptrace);
    }
    return ret;
}


// convenience function to pull a value from a list of tuples.
// expects tuples to be key, value format.
// parameters cannot be invalid!
// steals all references.
// returns new reference, new reference to Py_None if not found
PyObject* tuplelist_getitem(PyObject *list, PyObject *pyname)
{
    Py_ssize_t i, listSize;
    PyObject *ret = NULL;

    listSize = PyList_GET_SIZE(list);
    for (i = 0; i < listSize; i++) {
        PyObject *tuple = PyList_GetItem(list, i);        /* borrowed */

        if (!tuple || !PyTuple_Check(tuple)) {
            continue;
        }

        if (PyTuple_Size(tuple) == 2) {
            PyObject *key = PyTuple_GetItem(tuple, 0);    /* borrowed */
            if (!key || !PyString_Check(key)) {
                continue;
            }

            if (PyObject_RichCompareBool(key, pyname, Py_EQ)) {
                ret   = PyTuple_GetItem(tuple, 1);        /* borrowed */
                break;
            }
        }
    }

    if (!ret) {
        ret = Py_None;
    }

    Py_INCREF(ret);
    return ret;
}
