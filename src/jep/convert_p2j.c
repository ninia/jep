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

#define JBYTE_MAX   127
#define JBYTE_MIN  -128
#define JSHORT_MAX  32767
#define JSHORT_MIN -32768
#define JINT_MAX    2147483647
/* Some compilers overflow parsing -2147483648 so derive it from JINT_MAX. */
#define JINT_MIN    (-1 * JINT_MAX - 1)
#define JLONG_MAX   9223372036854775807
/* Some compilers overflow parsing -9223372036854775808, so derive it from JLONG_MAX. */
#define JLONG_MIN   (-1 * JLONG_MAX - 1)

#define JCHAR_MAX   0xFFFF

static jmethodID hashmapIConstructor   = 0;
static jmethodID hashmapPut            = 0;
static jmethodID arraylistIConstructor = 0;
static jmethodID arraylistAdd          = 0;
static jmethodID unmodifiableList      = 0;

/*
 * When there is no way to convert a PyObject of a specific expected Java type
 * then a Python TypeError is raised. This function is used to make a pretty
 * error message.
 */
static void raiseTypeError(JNIEnv *env, PyObject *pyobject, jclass expectedType)
{
    /* expected and actual type names in java and c */
    jstring expTypeJavaName;
    const char *expTypeName, *actTypeName;

    expTypeJavaName = (*env)->CallObjectMethod(env, expectedType, JCLASS_GET_NAME);
    if (process_java_exception(env)) {
        return;
    }
    expTypeName = (*env)->GetStringUTFChars(env, expTypeJavaName, 0);
    if (pyjclass_check(pyobject)) {
        actTypeName = "java.lang.Class";
    } else if (pyjobject_check(pyobject)) {
        actTypeName = PyString_AsString(((PyJObject*) pyobject)->javaClassName);
    } else {
        actTypeName = pyobject->ob_type->tp_name;
    }
    PyErr_Format(PyExc_TypeError, "Expected %s but received a %s.", expTypeName,
                 actTypeName);
    (*env)->ReleaseStringUTFChars(env, expTypeJavaName, expTypeName);
    (*env)->DeleteLocalRef(env, expTypeJavaName);
}



