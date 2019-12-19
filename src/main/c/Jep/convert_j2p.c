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

#if PY_MAJOR_VERSION < 3
static jobject UTF8;

PyObject* jchar_As_PyObject(jchar jc)
{
    if (jc < 0xFF) {
        char c = (char) jc;
        return PyString_FromStringAndSize(&c, 1);
    } else {
        PyObject* pyunicode = NULL;
        PyObject* pystring  = NULL;
        pyunicode = PyUnicode_DecodeUTF16((const char*) &jc, 2, NULL, NULL);
        if (pyunicode == NULL) {
            return NULL;
        }
        pystring = PyUnicode_AsUTF8String(pyunicode);
        Py_DECREF(pyunicode);
        return pystring;
    }
}

PyObject* jstring_As_PyString(JNIEnv *env, jstring jstr)
{
    PyObject* result;
    // Do not use GetStringUTFChars because it does not return true UTF-8 and
    // fails for some unicode input
    jbyteArray stringJbytes = NULL;
    jsize      length       = 0;
    jbyte*     stringBytes  = NULL;
    if ( UTF8 == NULL) {
        jobject local = (*env)->NewStringUTF(env, "UTF-8");
        UTF8 = (*env)->NewGlobalRef(env, local);
        (*env)->DeleteLocalRef(env, local);
    }

    stringJbytes = java_lang_String_getBytes(env, jstr, UTF8);
    if (process_java_exception(env)) {
        return NULL;
    }

    length = (*env)->GetArrayLength(env, stringJbytes);
    stringBytes = (*env)->GetByteArrayElements(env, stringJbytes, NULL);

    result = PyString_FromStringAndSize((char *) stringBytes, length);

    (*env)->ReleaseByteArrayElements(env, stringJbytes, stringBytes, JNI_ABORT);
    (*env)->DeleteLocalRef(env, stringJbytes);
    return result;
}
#else
PyObject* jchar_As_PyObject(jchar c)
{
    Py_UCS2 value = (Py_UCS2) c;
    return PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, &value, 1);
}

PyObject* jstring_As_PyString(JNIEnv *env, jstring jstr)
{
    PyObject* result;
    const jchar *str = (*env)->GetStringChars(env, jstr, 0);
    jsize size = (*env)->GetStringLength(env, jstr);
    result = PyUnicode_DecodeUTF16((const char*) str, size * 2, NULL, NULL);
    (*env)->ReleaseStringChars(env, jstr, str);
    return result;
}
#endif

PyObject* JPyObject_As_PyObject(JNIEnv *env, jobject jobj)
{
    PyObject *ret;
    jlong l = jep_python_PyObject_getPyObject(env, jobj);
    ret = (PyObject*) l;
    Py_INCREF(ret);
    return ret;
}

PyObject* jobject_As_PyString(JNIEnv *env, jobject jobj)
{
    PyObject   *result;
    jstring     jstr;

    jstr = java_lang_Object_toString(env, jobj);
    if (process_java_exception(env)) {
        return NULL;
    } else if (jstr == NULL) {
        Py_RETURN_NONE;
    }
    result = jstring_As_PyString(env, jstr);
    (*env)->DeleteLocalRef(env, jstr);
    return result;
}

static PyObject* Boolean_As_PyObject(JNIEnv *env, jobject jobj)
{
    jboolean b = java_lang_Boolean_booleanValue(env, jobj);
    if ((*env)->ExceptionCheck(env)) {
        return NULL;
    }
    return jboolean_As_PyObject(b);
}

static PyObject* Character_As_PyObject(JNIEnv *env, jobject jobj)
{
    jchar c = java_lang_Character_charValue(env, jobj);
    if ((*env)->ExceptionCheck(env)) {
        return NULL;
    }

    return jchar_As_PyObject(c);
}

static PyObject* jnumber_As_PyObject(JNIEnv *env, jobject jobj, jclass class)
{
    if ((*env)->IsSameObject(env, class, JBYTE_OBJ_TYPE)) {
        jbyte b = java_lang_Number_byteValue(env, jobj);
        if ((*env)->ExceptionCheck(env)) {
            process_java_exception(env);
            return NULL;
        }
        return jbyte_As_PyObject(b);
    } else if ((*env)->IsSameObject(env, class, JSHORT_OBJ_TYPE)) {
        jshort s = java_lang_Number_shortValue(env, jobj);
        if ((*env)->ExceptionCheck(env)) {
            process_java_exception(env);
            return NULL;
        }
        return jshort_As_PyObject(s);
    } else if ((*env)->IsSameObject(env, class, JINT_OBJ_TYPE)) {
        jint i = java_lang_Number_intValue(env, jobj);
        if ((*env)->ExceptionCheck(env)) {
            process_java_exception(env);
            return NULL;
        }
        return jint_As_PyObject(i);
    } else if ((*env)->IsSameObject(env, class, JLONG_OBJ_TYPE)) {
        jlong j = java_lang_Number_longValue(env, jobj);
        if ((*env)->ExceptionCheck(env)) {
            process_java_exception(env);
            return NULL;
        }
        return jlong_As_PyObject(j);
    } else if ((*env)->IsSameObject(env, class, JDOUBLE_OBJ_TYPE)) {
        jdouble d = java_lang_Number_doubleValue(env, jobj);
        if ((*env)->ExceptionCheck(env)) {
            process_java_exception(env);
            return NULL;
        }
        return jdouble_As_PyObject(d);
    } else if ((*env)->IsSameObject(env, class, JFLOAT_OBJ_TYPE)) {
        jfloat f = java_lang_Number_floatValue(env, jobj);
        if ((*env)->ExceptionCheck(env)) {
            process_java_exception(env);
            return NULL;
        }
        return jfloat_As_PyObject(f);
    } else {
        return PyJNumber_Wrap(env, jobj, class);
    }

}

