/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/* 
   jep - Java Embedded Python

   Copyright (C) 2004 Mike Johnson

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
   
   
   pyjarray is a python object for using java arrays within the interpreter.
   i got a lot of inspiration from listobject.c in the Objects/ folder
   of the python distribution.
*/     

#ifdef WIN32
# include "winconfig.h"
#endif

#if HAVE_CONFIG_H
# include <config.h>
#endif

// shut up the compiler
#ifdef _POSIX_C_SOURCE
#  undef _POSIX_C_SOURCE
#endif
#include <jni.h>

// shut up the compiler
#ifdef _POSIX_C_SOURCE
#  undef _POSIX_C_SOURCE
#endif
#ifdef _FILE_OFFSET_BITS
# undef _FILE_OFFSET_BITS
#endif
#include "Python.h"

#include "pyjarray.h"
#include "pyjobject.h"
#include "pyembed.h"
#include "util.h"

extern PyTypeObject PyJarray_Type;
extern PyMethodDef  pyjarray_methods[];


jmethodID objectComponentType = 0;


static void pyjarray_dealloc(PyJarray_Object *self);
static int pyjarray_init(PyJarray_Object*, int, PyObject*);
static int pyjarray_length(PyJarray_Object *self);
static void pyjarray_release_pinned(PyJarray_Object *self, jint mode);


// called internally to make new PyJarray_Object instances
PyObject* pyjarray_new(JNIEnv *env, jobjectArray obj) {
    PyJarray_Object *pyarray;
    jclass           clazz;
    
    if(PyType_Ready(&PyJarray_Type) < 0)
        return NULL;
    if(!obj) {
        PyErr_Format(PyExc_RuntimeError, "Invalid array object.");
        return NULL;
    }
    
    clazz = (*env)->GetObjectClass(env, obj);
    if(process_java_exception(env) || !clazz)
        return NULL;
    
    pyarray                = PyObject_NEW(PyJarray_Object, &PyJarray_Type);
    pyarray->object        = (*env)->NewGlobalRef(env, obj);
    pyarray->clazz         = (*env)->NewGlobalRef(env, clazz);
    pyarray->env           = env;
    pyarray->componentType = -1;
    pyarray->length        = -1;
    pyarray->pinnedArray   = NULL;
    
    if(pyjarray_init(pyarray, 0, NULL))
        return (PyObject *) pyarray;
    else {
        pyjarray_dealloc(pyarray);
        return NULL;
    }
}


