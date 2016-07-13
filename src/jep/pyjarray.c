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


   pyjarray is a python object for using java arrays within the interpreter.
   i got a lot of inspiration from listobject.c in the Objects/ folder
   of the python distribution.
*/

#include "Jep.h"

jmethodID objectComponentType = 0;

static void pyjarray_dealloc(PyJArrayObject *self);
static int pyjarray_init(JNIEnv*, PyJArrayObject*, int, PyObject*);
static Py_ssize_t pyjarray_length(PyObject *self);



// called internally to make new PyJArrayObject instances
PyObject* pyjarray_new(JNIEnv *env, jobjectArray obj)
{
    PyJArrayObject  *pyarray;
    jclass           clazz;

    if (PyType_Ready(&PyJArray_Type) < 0) {
        return NULL;
    }
    if (!obj) {
        PyErr_Format(PyExc_RuntimeError, "Invalid array object.");
        return NULL;
    }

    clazz = (*env)->GetObjectClass(env, obj);

    pyarray                 = PyObject_NEW(PyJArrayObject, &PyJArray_Type);
    pyarray->object         = (*env)->NewGlobalRef(env, obj);
    pyarray->clazz          = (*env)->NewGlobalRef(env, clazz);
    pyarray->componentType  = -1;
    pyarray->componentClass = NULL;
    pyarray->length         = -1;
    pyarray->pinnedArray    = NULL;

    (*env)->DeleteLocalRef(env, clazz);

    if (pyjarray_init(env, pyarray, 0, NULL)) {
        return (PyObject *) pyarray;
    } else {
        pyjarray_dealloc(pyarray);
        return NULL;
    }
}


// called from module to create new arrays.
// args are variable, should accept:
// (size, typeid, [value]), (size, jobject),
//     (size, pyjarray), (list)
PyObject* pyjarray_new_v(PyObject *isnull, PyObject *args)
{
    PyJArrayObject  *pyarray;
    jclass           clazz     = NULL, componentClass = NULL;
    JNIEnv          *env       = NULL;
    jobjectArray     arrayObj  = NULL;
    long             typeId    = -1;
    long             size      = -1;

    // args
    PyObject *one, *two, *three;
    one = two = three = NULL;

    if (PyType_Ready(&PyJArray_Type) < 0) {
        return NULL;
    }

    env = pyembed_get_env();

    if (!PyArg_UnpackTuple(args, "ref", 1, 3, &one, &two, &three)) {
        return NULL;
    }

    if (PyInt_Check(one)) {
        size = (long) PyInt_AsLong(one);

        if (PyInt_Check(two)) {
            typeId = (int) PyInt_AsLong(two);

            if (size < 0) {
                return PyErr_Format(PyExc_ValueError, "Invalid size %li", size);
            }

            // make a new primitive array
            switch (typeId) {
            case JSTRING_ID:
                arrayObj = (*env)->NewObjectArray(env,
                                                  (jsize) size,
                                                  JSTRING_TYPE,
                                                  NULL);
                break;

            case JINT_ID:
                arrayObj = (*env)->NewIntArray(env, (jsize) size);
                break;

            case JLONG_ID:
                arrayObj = (*env)->NewLongArray(env, (jsize) size);
                break;

            case JBOOLEAN_ID:
                arrayObj = (*env)->NewBooleanArray(env, (jsize) size);
                break;

            case JDOUBLE_ID:
                arrayObj = (*env)->NewDoubleArray(env, (jsize) size);
                break;

            case JSHORT_ID:
                arrayObj = (*env)->NewShortArray(env, (jsize) size);
                break;

            case JFLOAT_ID:
                arrayObj = (*env)->NewFloatArray(env, (jsize) size);
                break;

            case JBYTE_ID:
                arrayObj = (*env)->NewByteArray(env, (jsize) size);
                break;

            case JCHAR_ID:
                arrayObj = (*env)->NewCharArray(env, (jsize) size);
                break;
            } // switch

        } // if int(two)
        else if (pyjobject_check(two)) {
            PyJObject *pyjob = (PyJObject *) two;
            typeId = JOBJECT_ID;

            componentClass = pyjob->clazz;
            arrayObj = (*env)->NewObjectArray(env,
                                              (jsize) size,
                                              componentClass,
                                              NULL);
        } else if (pyjarray_check(two)) {
            PyJArrayObject *pyarray = (PyJArrayObject *) two;
            typeId = JARRAY_ID;

            componentClass = pyarray->clazz;
            arrayObj = (*env)->NewObjectArray(env,
                                              (jsize) size,
                                              componentClass,
                                              NULL);
        } else {
            PyErr_SetString(PyExc_ValueError, "Unknown arg type: expected "
                            "one of: J<foo>_ID, pyjobject, jarray");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_ValueError, "Unknown arg types.");
        return NULL;
    }

    if (process_java_exception(env)) {
        return NULL;
    }

    if (!arrayObj || typeId < -1 || size < -1) {
        PyErr_SetString(PyExc_ValueError, "Unknown type.");
        return NULL;
    }

    clazz = (*env)->GetObjectClass(env, arrayObj);

    pyarray                 = PyObject_NEW(PyJArrayObject, &PyJArray_Type);
    pyarray->object         = (*env)->NewGlobalRef(env, arrayObj);
    pyarray->clazz          = (*env)->NewGlobalRef(env, clazz);
    pyarray->componentType  = (int) typeId;
    pyarray->componentClass = NULL;
    pyarray->length         = -1;
    pyarray->pinnedArray    = NULL;

    if (typeId == JOBJECT_ID || typeId == JARRAY_ID) {
        pyarray->componentClass = (*env)->NewGlobalRef(env, componentClass);
    }

    (*env)->DeleteLocalRef(env, arrayObj);
    (*env)->DeleteLocalRef(env, clazz);

    if (pyjarray_init(env, pyarray, 1, three)) {
        return (PyObject *) pyarray;
    } else {
        pyjarray_dealloc(pyarray);
        return NULL;
    }
}