PyObject* jobject_As_PyJObject(JNIEnv *env, jobject jobj, jclass class)
{
    PyObject* result = NULL;
    if ((*env)->IsAssignableFrom(env, class, JITERABLE_TYPE)) {
        if ((*env)->IsAssignableFrom(env, class, JCOLLECTION_TYPE)) {
            if ((*env)->IsAssignableFrom(env, class, JLIST_TYPE)) {
                result = PyJList_Wrap(env, jobj, class);
            } else {
                result = PyJCollection_Wrap(env, jobj, class);
            }
        } else {
            result = PyJIterable_Wrap(env, jobj, class);
        }
    } else if ((*env)->IsAssignableFrom(env, class, JMAP_TYPE)) {
        result = PyJMap_Wrap(env, jobj, class);
    } else if ((*env)->IsAssignableFrom(env, class, JITERATOR_TYPE)) {
        result = PyJIterator_Wrap(env, jobj, class);
    } else if ((*env)->IsAssignableFrom(env, class, JAUTOCLOSEABLE_TYPE)) {
        result = PyJAutoCloseable_Wrap(env, jobj, class);
    } else if ((*env)->IsSameObject(env, class, JCLASS_TYPE)) {
        result = PyJClass_Wrap(env, jobj);
    } else if ((*env)->IsAssignableFrom(env, class, JNUMBER_TYPE)) {
        result = PyJNumber_Wrap(env, jobj, class);
    } else if ((*env)->IsAssignableFrom(env, class, JBUFFER_TYPE)) {
        result = PyJBuffer_Wrap(env, jobj, class);
    } else {
        result = PyJObject_Wrap(env, jobj, class);
    }
    return result;
}

PyObject* jobject_As_PyObject(JNIEnv *env, jobject jobj)
{
    PyObject* result = NULL;
    jclass    class  = NULL;
    if (jobj == NULL) {
        Py_RETURN_NONE;
    }
    class = (*env)->GetObjectClass(env, jobj);
    if ((*env)->IsSameObject(env, class, JSTRING_TYPE)) {
        result = jstring_As_PyString(env, (jstring) jobj);
    } else if ((*env)->IsAssignableFrom(env, class, JNUMBER_TYPE)) {
        result = jnumber_As_PyObject(env, jobj, class);
    } else if ((*env)->IsSameObject(env, class, JBOOL_OBJ_TYPE)) {
        result = Boolean_As_PyObject(env, jobj);
    } else if ((*env)->IsSameObject(env, class, JCHAR_OBJ_TYPE)) {
        result = Character_As_PyObject(env, jobj);
    } else if ((*env)->IsAssignableFrom(env, class, JPYOBJECT_TYPE)) {
        result = JPyObject_As_PyObject(env, jobj);
#if JEP_NUMPY_ENABLED
    } else if (jndarray_check(env, jobj)) {
        result = convert_jndarray_pyndarray(env, jobj);
#endif
    } else {
        jboolean array = java_lang_Class_isArray(env, class);
        if ((*env)->ExceptionCheck(env)) {
            process_java_exception(env);
        } else if (array) {
            result = pyjarray_new(env, jobj);
        } else {
            int proxy_exc_occurred = 0;
            if ((*env)->IsAssignableFrom(env, class, JAVA_PROXY_TYPE)) {
                jobject jpyObject = jep_Proxy_getPyObject(env, jobj);
                if (jpyObject) {
                    result = JPyObject_As_PyObject(env, jpyObject);
                } else if ((*env)->ExceptionCheck(env)) {
                    proxy_exc_occurred = 1;
                    process_java_exception(env);
                }
            }

            if (!result && !proxy_exc_occurred) {
                result = jobject_As_PyJObject(env, jobj, class);
#if JEP_NUMPY_ENABLED
                /*
                 * check for jep/DirectNDArray and autoconvert to numpy.ndarray
                 * pyjobject
                 */
                if (jdndarray_check(env, jobj)) {
                    PyObject* ndarray = convert_jdndarray_pyndarray(env, result);
                    if (ndarray) {
                        result = ndarray;
                    } else {
                        Py_CLEAR(result);
                    }
                } else if (PyErr_Occurred()) {
                    Py_CLEAR(result);
                }
#endif
            }
        }
    }
    (*env)->DeleteLocalRef(env, class);
    return result;
}