jboolean PyObject_As_jboolean(PyObject *pylong)
{
    if (PyObject_IsTrue(pylong)) {
        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}

jbyte PyObject_As_jbyte(PyObject *pyobject)
{
    PyObject *pyindex;
    long i;
    pyindex = PyNumber_Index(pyobject);
    if (pyindex == NULL) {
        return -1;
    }
    i = PyLong_AsLong(pyindex);
    Py_DECREF(pyindex);
    /* No need to check for error, just propagate the -1, */
    if (i < JBYTE_MIN || i > JBYTE_MAX) {
        PyErr_Format(PyExc_OverflowError, "%ld is outside the valid range "
                     "of a Java byte.", i);
        return -1;
    }
    return (jbyte) i;
}

jshort PyObject_As_jshort(PyObject *pyobject)
{
    PyObject *pyindex;
    long i;
    pyindex = PyNumber_Index(pyobject);
    if (pyindex == NULL) {
        return -1;
    }
    i = PyLong_AsLong(pyindex);
    Py_DECREF(pyindex);
    /* No need to check for error, just propagate the -1. */
    if (i < JSHORT_MIN || i > JSHORT_MAX) {
        PyErr_Format(PyExc_OverflowError, "%ld is outside the valid range "
                     "of a Java short.", i);
        return -1;
    }
    return (jshort) i;
}

jint PyObject_As_jint(PyObject *pyobject)
{
    PyObject *pyindex;
    long i;
    pyindex = PyNumber_Index(pyobject);
    if (pyindex == NULL) {
        return -1;
    }
    i = PyLong_AsLong(pyindex);
    Py_DECREF(pyindex);
    /* No need to check for error, just propagate the -1. */
    if (i < JINT_MIN || i > JINT_MAX) {
        PyErr_Format(PyExc_OverflowError, "%ld is outside the valid range "
                     "of a Java int.", i);
        return -1;
    }
    return (jint) i;
}

jlong PyObject_As_jlong(PyObject *pyobject)
{
    PyObject *pyindex;
    PY_LONG_LONG i;
    pyindex = PyNumber_Index(pyobject);
    if (pyindex == NULL) {
        return -1;
    }
    i = PyLong_AsLongLong(pyindex);
    Py_DECREF(pyindex);
    /* No need to check for error, just propagate the -1. */
    if (i < JLONG_MIN || i > JLONG_MAX) {
        PyErr_Format(PyExc_OverflowError, "%lld is outside the valid range "
                     "of a Java long.", i);
        return -1;
    }
    return (jlong) i;
}

/*
 * This is a function instead of a macro to be consistent with other
 * conversions. Hopefully the compiler will figure out to inline it.
 */
jfloat PyObject_As_jfloat(PyObject *pyfloat)
{
    return (jfloat) PyFloat_AsDouble(pyfloat);
}

/*
 * This is a function instead of a macro to be consistent with other
 * conversions. Hopefully the compiler will figure out to inline it.
 */
jdouble PyObject_As_jdouble(PyObject *pyfloat)
{
    return (jdouble) PyFloat_AsDouble(pyfloat);
}

static jchar pystring_as_jchar(PyObject *pystring)
{
    if (PyString_Check(pystring) && PyString_Size(pystring) == 1) {
        return (jchar) PyString_AsString(pystring)[0];
    }
    PyErr_Format(PyExc_TypeError, "Expected char but received a %s.",
                 pystring->ob_type->tp_name);
    return 0;
}

jchar PyObject_As_jchar(PyObject *pyobject)
{
    return pystring_as_jchar(pyobject);
}

static jstring pystring_as_jstring(JNIEnv *env, PyObject *pystring)
{
    const char *s = PyString_AsString(pystring);
    if (s == NULL) {
        return NULL;
    }
    return (*env)->NewStringUTF(env, s);
}

jstring PyObject_As_jstring(JNIEnv *env, PyObject *pyobject)
{
    PyObject *pystring = PyObject_Str(pyobject);
    if (pystring == NULL) {
        return NULL;
    }
    return pystring_as_jstring(env, pystring);
}

static jobject pybool_as_jobject(JNIEnv *env, PyObject *pyobject,
                                 jclass expectedType)
{
    if ((*env)->IsAssignableFrom(env, JBOOL_OBJ_TYPE, expectedType)) {
        jboolean z = PyObject_As_jboolean(pyobject);
        if (PyErr_Occurred()) {
            return NULL;
        }
        return JBox_Boolean(env, z);
    }
    raiseTypeError(env, pyobject, expectedType);
    return NULL;
}

static jobject pystring_as_jobject(JNIEnv *env, PyObject *pyobject,
                                   jclass expectedType)
{
    if ((*env)->IsAssignableFrom(env, JSTRING_TYPE, expectedType)) {
        return (jobject) pystring_as_jstring(env, pyobject);
    } else if ((*env)->IsAssignableFrom(env, JCHAR_OBJ_TYPE, expectedType)) {
        jchar c = pystring_as_jchar(pyobject);
        if (c == 0 && PyErr_Occurred()) {
            return NULL;
        }
        return JBox_Char(env, c);
    }
    raiseTypeError(env, pyobject, expectedType);
    return NULL;
}

static jobject pylong_as_jobject(JNIEnv *env, PyObject *pyobject,
                                 jclass expectedType)
{
    if ((*env)->IsAssignableFrom(env, JLONG_OBJ_TYPE, expectedType)) {
        jlong j = PyObject_As_jlong(pyobject);
        if (j == -1 && PyErr_Occurred()) {
            return NULL;
        }
        return JBox_Long(env, j);
    } else if ((*env)->IsAssignableFrom(env, JINT_OBJ_TYPE, expectedType)) {
        jint i = PyObject_As_jint(pyobject);
        if (i == -1 && PyErr_Occurred()) {
            return NULL;
        }
        return JBox_Int(env, i);
    } else if ((*env)->IsAssignableFrom(env, JBYTE_OBJ_TYPE, expectedType)) {
        jbyte b = PyObject_As_jbyte(pyobject);
        if (b == -1 && PyErr_Occurred()) {
            return NULL;
        }
        return JBox_Byte(env, b);
    } else if ((*env)->IsAssignableFrom(env, JSHORT_OBJ_TYPE, expectedType)) {
        jshort s = PyObject_As_jshort(pyobject);
        if (s == -1 && PyErr_Occurred()) {
            return NULL;
        }
        return JBox_Short(env, s);
    }
    raiseTypeError(env, pyobject, expectedType);
    return NULL;
}

#if PY_MAJOR_VERSION < 3
static jobject pyint_as_jobject(JNIEnv *env, PyObject *pyobject,
                                jclass expectedType)
{
    if ((*env)->IsAssignableFrom(env, JINT_OBJ_TYPE, expectedType)) {
        jint i = PyObject_As_jint(pyobject);
        if (i == -1 && PyErr_Occurred()) {
            if (PyErr_ExceptionMatches(PyExc_OverflowError)) {
                /* Just in case expectedType is Object or Number and it fits in a long. */
                PyErr_Clear();
                return pylong_as_jobject(env, pyobject, expectedType);
            } else {
                return NULL;
            }
        }
        return JBox_Int(env, i);
    }
    return pylong_as_jobject(env, pyobject, expectedType);
}
#endif

static jobject pyfloat_as_jobject(JNIEnv *env, PyObject *pyobject,
                                  jclass expectedType)
{
    if ((*env)->IsAssignableFrom(env, JDOUBLE_OBJ_TYPE, expectedType)) {
        jdouble d = PyObject_As_jdouble(pyobject);
        if (d == -1.0 && PyErr_Occurred()) {
            return NULL;
        }
        return JBox_Double(env, d);
    } else if ((*env)->IsAssignableFrom(env, JFLOAT_OBJ_TYPE, expectedType)) {
        jfloat f = PyObject_As_jfloat(pyobject);
        if (f == -1.0 && PyErr_Occurred()) {
            return NULL;
        }
        return JBox_Float(env, f);
    }
    raiseTypeError(env, pyobject, expectedType);
    return NULL;
}

/* Convert a list or tuple to an ArrayList */
static jobject pyfastsequence_as_jobject(JNIEnv *env, PyObject *pyseq,
        jclass expectedType)
{
    if ((*env)->IsAssignableFrom(env, JLIST_TYPE, expectedType)
            || (PyList_Check(pyseq)
                && (*env)->IsAssignableFrom(env, JARRAYLIST_TYPE, expectedType))) {
        jobject jlist;
        Py_ssize_t size, i;

        if (!JNI_METHOD(arraylistIConstructor, env, JARRAYLIST_TYPE, "<init>",
                        "(I)V")) {
            process_java_exception(env);
            return NULL;
        }
        if (!JNI_METHOD(arraylistAdd, env, JARRAYLIST_TYPE, "add",
                        "(Ljava/lang/Object;)Z")) {
            process_java_exception(env);
            return NULL;
        }

        size = PySequence_Fast_GET_SIZE(pyseq);

        if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
            process_java_exception(env);
            return NULL;
        }

        jlist = (*env)->NewObject(env, JARRAYLIST_TYPE, arraylistIConstructor,
                                  (int) size);
        if (!jlist) {
            process_java_exception(env);
            return (*env)->PopLocalFrame(env, NULL);
        }

        for (i = 0; i < size; i++) {
            jobject value;
            PyObject *item = PySequence_Fast_GET_ITEM(pyseq, i);
            value = PyObject_As_jobject(env, item, JOBJECT_TYPE);
            if (value == NULL && PyErr_Occurred()) {
                /*
                 * java exceptions will have been transformed to python
                 * exceptions by this point
                 */
                return (*env)->PopLocalFrame(env, NULL);
            }
            (*env)->CallBooleanMethod(env, jlist, arraylistAdd, value);
            (*env)->DeleteLocalRef(env, value);
            if (process_java_exception(env)) {
                return (*env)->PopLocalFrame(env, NULL);
            }
        }

        if (PyTuple_Check(pyseq)) {
            if (!unmodifiableList) {
                unmodifiableList = (*env)->GetStaticMethodID(env, JCOLLECTIONS_TYPE,
                                   "unmodifiableList", "(Ljava/util/List;)Ljava/util/List;");
                if (!unmodifiableList) {
                    process_java_exception(env);
                    return (*env)->PopLocalFrame(env, NULL);
                }
            }
            jlist = (*env)->CallStaticObjectMethod(env, JCOLLECTIONS_TYPE,
                                                   unmodifiableList, jlist);
            if (process_java_exception(env)) {
                return (*env)->PopLocalFrame(env, NULL);
            }
        }
        return (*env)->PopLocalFrame(env, jlist);
    }
    raiseTypeError(env, pyseq, expectedType);
    return NULL;
}