static int pyjarray_init(JNIEnv *env,
                         PyJArrayObject *pyarray,
                         int zero,
                         PyObject *value)
{
    jobject compType  = NULL;
    int     comp;

    // ------------------------------ first, get the array's type

    if (pyarray->componentType < 0) { // may already know that
        if (!JNI_METHOD(objectComponentType, env, JCLASS_TYPE, "getComponentType",
                        "()Ljava/lang/Class;")) {
            process_java_exception(env);
            goto EXIT_ERROR;
        }

        compType = (*env)->CallObjectMethod(env,
                                            pyarray->clazz,
                                            objectComponentType);
        if (process_java_exception(env) || !compType) {
            goto EXIT_ERROR;
        }

        comp = get_jtype(env, compType);
        if (process_java_exception(env) || comp < 0) {
            goto EXIT_ERROR;
        }

        pyarray->componentClass = (*env)->NewGlobalRef(env, compType);
        pyarray->componentType  = comp;
    }

    if (pyarray->length < 0) { // may already know that, too
        pyarray->length = (*env)->GetArrayLength(env, pyarray->object);
    }

    // ------------------------------ pinned array support
    // now, for primitive arrays we have to Release() the
    // array when we're done with it.

    pyjarray_pin(pyarray);

    if (zero && !PyErr_Occurred()) { // skip if we're not nulling the array

        switch (pyarray->componentType) {

        case JINT_ID: {
            int   i;
            long  v  = 0;
            jint *ar = (jint *) pyarray->pinnedArray;

            if (value && PyInt_Check(value)) {
                v = (long) PyInt_AS_LONG(value);
            }

            for (i = 0; i < pyarray->length; i++) {
                ar[i] = (jint) v;
            }

            break;
        }

        case JCHAR_ID: {
            int   i;
            long  v = 0;
            char  *val;
            jchar *ar = (jchar *) pyarray->pinnedArray;
            if (!value || !PyString_Check(value)) {
                if (value && PyInt_Check(value)) {
                    v = (long) PyInt_AS_LONG(value);
                }

                for (i = 0; i < pyarray->length; i++) {
                    ar[i] = (jchar) v;
                }
            } else {
                // it's a string, set the elements
                // we won't throw an error for index, length problems. just deal...

                val = PyString_AS_STRING(value);
                for (i = 0; i < pyarray->length && val[i] != '\0'; i++) {
                    ar[i] = (jchar) val[i];
                }
            }

            break;
        }

        case JBYTE_ID: {
            int    i;
            long   v  = 0;
            jbyte *ar = (jbyte *) pyarray->pinnedArray;

            if (value && PyInt_Check(value)) {
                v = (long) PyInt_AS_LONG(value);
            }

            for (i = 0; i < pyarray->length; i++) {
                ar[i] = (jbyte) v;
            }

            break;
        }

        case JLONG_ID: {
            int           i;
            PY_LONG_LONG  v  = 0;
            jlong   *ar = (jlong *) pyarray->pinnedArray;

            if (!value)
                ;
            else {
                if (PyLong_Check(value)) {
                    v = PyLong_AsLongLong(value);
                } else if (PyInt_Check(value)) {
                    v = PyInt_AS_LONG(value);
                }
            }

            for (i = 0; i < pyarray->length; i++) {
                ar[i] = (jlong) v;
            }

            break;
        }

        case JBOOLEAN_ID: {
            int       i;
            long      v  = 0;
            jboolean *ar = (jboolean *) pyarray->pinnedArray;

            if (value && PyInt_Check(value)) {
                v = (long) PyInt_AS_LONG(value);
            }

            for (i = 0; i < pyarray->length; i++) {
                if (v) {
                    ar[i] = JNI_TRUE;
                } else {
                    ar[i] = JNI_FALSE;
                }
            }

            break;
        }

        case JDOUBLE_ID: {
            int      i;
            double   v  = 0;
            jdouble *ar = (jdouble *) pyarray->pinnedArray;

            if (value && PyFloat_Check(value)) {
                v = PyFloat_AS_DOUBLE(value);
            }

            for (i = 0; i < pyarray->length; i++) {
                ar[i] = (jdouble) v;
            }

            break;
        }

        case JSHORT_ID: {
            int     i;
            long    v  = 0;
            jshort *ar = (jshort *) pyarray->pinnedArray;

            if (value && PyInt_Check(value)) {
                v  = (long) PyInt_AS_LONG(value);
            }

            for (i = 0; i < pyarray->length; i++) {
                ar[i] = (jshort) v;
            }

            break;
        }

        case JFLOAT_ID: {
            int     i;
            double  v  = 0;
            jfloat *ar = (jfloat *) pyarray->pinnedArray;

            if (value && PyFloat_Check(value)) {
                v = PyFloat_AS_DOUBLE(value);
            }

            for (i = 0; i < pyarray->length; i++) {
                ar[i] = (jfloat) v;
            }

            break;
        }

        } // switch
    } // if zero

    (*env)->DeleteLocalRef(env, compType);

    if (process_java_exception(env)) {
        return 0;
    }
    return 1;

EXIT_ERROR:
    if (compType) {
        (*env)->DeleteLocalRef(env, compType);
    }

    return -1;
}