// called from module to create new arrays.
// args are variable, should accept:
// (size, typeid, [value]), (size, jobject),
//     (size, string), (size, pyjarray), (list)
PyObject* pyjarray_new_v(PyObject *isnull, PyObject *args) {
    PyJarray_Object *pyarray;
    jclass           clazz;
    JNIEnv          *env       = NULL;
    PyObject        *_env      = NULL;
    jobjectArray     arrayObj  = NULL;
    int              typeId    = -1;
    long             size      = -1;
    
    // args
    PyObject *one, *two, *three;
    one = two = three = NULL;
    
    if(PyType_Ready(&PyJarray_Type) < 0)
        return NULL;
    
    _env = pyembed_getthread_object(LIST_ENV);
    if(!_env || !PyCObject_Check(_env)) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_ValueError, "Invalid env pointer.");
        return NULL;
    }
    
    env = (JNIEnv *) PyCObject_AsVoidPtr((PyObject *) _env);
    if(!env) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_ValueError,
                            "Invalid env pointer, AsVoidPtr returned NULL.");
        return NULL;
    }
    
    if(!PyArg_UnpackTuple(args, "ref", 1, 3, &one, &two, &three))
        return NULL;

    if(PyInt_Check(one)) {
        size = PyInt_AsLong(one);

        if(PyInt_Check(two)) {
            typeId = PyInt_AsLong(two);
        
            if(size < 0)
                return PyErr_Format(PyExc_ValueError, "Invalid size %i", size);

            // make a new primitive array
            switch(typeId) {
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
            } // switch
            
        } // if int(two)
        else if(PyString_Check(two)) {
            jstring  jstr;
            char    *val;
            
            typeId = JSTRING_ID;
            val    = PyString_AsString(two);
            jstr   = (*env)->NewStringUTF(env, (const char *) val);
            
            arrayObj = (*env)->NewObjectArray(env,
                                              (jsize) size,
                                              JSTRING_TYPE,
                                              jstr);
        }
        else if(pyjobject_check(two)) {
            PyJobject_Object *pyjob = (PyJobject_Object *) two;
            typeId = JOBJECT_ID;
            
            arrayObj = (*env)->NewObjectArray(env,
                                              (jsize) size,
                                              pyjob->clazz,
                                              pyjob->object);
        }
        else if(pyjarray_check(two)) {
            PyJarray_Object *pyarray = (PyJarray_Object *) two;
            typeId = JARRAY_ID;
            
            arrayObj = (*env)->NewObjectArray(env,
                                              (jsize) size,
                                              pyarray->clazz,
                                              pyarray->object);
        }
        else {
            PyErr_SetString(PyExc_ValueError, "Unknown arg type: expected "
                            "one of: J<foo>_ID, pyjobject, jarray");
            return NULL;
        }
    }
    else {
        PyErr_SetString(PyExc_ValueError, "Unknown arg types.");
        return NULL;
    }

    if(process_java_exception(env))
        return NULL;
    
    if(!arrayObj || typeId < -1 || size < -1) {
        PyErr_SetString(PyExc_ValueError, "Unknown type.");
        return NULL;
    }
    
    clazz = (*env)->GetObjectClass(env, arrayObj);
    if(process_java_exception(env) || !clazz)
        return NULL;
    
    pyarray                = PyObject_NEW(PyJarray_Object, &PyJarray_Type);
    pyarray->object        = (*env)->NewGlobalRef(env, arrayObj);
    pyarray->clazz         = (*env)->NewGlobalRef(env, clazz);
    pyarray->env           = env;
    pyarray->componentType = typeId;
    pyarray->length        = -1;
    pyarray->pinnedArray   = NULL;
    
    (*env)->DeleteLocalRef(env, arrayObj);
    (*env)->DeleteLocalRef(env, clazz);

    if(pyjarray_init(pyarray, 1, three))
        return (PyObject *) pyarray;
    else {
        pyjarray_dealloc(pyarray);
        return NULL;
    }
}