static jobject pydict_as_jobject(JNIEnv *env, PyObject *pydict,
                                 jclass expectedType)
{
    if ((*env)->IsAssignableFrom(env, JHASHMAP_TYPE, expectedType)) {
        jobject jmap;
        Py_ssize_t size, pos;
        PyObject *key, *value;

        if (!JNI_METHOD(hashmapIConstructor, env, JHASHMAP_TYPE, "<init>", "(I)V")) {
            process_java_exception(env);
            return NULL;
        }
        if (!JNI_METHOD(hashmapPut, env, JHASHMAP_TYPE, "put",
                        "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;")) {
            process_java_exception(env);
            return NULL;
        }

        size = PyDict_Size(pydict);
        /* Account for default load factor */
        size = (Py_ssize_t) ((size / 0.75) + 1);

        if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
            process_java_exception(env);
            return NULL;
        }

        jmap = (*env)->NewObject(env, JHASHMAP_TYPE, hashmapIConstructor, (jint) size);
        if (!jmap) {
            process_java_exception(env);
            (*env)->PopLocalFrame(env, NULL);
            return NULL;
        }

        pos = 0;
        while (PyDict_Next(pydict, &pos, &key, &value)) {
            jobject jkey, jvalue;
            jkey = PyObject_As_jobject(env, key, JOBJECT_TYPE);
            if (jkey == NULL && PyErr_Occurred()) {
                (*env)->PopLocalFrame(env, NULL);
                return NULL;
            }
            jvalue = PyObject_As_jobject(env, value, JOBJECT_TYPE);
            if (jvalue == NULL && PyErr_Occurred()) {
                (*env)->PopLocalFrame(env, NULL);
                return NULL;
            }

            (*env)->CallObjectMethod(env, jmap, hashmapPut, jkey, jvalue);
            (*env)->DeleteLocalRef(env, jkey);
            (*env)->DeleteLocalRef(env, jvalue);
            if (process_java_exception(env)) {
                (*env)->PopLocalFrame(env, NULL);
                return NULL;
            }
        }
        return (*env)->PopLocalFrame(env, jmap);
    }
    raiseTypeError(env, pydict, expectedType);
    return NULL;
}