// pin primitive array memory. NOOP for object arrays.
void pyjarray_pin(PyJArrayObject *self)
{
    JNIEnv *env = pyembed_get_env();

    switch (self->componentType) {

    case JINT_ID:
        self->pinnedArray = (*env)->GetIntArrayElements(
                                env,
                                self->object,
                                &(self->isCopy));
        break;

    case JCHAR_ID:
        self->pinnedArray = (*env)->GetCharArrayElements(
                                env,
                                self->object,
                                &(self->isCopy));
        break;

    case JBYTE_ID:
        self->pinnedArray = (*env)->GetByteArrayElements(
                                env,
                                self->object,
                                &(self->isCopy));
        break;

    case JLONG_ID:
        self->pinnedArray = (*env)->GetLongArrayElements(
                                env,
                                self->object,
                                &(self->isCopy));
        break;

    case JBOOLEAN_ID:
        self->pinnedArray = (*env)->GetBooleanArrayElements(
                                env,
                                self->object,
                                &(self->isCopy));
        break;

    case JDOUBLE_ID:
        self->pinnedArray = (*env)->GetDoubleArrayElements(
                                env,
                                self->object,
                                &(self->isCopy));
        break;

    case JSHORT_ID:
        self->pinnedArray = (*env)->GetShortArrayElements(
                                env,
                                self->object,
                                &(self->isCopy));
        break;

    case JFLOAT_ID:
        self->pinnedArray = (*env)->GetFloatArrayElements(
                                env,
                                self->object,
                                &(self->isCopy));
        break;

    } // switch

    process_java_exception(env);
}


static void pyjarray_dealloc(PyJArrayObject *self)
{
#if USE_DEALLOC
    JNIEnv *env = pyembed_get_env();
    if (env) {
        if (self->clazz) {
            (*env)->DeleteGlobalRef(env, self->clazz);
        }
        if (self->componentClass) {
            (*env)->DeleteGlobalRef(env, self->componentClass);
        }

        // can't guarantee mode 0 will work in this case...
        pyjarray_release_pinned(self, JNI_ABORT);

        // pyjarray_release_pinned potentially uses self->object so we can
        // only delete self->object afterwards
        if (self->object) {
            (*env)->DeleteGlobalRef(env, self->object);
        }
    } // if env

    PyObject_Del(self);
#endif
}


// used to either release pinned memory, commit, or abort.
void pyjarray_release_pinned(PyJArrayObject *self, jint mode)
{
    JNIEnv *env = pyembed_get_env();

    if (!self->pinnedArray) {
        return;
    }

    // don't release if it's the raw data, but do release if it's a copy
    if (!self->isCopy && mode == JNI_ABORT) {
        return;
    }

    switch (self->componentType) {

    case JINT_ID:
        (*env)->ReleaseIntArrayElements(env,
                                        self->object,
                                        (jint *) self->pinnedArray,
                                        mode);
        break;

    case JCHAR_ID:
        (*env)->ReleaseCharArrayElements(env,
                                         self->object,
                                         (jchar *) self->pinnedArray,
                                         mode);
        break;

    case JBYTE_ID:
        (*env)->ReleaseByteArrayElements(env,
                                         self->object,
                                         (jbyte *) self->pinnedArray,
                                         mode);
        break;

    case JLONG_ID:
        (*env)->ReleaseLongArrayElements(env,
                                         self->object,
                                         (jlong *) self->pinnedArray,
                                         mode);
        break;

    case JBOOLEAN_ID:
        (*env)->ReleaseBooleanArrayElements(env,
                                            self->object,
                                            (jboolean *) self->pinnedArray,
                                            mode);
        break;

    case JDOUBLE_ID:
        (*env)->ReleaseDoubleArrayElements(env,
                                           self->object,
                                           (jdouble *) self->pinnedArray,
                                           mode);
        break;

    case JSHORT_ID:
        (*env)->ReleaseShortArrayElements(env,
                                          self->object,
                                          (jshort *) self->pinnedArray,
                                          mode);
        break;

    case JFLOAT_ID:
        (*env)->ReleaseFloatArrayElements(env,
                                          self->object,
                                          (jfloat *) self->pinnedArray,
                                          mode);
        break;

    } // switch
}


int pyjarray_check(PyObject *obj)
{
    if (PyObject_TypeCheck(obj, &PyJArray_Type)) {
        return 1;
    }
    return 0;
}


