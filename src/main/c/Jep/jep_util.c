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

#include "Jep.h"


/*
 * Caching of jclass objects for optimal performance.  These are shared for
 * all threads and should be considered constants.
 */

// primitive class types
jclass JINT_TYPE     = NULL;
jclass JLONG_TYPE    = NULL;
jclass JBOOLEAN_TYPE = NULL;
jclass JVOID_TYPE    = NULL;
jclass JDOUBLE_TYPE  = NULL;
jclass JSHORT_TYPE   = NULL;
jclass JFLOAT_TYPE   = NULL;
jclass JCHAR_TYPE    = NULL;
jclass JBYTE_TYPE    = NULL;

// cached types for primitive arrays
jclass JBOOLEAN_ARRAY_TYPE = NULL;
jclass JBYTE_ARRAY_TYPE    = NULL;
jclass JCHAR_ARRAY_TYPE    = NULL;
jclass JSHORT_ARRAY_TYPE   = NULL;
jclass JINT_ARRAY_TYPE     = NULL;
jclass JLONG_ARRAY_TYPE    = NULL;
jclass JFLOAT_ARRAY_TYPE   = NULL;
jclass JDOUBLE_ARRAY_TYPE  = NULL;

#define DEFINE_CLASS_VAR(var, name) jclass var = NULL;
CLASS_TABLE(DEFINE_CLASS_VAR)

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
        if((*env)->ExceptionCheck(env))\
            return 0;\
        var = (*env)->NewGlobalRef(env, clazz);\
        (*env)->DeleteLocalRef(env, clazz);\
    }\

#define UNCACHE_CLASS(var, name)\
    if(var != NULL) {\
        (*env)->DeleteGlobalRef(env, var);\
        var = NULL;\
    }\

#define CACHE_PRIMITIVE_ARRAY(primitive, array, name)\
    if(primitive == NULL) {\
        if(array == NULL) {\
            clazz = (*env)->FindClass(env, name);\
            if((*env)->ExceptionCheck(env))\
                return 0;\
            array = (*env)->NewGlobalRef(env, clazz);\
            (*env)->DeleteLocalRef(env, clazz);\
        }\
        clazz = java_lang_Class_getComponentType(env, array);\
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
        if ((*env)->ExceptionCheck(env)) {
            return 0;
        }

        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if ((*env)->ExceptionCheck(env)) {
            return 0;
        }

        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                   clazz,
                   fieldId);
        if ((*env)->ExceptionCheck(env)) {
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
    UNCACHE_CLASS(JBOOLEAN_TYPE,);
    UNCACHE_CLASS(JBYTE_TYPE,);
    UNCACHE_CLASS(JSHORT_TYPE,);
    UNCACHE_CLASS(JINT_TYPE,);
    UNCACHE_CLASS(JLONG_TYPE,);
    UNCACHE_CLASS(JFLOAT_TYPE,);
    UNCACHE_CLASS(JDOUBLE_TYPE,);

    // release the primitive array types
    UNCACHE_CLASS(JBOOLEAN_ARRAY_TYPE,);
    UNCACHE_CLASS(JBYTE_ARRAY_TYPE,);
    UNCACHE_CLASS(JSHORT_ARRAY_TYPE,);
    UNCACHE_CLASS(JINT_ARRAY_TYPE,);
    UNCACHE_CLASS(JLONG_ARRAY_TYPE,);
    UNCACHE_CLASS(JFLOAT_ARRAY_TYPE,);
    UNCACHE_CLASS(JDOUBLE_ARRAY_TYPE,);
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

    CLASS_TABLE(CACHE_CLASS)

    return 1;
}

/*
 * Releases the global references to the cached jclasses that Jep may use
 * frequently and were setup in the above function.
 */
void unref_cache_frequent_classes(JNIEnv *env)
{
    CLASS_TABLE(UNCACHE_CLASS)
}