jobject PyObject_As_jobject(JNIEnv *env, PyObject *pyobject,
                            jclass expectedType)
{
    if (pyobject == Py_None) {
        return NULL;
    } else if (pyjclass_check(pyobject)) {
        if ((*env)->IsAssignableFrom(env, JCLASS_TYPE, expectedType)) {
            return (*env)->NewLocalRef(env, ((PyJObject *) pyobject)->clazz);
        }
    } else if (pyjobject_check(pyobject)) {
        PyJObject *pyjobject = (PyJObject*) pyobject;
        if ((*env)->IsAssignableFrom(env, pyjobject->clazz, expectedType)) {
            return (*env)->NewLocalRef(env, pyjobject->object);
        }
    } else if (PyString_Check(pyobject)) {
        return pystring_as_jobject(env, pyobject, expectedType);
    } else if (PyBool_Check(pyobject)) {
        return pybool_as_jobject(env, pyobject, expectedType);
    } else if (PyLong_Check(pyobject)) {
        return pylong_as_jobject(env, pyobject, expectedType);
#if PY_MAJOR_VERSION < 3
    } else if (PyInt_Check(pyobject)) {
        return pyint_as_jobject(env, pyobject, expectedType);
#endif
    } else if (PyFloat_Check(pyobject)) {
        return pyfloat_as_jobject(env, pyobject, expectedType);
    } else if (pyjarray_check(pyobject)) {
        PyJArrayObject *pyjarray = (PyJArrayObject *) pyobject;
        if ((*env)->IsAssignableFrom(env, pyjarray->clazz, expectedType)) {
            pyjarray_release_pinned(pyjarray, JNI_COMMIT);
            return (*env)->NewLocalRef(env, pyjarray->object);
        }
    } else if (PyList_Check(pyobject) || PyTuple_Check(pyobject)) {
        return pyfastsequence_as_jobject(env, pyobject, expectedType);
    } else if (PyDict_Check(pyobject)) {
        return pydict_as_jobject(env, pyobject, expectedType);
#if JEP_NUMPY_ENABLED
    } else if (npy_array_check(pyobject)) {
        if ((*env)->IsAssignableFrom(env, JEP_NDARRAY_TYPE, expectedType)) {
            return convert_pyndarray_jndarray(env, pyobject);
        } else {
            return convert_pyndarray_jprimitivearray(env, pyobject, expectedType);
        }
#endif
    } else if ((*env)->IsAssignableFrom(env, JSTRING_TYPE, expectedType)) {
        return (jobject) PyObject_As_jstring(env, pyobject);
    }
    raiseTypeError(env, pyobject, expectedType);
    return NULL;
}