static int pyjarray_setitem(PyJArrayObject *self,
                            int pos,
                            PyObject *newitem)
{

    JNIEnv *env = pyembed_get_env();

    if (pos < 0 || pos >= self->length || self->length < 1) {
        PyErr_Format(PyExc_IndexError,
                     "array assignment index out of range: %i", pos);
        return -1;
    }

    // first, do the object types.

    switch (self->componentType) {

    case JSTRING_ID: {
        jstring  jstr = NULL;
        char    *val;

        if (newitem == Py_None)
            ; // setting NULL
        else {
            if (!PyString_Check(newitem)) {
                PyErr_SetString(PyExc_TypeError, "Expected string.");
                return -1;
            }

            val  = PyString_AS_STRING(newitem);
            jstr = (*env)->NewStringUTF(env, (const char *) val);
        }

        (*env)->SetObjectArrayElement(env,
                                      self->object,
                                      pos,
                                      jstr);
        (*env)->DeleteLocalRef(env, jstr);
        if (process_java_exception(env)) {
            return -1;
        }
        return 0;
    }

    case JOBJECT_ID: {
        jobject    obj = NULL;
        PyJObject *pyjob;

        if (newitem == Py_None)
            ; // setting NULL
        else {
            if (!pyjobject_check(newitem)) {
                PyErr_SetString(PyExc_TypeError, "Expected jobject.");
                return -1;
            }

            pyjob = (PyJObject *) newitem;
            obj = pyjob->object;

            if (!obj) {
                PyErr_SetString(PyExc_TypeError, "Expected instance, not class.");
                return -1;
            }
        }

        (*env)->SetObjectArrayElement(env,
                                      self->object,
                                      pos,
                                      obj);
        if (process_java_exception(env)) {
            return -1;
        }
        return 0;
    }

    case JARRAY_ID: {
        jobject          obj = NULL;
        PyJArrayObject *pyarray;

        if (newitem == Py_None)
            ; // setting NULL
        else {
            if (!pyjarray_check(newitem)) {
                PyErr_SetString(PyExc_TypeError, "Expected jarray.");
                return -1;
            }

            pyarray = (PyJArrayObject *) newitem;
            obj = pyarray->object;
        }

        (*env)->SetObjectArrayElement(env,
                                      self->object,
                                      pos,
                                      obj);
        if (process_java_exception(env)) {
            return -1;
        }
        return 0;
    }

    } // switch

    // ------------------------------ primitive types

    if (!self->pinnedArray) {
        PyErr_SetString(PyExc_RuntimeError, "Pinned array shouldn't be null.");
        return -1;
    }

    switch (self->componentType) {

    case JINT_ID:
        if (!PyInt_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected int.");
            return -1;
        }

        ((jint *) self->pinnedArray)[pos] = (jint) PyInt_AS_LONG(newitem);
        return 0; /* success */

    case JBYTE_ID:
        if (!PyInt_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected byte.");
            return -1;
        }

        ((jbyte *) self->pinnedArray)[pos] = (jbyte) PyInt_AS_LONG(newitem);
        return 0; /* success */

    case JCHAR_ID:
        if (PyInt_Check(newitem)) {
            ((jchar *) self->pinnedArray)[pos] = (jchar) PyInt_AS_LONG(newitem);
        } else if (PyString_Check(newitem) && PyString_GET_SIZE(newitem) == 1) {
            char *val = PyString_AS_STRING(newitem);
            ((jchar *) self->pinnedArray)[pos] = (jchar) val[0];
        } else {
            PyErr_SetString(PyExc_TypeError, "Expected char.");
            return -1;
        }

        return 0; /* success */

    case JLONG_ID:
        if (!PyLong_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected long.");
            return -1;
        }

        ((jlong *) self->pinnedArray)[pos] = (jlong) PyLong_AsLongLong(newitem);
        return 0; /* success */

    case JBOOLEAN_ID:
        if (!PyInt_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected boolean.");
            return -1;
        }

        if (PyInt_AS_LONG(newitem)) {
            ((jboolean *) self->pinnedArray)[pos] = JNI_TRUE;
        } else {
            ((jboolean *) self->pinnedArray)[pos] = JNI_FALSE;
        }

        return 0; /* success */

    case JDOUBLE_ID:
        if (!PyFloat_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected float.");
            return -1;
        }

        ((jdouble *) self->pinnedArray)[pos] =
            (jdouble) PyFloat_AS_DOUBLE(newitem);
        return 0; /* success */

    case JSHORT_ID:
        if (!PyInt_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected int.");
            return -1;
        }

        ((jshort *) self->pinnedArray)[pos] =
            (jshort) PyInt_AS_LONG(newitem);
        return 0; /* success */

    case JFLOAT_ID:
        if (!PyFloat_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected float.");
            return -1;
        }

        ((jfloat *) self->pinnedArray)[pos] =
            (jfloat) PyFloat_AS_DOUBLE(newitem);
        return 0; /* success */

    } // switch

    PyErr_SetString(PyExc_TypeError, "Unknown type.");
    return -1;
}


static PyObject* pyjarray_item(PyJArrayObject *self, Py_ssize_t pos)
{
    PyObject *ret = NULL;
    JNIEnv   *env = pyembed_get_env();

    if (self->length < 1) {
        PyErr_Format(PyExc_IndexError,
                     "array assignment index out of range: %zd", pos);
        return NULL;
    }

    if (pos < 0) {
        pos = 0;
    }
    if (pos >= self->length) {
        pos = self->length - 1;
    }

    switch (self->componentType) {

    case JSTRING_ID: {
        jstring     jstr;
        const char *str;

        jstr = (jstring) (*env)->GetObjectArrayElement(env,
                self->object,
                (jsize) pos);

        if (process_java_exception(env))
            ;
        else if (jstr != NULL) {
            str = (*env)->GetStringUTFChars(env, jstr, 0);
            ret = PyString_FromString((char *) str);

            (*env)->ReleaseStringUTFChars(env, jstr, str);
            (*env)->DeleteLocalRef(env, jstr);
        } else {
            // no error occurred, just return None
            Py_INCREF(Py_None);
            ret = Py_None;
        }

        break;
    }

    case JARRAY_ID: {
        jobjectArray obj;

        obj = (jobjectArray) (*env)->GetObjectArrayElement(env,
                self->object,
                (jsize) pos);

        if (process_java_exception(env))
            ;
        else if (obj != NULL) {
            ret = pyjarray_new(env, obj);
        } else {
            // null is okay
            Py_INCREF(Py_None);
            ret = Py_None;
        }

        break;
    }

    case JOBJECT_ID: {
        jobject obj;

        obj = (*env)->GetObjectArrayElement(env,
                                            self->object,
                                            (jsize) pos);
        if (process_java_exception(env))
            ;
        else if (obj != NULL) {
            ret = convert_jobject_pyobject(env, obj);
            (*env)->DeleteLocalRef(env, obj);
        } else {
            // null is okay
            Py_INCREF(Py_None);
            ret = Py_None;
        }

        break;
    }

    case JBOOLEAN_ID:
        ret = Py_BuildValue("i", ((jboolean *) self->pinnedArray)[(jsize) pos]);
        break;

    case JSHORT_ID:
        ret = Py_BuildValue("i", ((jshort *) self->pinnedArray)[(jsize) pos]);
        break;

    case JINT_ID:
        ret = Py_BuildValue("i", ((jint *) self->pinnedArray)[(jsize) pos]);
        break;

    case JBYTE_ID:
        ret = Py_BuildValue("i", ((jbyte *) self->pinnedArray)[(jsize) pos]);
        break;

    case JCHAR_ID: {
        char val[2];
        val[0] = (char) ((jchar *) self->pinnedArray)[pos];
        val[1] = '\0';
        ret = PyString_FromString(val);
        break;
    }

    case JLONG_ID:
        ret = PyLong_FromLongLong(((jlong *) self->pinnedArray)[(jsize) pos]);
        break;

    case JFLOAT_ID:
        ret = PyFloat_FromDouble(((jfloat *) self->pinnedArray)[(jsize) pos]);
        break;

    case JDOUBLE_ID:
        ret = PyFloat_FromDouble(((jdouble *) self->pinnedArray)[(jsize) pos]);
        break;

    default:
        PyErr_Format(PyExc_TypeError, "Unknown type %i.",
                     self->componentType);
    }

    return ret;
}