// given the Class object, return the const ID.
// -1 on error or NULL.
// doesn't process errors!
int get_jtype(JNIEnv *env, jclass clazz)
{
    jboolean equals = JNI_FALSE;
    jboolean array  = JNI_FALSE;

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
        array = java_lang_Class_isArray(env, clazz);
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
            return 8;
        case JBYTE_ID:
            return 7;
        case JSHORT_ID:
            return 6;
        case JINT_ID:
            return 5;
        case JLONG_ID:
            return 4;
        case JOBJECT_ID:
            if ((*env)->IsSameObject(env, JBOOL_OBJ_TYPE, paramType)) {
                return 3;
            } else if ((*env)->IsSameObject(env, JOBJECT_TYPE, paramType)) {
                return 1;
            } else if ((*env)->IsAssignableFrom(env, JBOOL_OBJ_TYPE, paramType)) {
                return 2;
            }
        }
    } else if (PyLong_Check(param)) {
        switch (paramTypeId) {
        case JLONG_ID:
            return 11;
        case JINT_ID:
            return 10;
        case JDOUBLE_ID:
            return 9;
        case JFLOAT_ID:
            return 8;
        case JSHORT_ID:
            return 7;
        case JBYTE_ID:
            return 6;
        case JBOOLEAN_ID:
            return 5;
        case JOBJECT_ID:
            if ((*env)->IsSameObject(env, JLONG_OBJ_TYPE, paramType)) {
                return 4;
            } else if ((*env)->IsSameObject(env, JINT_OBJ_TYPE, paramType)) {
                return 3;
            } else if ((*env)->IsSameObject(env, JOBJECT_TYPE, paramType)) {
                return 1;
            } else if ((*env)->IsAssignableFrom(env, JLONG_OBJ_TYPE, paramType)) {
                return 2;
            }
        }
    } else if (PyInt_Check(param)) {
        switch (paramTypeId) {
        case JINT_ID:
            return 11;
        case JLONG_ID:
            return 10;
        case JDOUBLE_ID:
            return 9;
        case JFLOAT_ID:
            return 8;
        case JSHORT_ID:
            return 7;
        case JBYTE_ID:
            return 6;
        case JBOOLEAN_ID:
            return 5;
        case JOBJECT_ID:
            if ((*env)->IsSameObject(env, JINT_OBJ_TYPE, paramType)) {
                return 4;
            } else if ((*env)->IsSameObject(env, JLONG_OBJ_TYPE, paramType)) {
                return 3;
            } else if ((*env)->IsSameObject(env, JOBJECT_TYPE, paramType)) {
                return 1;
            } else if ((*env)->IsAssignableFrom(env, JINT_OBJ_TYPE, paramType)) {
                return 2;
            }
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
    } else if (PyUnicode_Check(param)) {
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
        case JDOUBLE_ID:
            return 6;
        case JFLOAT_ID:
            return 5;
        case JOBJECT_ID:
            if ((*env)->IsSameObject(env, JDOUBLE_OBJ_TYPE, paramType)) {
                return 4;
            } else if ((*env)->IsSameObject(env, JFLOAT_OBJ_TYPE, paramType)) {
                return 3;
            } else if ((*env)->IsSameObject(env, JOBJECT_TYPE, paramType)) {
                return 1;
            } else if ((*env)->IsAssignableFrom(env, JDOUBLE_OBJ_TYPE, paramType)) {
                return 2;
            }
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
    } else if (PyJClass_Check(param)) {
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
    } else if (PyJObject_Check(param)) {
        switch (paramTypeId) {
        case JARRAY_ID:
        case JOBJECT_ID:
            if ((*env)->IsSameObject(env,
                                     ((PyJObject *) param)->clazz,
                                     paramType)) {
                return 3;
            } else if ((*env)->IsSameObject(env, JOBJECT_TYPE, paramType)) {
                return 1;
            } else if ((*env)->IsAssignableFrom(env,
                                                ((PyJObject *) param)->clazz,
                                                paramType)) {
                return 2;
            }
        }
    } else if (PyList_Check(param)) {
        switch (paramTypeId) {
        case JBOOLEAN_ID:
            return 1;
        case JOBJECT_ID:
            if ((*env)->IsSameObject(env, JOBJECT_TYPE, paramType)) {
                return 2;
            } else if ((*env)->IsSameObject(env, JARRAYLIST_TYPE, paramType)) {
                return 4;
            } else if ((*env)->IsAssignableFrom(env, JLIST_TYPE, paramType)) {
                return 3;
            }
        }
    } else if (PyTuple_Check(param)) {
        switch (paramTypeId) {
        case JBOOLEAN_ID:
            return 1;
        case JOBJECT_ID:
            if ((*env)->IsSameObject(env, JOBJECT_TYPE, paramType)) {
                return 2;
            } else if ((*env)->IsAssignableFrom(env, JLIST_TYPE, paramType)) {
                return 3;
            }
        }
    } else if (PyDict_Check(param)) {
        switch (paramTypeId) {
        case JBOOLEAN_ID:
            return 1;
        case JOBJECT_ID:
            if ((*env)->IsSameObject(env, JOBJECT_TYPE, paramType)) {
                return 2;
            } else if ((*env)->IsAssignableFrom(env, JMAP_TYPE, paramType)) {
                return 3;
            }
        }
    }
    // no match
    return 0;
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