static int pyjarray_init(PyJarray_Object *pyarray, int zero, PyObject *value) {
    jobject compType  = NULL;
    jclass  compClass = NULL;
    int     comp;

    JNIEnv *env = pyarray->env;

    // ------------------------------ first, get the array's type

    if(pyarray->componentType < 0) { // may already know that
        if(objectComponentType == 0) {
            jmethodID getClass;
            jobject   langClass = NULL;
        
            getClass = (*env)->GetMethodID(env,
                                           pyarray->clazz,
                                           "getClass",
                                           "()Ljava/lang/Class;");
            if(process_java_exception(env) || !getClass)
                goto EXIT_ERROR;
        
            langClass = (*env)->CallObjectMethod(env, pyarray->clazz, getClass);
            if(process_java_exception(env) || !langClass)
                goto EXIT_ERROR;
        
            objectComponentType = (*env)->GetMethodID(env,
                                                      langClass,
                                                      "getComponentType",
                                                      "()Ljava/lang/Class;");
            if(process_java_exception(env) || !objectComponentType) {
                (*env)->DeleteLocalRef(env, langClass);
                goto EXIT_ERROR;
            }
        }
    
        compType = (*env)->CallObjectMethod(env,
                                            pyarray->clazz,
                                            objectComponentType);
        if(process_java_exception(env) || !compType)
            goto EXIT_ERROR;
    
        compClass = (*env)->GetObjectClass(env, compType);
        if(process_java_exception(env) || !compClass)
            goto EXIT_ERROR;

        comp = get_jtype(env, compType, compClass);
        if(process_java_exception(env) || comp < 0)
            goto EXIT_ERROR;
    
        pyarray->componentType = comp;
    }
    
    if(pyarray->length < 0) // may already know that, too
        pyarray->length = (*env)->GetArrayLength(env, pyarray->object);
    
    // ------------------------------ pinned array support
    // now, for primitive arrays we have to Release() the
    // array when we're done with it.

    switch(pyarray->componentType) {

    case JINT_ID: {
        int   i;
        long  v = 0;
        jint *ar;

        ar = pyarray->pinnedArray = (*env)->GetIntArrayElements(
            env,
            pyarray->object,
            &(pyarray->isCopy));

        if(zero) {
            if(value && PyInt_Check(value))
                v = PyInt_AS_LONG(value);
            
            for(i = 0; i < pyarray->length; i++)
                ar[i] = v;
        }
        
        break;
    }
        
    case JLONG_ID: {
        int      i;
        jeplong  v = 0;
        jlong   *ar;
        
        ar = pyarray->pinnedArray = (*env)->GetLongArrayElements(
            env,
            pyarray->object,
            &(pyarray->isCopy));
        
        if(zero) {
            if(!value)
                ;
            else {
                if(PyLong_Check(value))
                    v = PyLong_AsLongLong(value);
                else if(PyInt_Check(value))
                    v = PyInt_AS_LONG(value);
            }
            
            for(i = 0; i < pyarray->length; i++)
                ar[i] = v;
        }
        
        break;
    }
        
    case JBOOLEAN_ID: {
        int       i;
        long      v = 0;
        jboolean *ar;

        ar = pyarray->pinnedArray = (*env)->GetBooleanArrayElements(
            env,
            pyarray->object,
            &(pyarray->isCopy));
        
        if(zero) {
            if(value && PyInt_Check(value))
                v = PyInt_AS_LONG(value);
            
            for(i = 0; i < pyarray->length; i++) {
                if(v)
                    ar[i] = JNI_TRUE;
                else
                    ar[i] = JNI_FALSE;
            }
        }
        
        break;
    }
        
    case JDOUBLE_ID: {
        int      i;
        double   v = 0;
        jdouble *ar;

        ar = pyarray->pinnedArray = (*env)->GetDoubleArrayElements(
            env,
            pyarray->object,
            &(pyarray->isCopy));

        if(zero) {
            if(value && PyFloat_Check(value))
                v = PyFloat_AS_DOUBLE(value);
            
            for(i = 0; i < pyarray->length; i++)
                ar[i] = (jdouble) v;
        }
        
        break;
    }
        
    case JSHORT_ID: {
        int     i;
        long    v = 0;
        jshort *ar;

        ar = pyarray->pinnedArray = (*env)->GetShortArrayElements(
            env,
            pyarray->object,
            &(pyarray->isCopy));

        if(zero) {
            if(value && PyInt_Check(value))
                v  = PyInt_AS_LONG(value);
            
            for(i = 0; i < pyarray->length; i++)
                ar[i] = (jshort) v;
        }

        break;
    }
        
    case JFLOAT_ID: {
        int      i;
        double   v = 0;
        jfloat *ar;

        ar = pyarray->pinnedArray = (*env)->GetFloatArrayElements(
            env,
            pyarray->object,
            &(pyarray->isCopy));
        
        if(zero) {
            if(value && PyFloat_Check(value))
                v = PyFloat_AS_DOUBLE(value);
            
            for(i = 0; i < pyarray->length; i++)
                ar[i] = (jfloat) v;
        }
        
        break;
    }

    } // switch

    (*env)->DeleteLocalRef(env, compType);
    (*env)->DeleteLocalRef(env, compClass);
    
    if(process_java_exception(env))
        return 0;
    return 1;
    
EXIT_ERROR:
    if(compType)
        (*env)->DeleteLocalRef(env, compType);
    if(compClass)
        (*env)->DeleteLocalRef(env, compClass);

    return -1;
}


static void pyjarray_dealloc(PyJarray_Object *self) {
#if USE_DEALLOC
    JNIEnv *env = self->env;
    if(env) {
        if(self->object)
            (*env)->DeleteGlobalRef(env, self->object);
        if(self->clazz)
            (*env)->DeleteGlobalRef(env, self->clazz);

        // can't guarantee mode 0 will work in this case...
        pyjarray_release_pinned(self, JNI_ABORT);
    } // if env
    
    PyObject_Del(self);
#endif
}