static int pyjarray_index(PyJArrayObject *self, PyObject *el)
{
    JNIEnv *env = pyembed_get_env();

    switch (self->componentType) {

    case JSTRING_ID: {
        int i, ret = 0;

        if (el != Py_None && !PyString_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected str.");
            return -1;
        }

        for (i = 0; ret == 0 && i < self->length; i++) {
            const char *val;
            PyObject   *t;
            jstring l = (*env)->GetObjectArrayElement(env,
                        self->object,
                        i);
            if (l == NULL) {
                if (el == Py_None) {
                    return i;
                }

                (*env)->DeleteLocalRef(env, l);
                continue;
            }

            val = jstring2char(env, l);
            t   = PyString_FromString((char *) val);

            ret = PyObject_RichCompareBool(el,
                                           t,
                                           Py_EQ);

            Py_DECREF(t);
            release_utf_char(env, l, val);
            (*env)->DeleteLocalRef(env, l);

            if (ret) {
                return i;
            }
        }

        return -1;
    }

    case JARRAY_ID: {
        PyJArrayObject *obj;
        int i, ret = 0;

        JNIEnv *env = pyembed_get_env();

        if (el != Py_None && !pyjarray_check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected jarray.");
            return -1;
        }

        obj = (PyJArrayObject *) el;
        for (i = 0; !ret && i < self->length; i++) {
            jobject l = (*env)->GetObjectArrayElement(env,
                        self->object,
                        i);
            if (l == NULL) {
                if (el == Py_None) {
                    return i;
                }

                (*env)->DeleteLocalRef(env, l);
                continue;
            }

            if ((*env)->IsSameObject(env, l, obj->object)) {
                ret = 1;
            }

            (*env)->DeleteLocalRef(env, l);

            if (ret) {
                return i;
            }
        }

        return -1;
    }

    case JOBJECT_ID: {
        PyJObject *obj;
        int i, ret = 0;

        JNIEnv *env = pyembed_get_env();

        if (el != Py_None && !pyjobject_check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected jobject.");
            return -1;
        }

        obj = (PyJObject *) el;
        for (i = 0; !ret && i < self->length; i++) {
            jobject l = (*env)->GetObjectArrayElement(env,
                        self->object,
                        i);

            if (l == NULL) {
                if (el == Py_None) {
                    return i;
                }

                (*env)->DeleteLocalRef(env, l);
                continue;
            }

            if ((*env)->IsSameObject(env, l, obj->object)) {
                ret = 1;
            }

            (*env)->DeleteLocalRef(env, l);

            if (ret) {
                return i;
            }
        }

        return -1;
    }

    case JBOOLEAN_ID: {
        jboolean *ar = (jboolean *) self->pinnedArray;
        int       i;
        jboolean  v;

        if (!PyInt_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected boolean.");
            return -1;
        }

        if (PyInt_AS_LONG(el)) {
            v = JNI_TRUE;
        } else {
            v = JNI_FALSE;
        }

        for (i = 0; i < self->length; i++) {
            if (v == ar[i]) {
                return i;
            }
        }

        return -1;
    }

    case JSHORT_ID: {
        jshort *ar = (jshort *) self->pinnedArray;
        int     i;
        jshort  v;

        if (!PyInt_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected int (short).");
            return -1;
        }

        v = (jshort) PyInt_AS_LONG(el);
        for (i = 0; i < self->length; i++) {
            if (v == ar[i]) {
                return i;
            }
        }

        return -1;
    }

    case JINT_ID: {
        jint *ar = (jint *) self->pinnedArray;
        int   i;
        jint  v;

        if (!PyInt_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected int.");
            return -1;
        }

        v = (jint) PyInt_AS_LONG(el);
        for (i = 0; i < self->length; i++) {
            if (v == ar[i]) {
                return i;
            }
        }

        return -1;
    }

    case JBYTE_ID: {
        jbyte *ar = (jbyte *) self->pinnedArray;
        int    i;
        jbyte  v;

        if (!PyInt_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected byte.");
            return -1;
        }

        v = (jbyte) PyInt_AS_LONG(el);
        for (i = 0; i < self->length; i++) {
            if (v == ar[i]) {
                return i;
            }
        }

        return -1;
    }

    case JCHAR_ID: {
        jchar *ar = (jchar *) self->pinnedArray;
        int    i;
        jchar  v;

        if (PyInt_Check(el)) {
            v = (jchar) PyInt_AS_LONG(el);
        } else if (PyString_Check(el) && PyString_GET_SIZE(el) == 1) {
            char *val = PyString_AS_STRING(el);
            v = (jchar) val[0];
        } else {
            PyErr_SetString(PyExc_TypeError, "Expected char.");
            return -1;
        }

        for (i = 0; i < self->length; i++) {
            if (v == ar[i]) {
                return i;
            }
        }

        return -1;
    }

    case JLONG_ID: {
        jlong *ar = (jlong *) self->pinnedArray;
        int    i;
        jlong  v;

        if (!PyLong_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected long.");
            return -1;
        }

        v = (jlong) PyLong_AsLongLong(el);
        for (i = 0; i < self->length; i++) {
            if (v == ar[i]) {
                return i;
            }
        }

        return -1;
    }

    case JFLOAT_ID: {
        jfloat *ar = (jfloat *) self->pinnedArray;
        int     i;
        jfloat  v;

        if (!PyFloat_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected long.");
            return -1;
        }

        v = (jfloat) PyFloat_AsDouble(el);
        for (i = 0; i < self->length; i++) {
            if (v == ar[i]) {
                return i;
            }
        }

        return -1;
    }

    case JDOUBLE_ID: {
        jdouble *ar = (jdouble *) self->pinnedArray;
        int      i;
        jdouble  v;

        if (!PyFloat_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected long.");
            return -1;
        }

        v = (jdouble) PyFloat_AsDouble(el);
        for (i = 0; i < self->length; i++) {
            if (v == ar[i]) {
                return i;
            }
        }

        return -1;
    }

    default:
        PyErr_Format(PyExc_RuntimeError, "Unknown type %i.",
                     self->componentType);
    } // switch

    return -1; // error, shouldn't happen
}