jvalue PyObject_As_jvalue(JNIEnv *env, PyObject *pyobject, jclass expectedType)
{
    jvalue result;
    if ((*env)->IsAssignableFrom(env, expectedType, JOBJECT_TYPE)) {
        result.l = PyObject_As_jobject(env, pyobject, expectedType);
    } else if ((*env)->IsSameObject(env, expectedType, JINT_TYPE)) {
        result.i = PyObject_As_jint(pyobject);
    } else if ((*env)->IsSameObject(env, expectedType, JDOUBLE_TYPE)) {
        result.d = PyObject_As_jdouble(pyobject);
    } else if ((*env)->IsSameObject(env, expectedType, JFLOAT_TYPE)) {
        result.f = PyObject_As_jfloat(pyobject);
    } else if ((*env)->IsSameObject(env, expectedType, JLONG_TYPE)) {
        result.j = PyObject_As_jlong(pyobject);
    } else if ((*env)->IsSameObject(env, expectedType, JBOOLEAN_TYPE)) {
        result.z = PyObject_As_jboolean(pyobject);
    } else if ((*env)->IsSameObject(env, expectedType, JCHAR_TYPE)) {
        result.c = PyObject_As_jchar(pyobject);
    } else if ((*env)->IsSameObject(env, expectedType, JBYTE_TYPE)) {
        result.b = PyObject_As_jbyte(pyobject);
    } else if ((*env)->IsSameObject(env, expectedType, JSHORT_TYPE)) {
        result.s = PyObject_As_jshort(pyobject);
    } else if ((*env)->IsSameObject(env, expectedType, JVOID_TYPE)) {
        PyErr_SetString(PyExc_TypeError, "Cannot convert any PyObject to Void");
        result.l = NULL;
    } else {
        /* This is theoretically impossible */
        PyErr_SetString(PyExc_TypeError, "Unrecognized java type.");
        result.l = NULL;
    }
    return result;
}