// used to either release pinned memory, commit, or abort.
static void pyjarray_release_pinned(PyJarray_Object *self, jint mode) {
    JNIEnv *env = self->env;

    if(!self->pinnedArray)
        return;
    
    if(self->isCopy && mode == JNI_ABORT)
        return;

    switch(self->componentType) {

    case JINT_ID:
        (*env)->ReleaseIntArrayElements(env,
                                        self->object,
                                        (jint *) self->pinnedArray,
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


int pyjarray_check(PyObject *obj) {
    if(PyObject_TypeCheck(obj, &PyJarray_Type))
        return 1;
    return 0;
}


static int pyjarray_setitem(PyJarray_Object *self,
                            int pos,
                            PyObject *newitem) {
    
    JNIEnv *env = self->env;
    
    if(pos < 0 || pos >= self->length) {
        PyErr_Format(PyExc_IndexError,
                     "array assignment index out of range: %i", pos);
        return -1;
    }

    // first, do the object types.
    
    switch(self->componentType) {
        
    case JSTRING_ID: {
        jstring  jstr = NULL;
        char    *val;
        
        if(newitem == Py_None)
            ; // setting NULL
        else {
            if(!PyString_Check(newitem)) {
                PyErr_SetString(PyExc_TypeError, "Expected string.");
                return -1;
            }
        
            val  = PyString_AsString(newitem);
            jstr = (*env)->NewStringUTF(env, (const char *) val);
        }
        
        (*env)->SetObjectArrayElement(env,
                                      self->object,
                                      pos,
                                      jstr);
        if(process_java_exception(env))
            return -1;
        return 0;
    }

    case JOBJECT_ID: {
        jobject  obj = NULL;
        
        if(newitem == Py_None)
            ; // setting NULL
        else {
            if(!pyjobject_check(newitem)) {
                PyErr_SetString(PyExc_TypeError, "Expected pyjobject.");
                return -1;
            }
            
            obj = ((PyJobject_Object *) newitem)->object;
        }
        
        (*env)->SetObjectArrayElement(env,
                                      self->object,
                                      pos,
                                      obj);
        if(process_java_exception(env))
            return -1;
        return 0;
    }

    } // switch
    
    // ------------------------------ primitive types

    if(!self->pinnedArray) {
        PyErr_SetString(PyExc_RuntimeError, "Pinned array shouldn't be null.");
        return -1;
    }

    switch(self->componentType) {
        
    case JINT_ID:
        if(!PyInt_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected int.");
            return -1;
        }
        
        ((jint *) self->pinnedArray)[pos] = (jint) PyInt_AS_LONG(newitem);
        pyjarray_release_pinned(self, JNI_COMMIT);
        return 0; /* success */
        
    case JLONG_ID:
        if(!PyLong_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected long.");
            return -1;
        }
        
        ((jlong *) self->pinnedArray)[pos] = (jlong) PyLong_AsLongLong(newitem);
        pyjarray_release_pinned(self, JNI_COMMIT);
        return 0; /* success */
        
    case JBOOLEAN_ID:
        if(!PyInt_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected boolean.");
            return -1;
        }
        
        if(PyInt_AS_LONG(newitem))
            ((jint *) self->pinnedArray)[pos] = JNI_TRUE;
        else
            ((jint *) self->pinnedArray)[pos] = JNI_FALSE;
        
        pyjarray_release_pinned(self, JNI_COMMIT);
        return 0; /* success */
        
    case JDOUBLE_ID:
        if(!PyFloat_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected float.");
            return -1;
        }
        
        ((jdouble *) self->pinnedArray)[pos] =
            (jdouble) PyFloat_AS_DOUBLE(newitem);
        pyjarray_release_pinned(self, JNI_COMMIT);
        return 0; /* success */
        
    case JSHORT_ID:
        if(!PyInt_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected int.");
            return -1;
        }
        
        ((jshort *) self->pinnedArray)[pos] =
            (jshort) PyInt_AS_LONG(newitem);
        pyjarray_release_pinned(self, JNI_COMMIT);
        return 0; /* success */
        
    case JFLOAT_ID:
        if(!PyFloat_Check(newitem)) {
            PyErr_SetString(PyExc_TypeError, "Expected float.");
            return -1;
        }
        
        ((jfloat *) self->pinnedArray)[pos] =
            (jfloat) PyFloat_AS_DOUBLE(newitem);
        pyjarray_release_pinned(self, JNI_COMMIT);
        pyjarray_release_pinned(self, JNI_COMMIT);
        return 0; /* success */

    } // switch

    PyErr_SetString(PyExc_TypeError, "Unknown type.");
    return -1;
}


static PyObject* pyjarray_item(PyJarray_Object *self, int pos) {
    PyObject *ret = NULL;
    JNIEnv   *env = self->env;
    
    if(pos < 0)
        pos = 0;
    if(pos >= self->length)
        pos = self->length -1;

    switch(self->componentType) {

    case JSTRING_ID: {
        jstring     jstr;
        const char *str;
        
        jstr = (jstring) (*env)->GetObjectArrayElement(env,
                                                       self->object,
                                                       pos);

        if(process_java_exception(env))
            ;
        else if(jstr != NULL) {
            str = (*env)->GetStringUTFChars(env, jstr, 0);
            ret = PyString_FromString((char *) str);
            
            (*env)->ReleaseStringUTFChars(env, jstr, str);
            (*env)->DeleteLocalRef(env, jstr);
        }
        else {
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
                                                           pos);
        
        if(process_java_exception(env))
            ;
        else if(obj != NULL)
            ret = pyjarray_new(env, obj);
        else {
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
                                            pos);
        if(process_java_exception(env))
            ;
        else if(obj != NULL)
            ret = (PyObject *) pyjobject_new(env, obj);
        else {
            // null is okay
            Py_INCREF(Py_None);
            ret = Py_None;
        }
        
        break;
    }

    case JBOOLEAN_ID:
        ret = Py_BuildValue("i", ((jboolean *) self->pinnedArray)[pos]);
        break;

    case JSHORT_ID:
        ret = Py_BuildValue("i", ((jshort *) self->pinnedArray)[pos]);
        break;

    case JINT_ID:
        ret = Py_BuildValue("i", ((jint *) self->pinnedArray)[pos]);
        break;

    case JLONG_ID:
        ret = PyLong_FromLongLong(((jlong *) self->pinnedArray)[pos]);
        break;
        
    case JFLOAT_ID:
        ret = PyFloat_FromDouble(((jfloat *) self->pinnedArray)[pos]);
        break;

    case JDOUBLE_ID:
        ret = PyFloat_FromDouble(((jdouble *) self->pinnedArray)[pos]);
        break;
        
    default:
        PyErr_Format(PyExc_RuntimeError, "Unknown type %i.",
                     self->componentType);
    }

    return ret;
}


static int pyjarray_contains(PyJarray_Object *self, PyObject *el) {
    JNIEnv *env = self->env;

    switch(self->componentType) {

    case JSTRING_ID: {
        int i, ret = 0;

        if(!PyString_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected str.");
            return -1;
        }
        
        for(i = 0; ret == 0 && i < self->length; i++) {
            const char *val;
            PyObject   *t;
            jstring l = (*env)->GetObjectArrayElement(env,
                                                      self->object,
                                                      i);
            val = jstring2char(env, l);
            t   = PyString_FromString((char *) val);
            
            ret = PyObject_RichCompareBool(el,
                                           t,
                                           Py_EQ);
            
            Py_DECREF(t);
            release_utf_char(env, l, val);
            (*env)->DeleteLocalRef(env, l);
        }

        return ret;
    }

    case JARRAY_ID: {
        PyJarray_Object *obj;
        int i, ret = 0;

        JNIEnv *env = self->env;
        
        if(!pyjarray_check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected jarray.");
            return -1;
        }

        obj = (PyJarray_Object *) el;
        for(i = 0; !ret && i < self->length; i++) {
            jobject l = (*env)->GetObjectArrayElement(env,
                                                      self->object,
                                                      i);

            if((*env)->IsSameObject(env, l, obj->object))
                ret = 1;

            (*env)->DeleteLocalRef(env, l);
        }

        return ret;
    }

    case JOBJECT_ID: {
        PyJobject_Object *obj;
        int i, ret = 0;

        JNIEnv *env = self->env;
        
        if(!pyjobject_check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected jobject.");
            return -1;
        }

        obj = (PyJobject_Object *) el;
        for(i = 0; !ret && i < self->length; i++) {
            jobject l = (*env)->GetObjectArrayElement(env,
                                                      self->object,
                                                      i);

            if((*env)->IsSameObject(env, l, obj->object))
                ret = 1;

            (*env)->DeleteLocalRef(env, l);
        }

        return ret;
    }
        
    case JBOOLEAN_ID: {
        jboolean *ar = (jboolean *) self->pinnedArray;
        int       i;
        jboolean  v;
        
        if(!PyInt_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected boolean.");
            return -1;
        }
        
        if(PyInt_AS_LONG(el))
            v = JNI_TRUE;
        else
            v = JNI_FALSE;
        
        for(i = 0; i < self->length; i++) {
            if(v == ar[i])
                return 1;
        }
        
        return 0;
    }
        
    case JSHORT_ID: {
        jshort *ar = (jshort *) self->pinnedArray;
        int     i;
        jshort  v;
        
        if(!PyInt_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected int (short).");
            return -1;
        }
        
        v = (jshort) PyInt_AS_LONG(el);
        for(i = 0; i < self->length; i++) {
            if(v == ar[i])
                return 1;
        }
        
        return 0;
    }

    case JINT_ID: {
        jint *ar = (jint *) self->pinnedArray;
        int   i;
        jint  v;
        
        if(!PyInt_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected int.");
            return -1;
        }
        
        v = (jint) PyInt_AS_LONG(el);
        for(i = 0; i < self->length; i++) {
            if(v == ar[i])
                return 1;
        }
        
        return 0;
    }

    case JLONG_ID: {
        jlong *ar = (jlong *) self->pinnedArray;
        int    i;
        jlong  v;
        
        if(!PyLong_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected long.");
            return -1;
        }
        
        v = (jlong) PyLong_AsLongLong(el);
        for(i = 0; i < self->length; i++) {
            if(v == ar[i])
                return 1;
        }
        
        return 0;
    }
        
    case JFLOAT_ID: {
        jfloat *ar = (jfloat *) self->pinnedArray;
        int     i;
        jfloat  v;
        
        if(!PyFloat_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected long.");
            return -1;
        }
        
        v = (jfloat) PyFloat_AsDouble(el);
        for(i = 0; i < self->length; i++) {
            if(v == ar[i])
                return 1;
        }
        
        return 0;
    }

    case JDOUBLE_ID: {
        jdouble *ar = (jdouble *) self->pinnedArray;
        int      i;
        jdouble  v;
        
        if(!PyFloat_Check(el)) {
            PyErr_SetString(PyExc_TypeError, "Expected long.");
            return -1;
        }
        
        v = (jdouble) PyFloat_AsDouble(el);
        for(i = 0; i < self->length; i++) {
            if(v == ar[i])
                return 1;
        }
        
        return 0;
    }
        
    default:
        PyErr_Format(PyExc_RuntimeError, "Unknown type %i.",
                     self->componentType);
    } // switch

    return -1; // error, shouldn't happen
}


// shamelessly taken from listobject.c
static PyObject* pyjarray_subscript(PyJarray_Object *self, PyObject *item) {
    if (PyInt_Check(item)) {
        long i = PyInt_AS_LONG(item);
        if (i < 0)
            i += self->length;
        return pyjarray_item(self, i);
    }
    else if (PyLong_Check(item)) {
        long i = PyLong_AsLong(item);
        if (i == -1 && PyErr_Occurred())
            return NULL;
        if (i < 0)
            i += self->length;
        return pyjarray_item(self, i);
    }
    else if (PySlice_Check(item)) {
        PyErr_SetString(PyExc_RuntimeError, "slices not implemented.");
        return NULL;
/*         int start, stop, step, slicelength, cur, i; */
/*         PyObject* result; */
/*         PyObject* it; */
/*         PyObject **src, **dest; */

/*         if (PySlice_GetIndicesEx((PySliceObject*)item, self->ob_size, */
/*                                  &start, &stop, &step, &slicelength) < 0) { */
/*             return NULL; */
/*         } */

/*         if (slicelength <= 0) { */
/*             return PyList_New(0); */
/*         } */
/*         else { */
/*             result = PyList_New(slicelength); */
/*             if (!result) return NULL; */

/*             src = self->ob_item; */
/*             dest = ((PyListObject *)result)->ob_item; */
/*             for (cur = start, i = 0; i < slicelength; */
/*                  cur += step, i++) { */
/*                 it = src[cur]; */
/*                 Py_INCREF(it); */
/*                 dest[i] = it; */
/*             } */

/*             return result; */
/*         } */
    }
    else {
        PyErr_SetString(PyExc_TypeError,
                        "list indices must be integers");
        return NULL;
    }
}


// -------------------------------------------------- sequence methods

static int pyjarray_length(PyJarray_Object *self) {
    if(self)
        return self->length;
    return 0;
}


PyDoc_STRVAR(list_doc,
             "jarray(size) -> new jarray of size");
PyDoc_STRVAR(getitem_doc,
             "x.__getitem__(y) <==> x[y]");

PyMethodDef pyjarray_methods[] = {
    { "__getitem__",
      (PyCFunction) pyjarray_subscript, METH_O, getitem_doc },
    
    { NULL, NULL }
};


static PySequenceMethods list_as_sequence = {
    (inquiry) pyjarray_length,                /* sq_length */
    (binaryfunc) 0,                           /* sq_concat */
    (intargfunc) 0,                           /* sq_repeat */
    (intargfunc) pyjarray_item,               /* sq_item */
    0,                                        /* sq_slice */
    (intobjargproc) pyjarray_setitem,         /* sq_ass_item */
    (intintobjargproc) 0,                     /* sq_ass_slice */
    (objobjproc) pyjarray_contains,           /* sq_contains */
    (binaryfunc) 0,                           /* sq_inplace_concat */
    (intargfunc) 0,                           /* sq_inplace_repeat */
};


static PyObject* pyjarray_iter(PyObject *);


static PyTypeObject PyJarray_Type = {
    PyObject_HEAD_INIT(0)
    0,                                        /* ob_size */
    "jarray",                                 /* tp_name */
    sizeof(PyJarray_Object),                  /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor) pyjarray_dealloc,            /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    &list_as_sequence,                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
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
    PyJarray_Object *it_seq; /* Set to NULL when iterator is exhausted */
} PyJarrayIterObject;

PyTypeObject PyJarrayIter_Type;

static PyObject *pyjarray_iter(PyObject *seq) {
    PyJarrayIterObject *it;
    
    if(!pyjarray_check(seq)) {
        PyErr_BadInternalCall();
        return NULL;
    }
    it = PyObject_GC_New(PyJarrayIterObject, &PyJarrayIter_Type);
    if (it == NULL)
        return NULL;
    it->it_index = 0;
    Py_INCREF(seq);
    it->it_seq = (PyJarray_Object *) seq;
    _PyObject_GC_TRACK(it);
    return (PyObject *)it;
}

static void pyjarrayiter_dealloc(PyJarrayIterObject *it) {
    _PyObject_GC_UNTRACK(it);
    Py_XDECREF(it->it_seq);
    PyObject_GC_Del(it);
}

static int pyjarrayiter_traverse(PyJarrayIterObject *it, visitproc visit, void *arg) {
    if (it->it_seq == NULL)
        return 0;
    return visit((PyObject *)it->it_seq, arg);
}

static PyObject *pyjarrayiter_next(PyJarrayIterObject *it) {
    PyJarray_Object *seq;
    PyObject *item;

    assert(it != NULL);
    seq = it->it_seq;
    if (seq == NULL)
        return NULL;

    if (it->it_index < seq->length) {
        item = pyjarray_item(seq, it->it_index);
        ++it->it_index;
        Py_INCREF(item);
        return item;
    }
    
    Py_DECREF(seq);
    it->it_seq = NULL;
    return NULL;
}

static int pyjarrayiter_len(PyJarrayIterObject *it) {
    int len;
    if (it->it_seq) {
        len = it->it_seq->length - it->it_index;
        if (len >= 0)
            return len;
    }
    return 0;
}

static PySequenceMethods pyjarrayiter_as_sequence = {
    (inquiry) pyjarrayiter_len,               /* sq_length */
    0,                                        /* sq_concat */
};

PyTypeObject PyJarrayIter_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                        /* ob_size */
    "pyjarrayiterator",                       /* tp_name */
    sizeof(PyJarrayIterObject),               /* tp_basicsize */
    0,                                        /* tp_itemsize */
    /* methods */
    (destructor)pyjarrayiter_dealloc,         /* tp_dealloc */
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
    PyObject_GenericGetAttr,                  /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,  /* tp_flags */
    0,                                        /* tp_doc */
    (traverseproc)pyjarrayiter_traverse,      /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    PyObject_SelfIter,                        /* tp_iter */
    (iternextfunc)pyjarrayiter_next,          /* tp_iternext */
    0,                                        /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
};