static PyObject* listindex(PyJArrayObject *self, PyObject *args)
{
    int pos;
    PyObject *v;

    if (!PyArg_ParseTuple(args, "O", &v)) {
        return NULL;
    }

    pos = pyjarray_index(self, v);
    if (PyErr_Occurred()) {
        return NULL;
    }

    if (pos >= 0) {
        return PyInt_FromLong((long) pos);
    }

    PyErr_SetString(PyExc_ValueError, "list.index(x): x not in array");
    return NULL;
}


static PyObject* pyjarray_commit(PyJArrayObject *self, PyObject *args)
{
    PyObject *v;

    if (!PyArg_ParseTuple(args, "", &v)) {
        return NULL;
    }

    pyjarray_release_pinned(self, JNI_COMMIT);

    Py_RETURN_NONE;
}


static int pyjarray_contains(PyJArrayObject *self, PyObject *el)
{
    int pos = pyjarray_index(self, el);
    if (PyErr_Occurred()) {
        return -1;
    }

    if (pos >= 0) {
        return 1;
    }
    return 0;
}


// shamelessly taken from listobject.c
static PyObject* pyjarray_slice(PyObject *_self, Py_ssize_t ilow,
                                Py_ssize_t ihigh)
{
    PyJArrayObject *pyarray  = NULL;
    jobjectArray     arrayObj = NULL;
    PyObject        *ret      = NULL;

    PyJArrayObject *self = (PyJArrayObject *) _self;

    Py_ssize_t len, i;
    JNIEnv *env = pyembed_get_env();

    if (ilow < 0) {
        ilow = 0;
    } else if (ilow > self->length) {
        ilow = self->length;
    }
    if (ihigh < ilow) {
        ihigh = ilow;
    } else if (ihigh > self->length) {
        ihigh = self->length;
    }
    len = ihigh - ilow;

    switch (self->componentType) {
    case JOBJECT_ID:
        arrayObj = (*env)->NewObjectArray(env,
                                          (jsize) len,
                                          self->componentClass,
                                          NULL);
        break;

    case JARRAY_ID:
        arrayObj = (*env)->NewObjectArray(env,
                                          (jsize) len,
                                          self->componentClass,
                                          NULL);
        break;

    case JSTRING_ID:
        arrayObj = (*env)->NewObjectArray(env,
                                          (jsize) len,
                                          JSTRING_TYPE,
                                          NULL);

        break;

    case JINT_ID: {
        jint *ar, *src;
        arrayObj = (*env)->NewIntArray(env, (jsize) len);
        pyarray  = (PyJArrayObject *) pyjarray_new(env, arrayObj);
        if (PyErr_Occurred()) {
            break;
        }

        ar  = (jint *) pyarray->pinnedArray;
        src = (jint *) self->pinnedArray;
        for (i = 0; i < len; i++) {
            ar[i] = src[ilow++];
        }

        ret = (PyObject *) pyarray;
        break;
    }

    case JBYTE_ID: {
        jbyte *ar, *src;
        arrayObj = (*env)->NewByteArray(env, (jsize) len);
        pyarray  = (PyJArrayObject *) pyjarray_new(env, arrayObj);
        if (PyErr_Occurred()) {
            break;
        }

        ar  = (jbyte *) pyarray->pinnedArray;
        src = (jbyte *) self->pinnedArray;
        for (i = 0; i < len; i++) {
            ar[i] = src[ilow++];
        }

        ret = (PyObject *) pyarray;
        break;
    }

    case JCHAR_ID: {
        jchar *ar, *src;
        arrayObj = (*env)->NewCharArray(env, (jsize) len);
        pyarray  = (PyJArrayObject *) pyjarray_new(env, arrayObj);
        if (PyErr_Occurred()) {
            break;
        }

        ar  = (jchar *) pyarray->pinnedArray;
        src = (jchar *) self->pinnedArray;
        for (i = 0; i < len; i++) {
            ar[i] = src[ilow++];
        }

        ret = (PyObject *) pyarray;
        break;
    }

    case JLONG_ID: {
        jlong *ar, *src;
        arrayObj = (*env)->NewLongArray(env, (jsize) len);
        pyarray  = (PyJArrayObject *) pyjarray_new(env, arrayObj);
        if (PyErr_Occurred()) {
            break;
        }

        ar  = (jlong *) pyarray->pinnedArray;
        src = (jlong *) self->pinnedArray;
        for (i = 0; i < len; i++) {
            ar[i] = src[ilow++];
        }

        ret = (PyObject *) pyarray;
        break;
    }

    case JBOOLEAN_ID: {
        jboolean *ar, *src;
        arrayObj = (*env)->NewBooleanArray(env, (jsize) len);
        pyarray  = (PyJArrayObject *) pyjarray_new(env, arrayObj);
        if (PyErr_Occurred()) {
            break;
        }

        ar  = (jboolean *) pyarray->pinnedArray;
        src = (jboolean *) self->pinnedArray;
        for (i = 0; i < len; i++) {
            ar[i] = src[ilow++];
        }

        ret = (PyObject *) pyarray;
        break;
    }

    case JDOUBLE_ID: {
        jdouble *ar, *src;
        arrayObj = (*env)->NewDoubleArray(env, (jsize) len);
        pyarray  = (PyJArrayObject *) pyjarray_new(env, arrayObj);
        if (PyErr_Occurred()) {
            break;
        }

        ar  = (jdouble *) pyarray->pinnedArray;
        src = (jdouble *) self->pinnedArray;
        for (i = 0; i < len; i++) {
            ar[i] = src[ilow++];
        }

        ret = (PyObject *) pyarray;
        break;
    }

    case JSHORT_ID: {
        jshort *ar, *src;
        arrayObj = (*env)->NewShortArray(env, (jsize) len);
        pyarray  = (PyJArrayObject *) pyjarray_new(env, arrayObj);
        if (PyErr_Occurred()) {
            break;
        }

        ar  = (jshort *) pyarray->pinnedArray;
        src = (jshort *) self->pinnedArray;
        for (i = 0; i < len; i++) {
            ar[i] = src[ilow++];
        }

        ret = (PyObject *) pyarray;
        break;
    }

    case JFLOAT_ID: {
        jfloat *ar, *src;
        arrayObj = (*env)->NewFloatArray(env, (jsize) len);
        pyarray  = (PyJArrayObject *) pyjarray_new(env, arrayObj);
        if (PyErr_Occurred()) {
            break;
        }

        ar  = (jfloat *) pyarray->pinnedArray;
        src = (jfloat *) self->pinnedArray;
        for (i = 0; i < len; i++) {
            ar[i] = src[ilow++];
        }

        ret = (PyObject *) pyarray;
        break;
    }

    } // switch

    if (self->componentType == JOBJECT_ID ||
            self->componentType == JSTRING_ID ||
            self->componentType == JARRAY_ID) {

        // all object operations. have to be handled differently...
        // *sigh*
        for (i = 0; i < len; i++) {
            jobject obj = (*env)->GetObjectArrayElement(env,
                          self->object,
                          (jsize) ilow++);
            (*env)->SetObjectArrayElement(env,
                                          arrayObj,
                                          (jsize) i,
                                          obj);
            if (obj) {
                (*env)->DeleteLocalRef(env, obj);
            }
        }

        ret = pyjarray_new(env, arrayObj);
    }

    if (arrayObj) {
        (*env)->DeleteLocalRef(env, arrayObj);
    }

    if (!ret && !PyErr_Occurred()) {
        PyErr_SetString(PyExc_ValueError, "Unsupported type.");
    }

    return ret;
}


// shamelessly taken from listobject.c
static PyObject* pyjarray_subscript(PyJArrayObject *self, PyObject *item)
{
    if (PyInt_Check(item)) {
        long i = (long) PyInt_AS_LONG(item);
        if (i < 0) {
            i += self->length;
        }
        return pyjarray_item(self, (Py_ssize_t) i);
    } else if (PyLong_Check(item)) {
        long i = PyLong_AsLong(item);
        if (i == -1 && PyErr_Occurred()) {
            return NULL;
        }
        if (i < 0) {
            i += self->length;
        }
        return pyjarray_item(self, (Py_ssize_t) i);
    } else if (PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slicelength;

#if PY_MAJOR_VERSION >= 3
        if (PySlice_GetIndicesEx(item, pyjarray_length((PyObject*) self), &start, &stop,
                                 &step, &slicelength) < 0) {
            // error will already be set
            return NULL;
        }
#else
        /*
         * This silences a compile warning on PySlice_GetIndicesEx by casting
         * item.  Python fixed the method signature in 3.2 to take item as a
         * PyObject*
         */
        if (PySlice_GetIndicesEx((PySliceObject *) item,
                                 pyjarray_length((PyObject*) self), &start, &stop, &step, &slicelength) < 0) {
            // error will already be set
            return NULL;
        }
#endif

        if (slicelength <= 0) {
            return pyjarray_slice((PyObject*) self, 0, 0);
        } else if (step != 1) {
            PyErr_SetString(PyExc_TypeError, "pyjarray slices must have step of 1");
            return NULL;
        } else {
            return pyjarray_slice((PyObject*) self, start, stop);
        }
    } else {
        PyErr_SetString(PyExc_TypeError,
                        "pyjarray indices must be integers, longs, or slices");
        return NULL;
    }
}


static PyObject* pyjarray_str(PyJArrayObject *self)
{
    PyObject *ret;
#if PY_MAJOR_VERSION >= 3
    JNIEnv   *env = pyembed_get_env();

    ret = jobject_topystring(env, self->object);
    return ret;
#else
    // retained to not break former behavior
    if (!self->pinnedArray) {
        PyErr_SetString(PyExc_RuntimeError, "No pinned array.");
        return NULL;
    }

    switch (self->componentType) {
    case JBYTE_ID:
        ret = PyBytes_FromStringAndSize((const char *) self->pinnedArray,
                                        self->length);
        return ret;

    default:
        PyErr_SetString(PyExc_TypeError,
                        "Unsupported type for str operation.");
        return NULL;
    }
#endif
}


// -------------------------------------------------- sequence methods

static Py_ssize_t pyjarray_length(PyObject *self)
{
    if (self && pyjarray_check(self)) {
        return ((PyJArrayObject *) self)->length;
    }
    return 0;
}


PyDoc_STRVAR(list_doc,
             "jarray(size) -> new jarray of size");
PyDoc_STRVAR(getitem_doc,
             "x.__getitem__(y) <==> x[y]");
PyDoc_STRVAR(index_doc,
             "L.index(value) -> integer -- return first index of value");
PyDoc_STRVAR(commit_doc,
             "x.commit() -- commit pinned array to Java memory");

PyMethodDef pyjarray_methods[] = {
    {
        "__getitem__",
        (PyCFunction) pyjarray_subscript, METH_O, getitem_doc
    },

    {"index", (PyCFunction) listindex, METH_VARARGS, index_doc},

    {"commit", (PyCFunction) pyjarray_commit, METH_VARARGS, commit_doc},

    { NULL, NULL }
};


static PySequenceMethods list_as_sequence = {
    (lenfunc) pyjarray_length,                /* sq_length */
    (binaryfunc) 0,                           /* sq_concat */
    (ssizeargfunc) 0,                         /* sq_repeat */
    (ssizeargfunc) pyjarray_item,             /* sq_item */
    (ssizessizeargfunc) pyjarray_slice,       /* sq_slice */
    (ssizeobjargproc) pyjarray_setitem,       /* sq_ass_item */
    (ssizessizeobjargproc) 0,                 /* sq_ass_slice */
    (objobjproc) pyjarray_contains,           /* sq_contains */
    (binaryfunc) 0,                           /* sq_inplace_concat */
    (ssizeargfunc) 0,                         /* sq_inplace_repeat */
};

static PyMappingMethods pyjarray_map_methods = {
    (lenfunc) pyjarray_length,                /* mp_length */
    (binaryfunc) pyjarray_subscript,          /* mp_subscript */
    0,                                        /* mp_ass_subscript */
};


static PyObject* pyjarray_iter(PyObject *);


PyTypeObject PyJArray_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJArray",                           /* tp_name */
    sizeof(PyJArrayObject),                   /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor) pyjarray_dealloc,            /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    &list_as_sequence,                        /* tp_as_sequence */
    &pyjarray_map_methods,                    /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    (reprfunc) pyjarray_str,                  /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_HAVE_ITER,                     /* tp_flags */
    list_doc,                                 /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    pyjarray_iter,                            /* tp_iter */
    0,                                        /* tp_iternext */
    pyjarray_methods,                         /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};


/*********************** List Iterator **************************/

// shamelessly copied from listobject.c

// there are generic iterators, but they kinda suck...

typedef struct {
    PyObject_HEAD
    long it_index;
    PyJArrayObject *it_seq; /* Set to NULL when iterator is exhausted */
} PyJArrayIterObject;

PyTypeObject PyJArrayIter_Type;

static PyObject *pyjarray_iter(PyObject *seq)
{
    PyJArrayIterObject *it;

    if (PyType_Ready(&PyJArrayIter_Type) < 0) {
        return NULL;
    }

    if (!pyjarray_check(seq)) {
        PyErr_BadInternalCall();
        return NULL;
    }
    it = PyObject_New(PyJArrayIterObject, &PyJArrayIter_Type);
    if (it == NULL) {
        return NULL;
    }
    it->it_index = 0;
    Py_INCREF(seq);
    it->it_seq = (PyJArrayObject *) seq;
    return (PyObject *)it;
}

static void pyjarrayiter_dealloc(PyJArrayIterObject *it)
{
    Py_XDECREF(it->it_seq);
    PyObject_Del(it);
}

static PyObject *pyjarrayiter_next(PyJArrayIterObject *it)
{
    PyJArrayObject *seq;
    PyObject *item;

    assert(it != NULL);
    seq = it->it_seq;
    if (seq == NULL) {
        return NULL;
    }

    if (it->it_index < seq->length) {
        item = (PyObject *) pyjarray_item(seq, it->it_index);
        ++it->it_index;
        return item;
    }

    Py_DECREF(seq);
    it->it_seq = NULL;
    return NULL;
}

static int pyjarrayiter_len(PyJArrayIterObject *it)
{
    Py_ssize_t len;
    if (it->it_seq) {
        len = (int) (it->it_seq->length - it->it_index);
        if (len >= 0) {
            return (int) len;
        }
    }
    return 0;
}

static PySequenceMethods pyjarrayiter_as_sequence = {
    (lenfunc) pyjarrayiter_len,               /* sq_length */
    0,                                        /* sq_concat */
};

PyObject* pyjarrayiter_getattr(PyObject *one, PyObject *two)
{
    return PyObject_GenericGetAttr(one, two);
}

PyTypeObject PyJArrayIter_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJArrayIter",                       /* tp_name */
    sizeof(PyJArrayIterObject),               /* tp_basicsize */
    0,                                        /* tp_itemsize */
    /* methods */
    (destructor) pyjarrayiter_dealloc,        /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    &pyjarrayiter_as_sequence,                /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    (getattrofunc) pyjarrayiter_dealloc,      /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_HAVE_ITER,                     /* tp_flags */
    0,                                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    PyObject_SelfIter,                        /* tp_iter */
    (iternextfunc) pyjarrayiter_next,         /* tp_iternext */
    0,                                        /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
};
