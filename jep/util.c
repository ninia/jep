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
*/ 	

#if HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
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
#include "Python.h"

#include "util.h"
#include "pyjobject.h"
#include "pyjmethod.h"
#include "pyjclass.h"
#include "pyembed.h"

// -------------------------------------------------- primitive class types
// these are shared for all threads, you shouldn't change them.

jclass JINT_TYPE     = NULL;
jclass JLONG_TYPE    = NULL;
jclass JOBJECT_TYPE  = NULL;
jclass JSTRING_TYPE  = NULL;
jclass JBOOLEAN_TYPE = NULL;
jclass JVOID_TYPE    = NULL;
jclass JDOUBLE_TYPE  = NULL;
jclass JSHORT_TYPE   = NULL;
jclass JFLOAT_TYPE   = NULL;


// cached methodids
jmethodID objectToString     = 0;
jmethodID objectEquals       = 0;


// call toString() on jobject, make a python string and return
// sets error conditions as needed.
// returns new reference to PyObject
PyObject* jobject_topystring(JNIEnv *env, jobject obj, jclass clazz) {
    const char *result;
    PyObject   *pyres;
    jstring     jstr;
    
    jstr = jobject_tostring(env, obj, clazz);
    // it's possible, i guess. don't throw an error....
    if(process_java_exception(env) || jstr == NULL)
        return PyString_FromString("");
    
    result = (*env)->GetStringUTFChars(env, jstr, 0);
    pyres  = PyString_FromString(result);
    (*env)->ReleaseStringUTFChars(env, jstr, result);
    
    // method returns new reference.
    return pyres;
}


PyObject* pystring_split_item(PyObject *str, char *split, int pos) {
    PyObject *splitList, *ret;
    int       len;

    if(pos < 0) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Invalid position to return.");
        return NULL;
    }

    splitList = PyObject_CallMethod(str, "split", "s", split);
    if(PyErr_Occurred() || !splitList)
        return NULL;
    
    if(!PyList_Check(splitList)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Oops, split string return is not a list.");
        return NULL;
    }
    
    len = PyList_Size(splitList);
    if(pos > len - 1) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Not enough items to return split position.");
        return NULL;
    }
    
    // get requested item
    ret = PyList_GetItem(splitList, pos);
    if(PyErr_Occurred())
        return NULL;
    if(!PyString_Check(ret)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Oops, item is not a string.");
        return NULL;
    }
    
    Py_INCREF(ret);
    Py_DECREF(splitList);
    return ret;
}


PyObject* pystring_split_last(PyObject *str, char *split) {
    PyObject *splitList, *ret;
    int       len;

    splitList = PyObject_CallMethod(str, "split", "s", split);
    if(PyErr_Occurred() || !splitList)
        return NULL;
    
    if(!PyList_Check(splitList)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Oops, split string return is not a list.");
        return NULL;
    }
    
    len = PyList_Size(splitList);
    
    // get the last one
    ret = PyList_GetItem(splitList, len - 1);
    if(PyErr_Occurred())
        return NULL;
    if(!PyString_Check(ret)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Oops, item is not a string.");
        return NULL;
    }
    
    Py_INCREF(ret);
    Py_DECREF(splitList);
    return ret;
}


// convert python exception to java.
int process_py_exception(JNIEnv *env, int printTrace) {
    jclass clazz;
    PyObject *ptype, *pvalue, *ptrace, *message = NULL;
    char *m;
    
    if(!PyErr_Occurred())
        return 0;
    
    // we only care about ptype and pvalue.
    // many people consider it a security vulnerability
    // to have the source code printed to the user's
    // screen. (i do.)
    //
    // so we pull relevant info and print strack to the
    // console.
    
    PyErr_Fetch(&ptype, &pvalue, &ptrace);
    
    if(ptype) {
        message = PyObject_Str(ptype);
        
        if(pvalue) {
            PyObject *v;
            m = PyString_AsString(message);
            
            v = PyObject_Str(pvalue);
            if(PyString_Check(v)) {
                PyObject *t;
                t = PyString_FromFormat("Python Encountered: %s: %s",
                                        m,
                                        PyString_AsString(v));
                
                Py_DECREF(v);
                Py_DECREF(message);
                message = t;
            }
        }
    }

    // disabled, caused a crash
/*     if(printTrace) { */
/*         PyErr_Restore(ptype, pvalue, ptrace); */
/*         PyErr_Print(); */
/*         PyErr_Clear(); */
/*     } */
/*     else { */
        if(ptype)
            Py_DECREF(ptype);
        if(pvalue)
            Py_DECREF(pvalue);
        if(ptrace)
            Py_DECREF(ptrace);
/*     } */
    
    clazz = (*env)->FindClass(env, JEPEXCEPTION);
    if(clazz && message && PyString_Check(message)) {
        m = PyString_AsString(message);
        (*env)->ThrowNew(env, clazz, m);
    }
    
    return 1;
}


// convert java exception to pyerr.
// true (1) if an exception was processed.
int process_java_exception(JNIEnv *env) {
    jstring     estr;
    jthrowable  exception    = NULL;
    jclass      clazz;
    PyObject   *pyException = PyExc_RuntimeError;
    PyObject   *str, *tmp, *jep, *texc, *className;
    int         len;
    char       *message;
    
    if(!(*env)->ExceptionCheck(env))
        return 0;
    
    if((exception = (*env)->ExceptionOccurred(env)) == NULL)
        return 0;
    
/*     (*env)->ExceptionDescribe(env); */
    // we're already processing this one, clear the old
    (*env)->ExceptionClear(env);
    
    clazz = (*env)->GetObjectClass(env, exception);
    if((*env)->ExceptionCheck(env) || !clazz) {
        (*env)->ExceptionDescribe(env);
        return 1;
    }
    
    estr = jobject_tostring(env,
                            exception,
                            clazz);
    if((*env)->ExceptionCheck(env) || !estr) {
        PyErr_Format(PyExc_RuntimeError,
                     "toString() on exception failed.");
        return 1;
    }

    message = (char *) jstring2char(env, estr);
    
#if USE_MAPPED_EXCEPTIONS
    
    // need to find the class name of the exception.
    // we already did toString(), so we'll just hackishly
    // find the name off that.
    //
    // format is:
    // java.lang.NumberFormatException: For input string: "asdf"
    //
    // if we don't find the name, just throw a RuntimeError
        
    str = PyString_FromString(message);

    if((tmp = pystring_split_last(str, ".")) == NULL ||
       PyErr_Occurred()) {
        
        Py_DECREF(str);
        return 1;
    }
    
    // may not have a message
    if((className = pystring_split_item(tmp, ":", 0)) == NULL ||
       PyErr_Occurred()) {
        
        Py_DECREF(tmp);
        Py_DECREF(str);
        return 1;
    }
    
    if((texc = pyembed_modjep_get(className)) != NULL) {
        pyException = texc;
        // Py_INCREF(pyException);
    }
    else
        printf("WARNING, didn't find mapped exception\n.");

    Py_DECREF(str);
    Py_DECREF(tmp);
    Py_DECREF(className);

#endif // #if USE_MAPPED_EXCEPTIONS

    PyErr_Format(pyException, message);
    release_utf_char(env, estr, message);
    
    (*env)->DeleteLocalRef(env, clazz);
    (*env)->DeleteLocalRef(env, exception);
    return 1;
}


// call toString() on jobject and return result.
// NULL on error
jstring jobject_tostring(JNIEnv *env, jobject obj, jclass clazz) {
    jstring     jstr;

    if(!env || !obj || !clazz)
        return NULL;

    if(objectToString == 0) {
        objectToString = (*env)->GetMethodID(env,
                                             clazz,
                                             "toString",
                                             "()Ljava/lang/String;");
        if(process_java_exception(env))
            return NULL;
    }

    if(!objectToString) {
        PyErr_Format(PyExc_RuntimeError, "Couldn't get methodId.");
        return NULL;
    }

    jstr = (jstring) (*env)->CallObjectMethod(env, obj, objectToString);
    if(process_java_exception(env))
        return NULL;

    return jstr;
}


// get a const char* string from java string.
// you *must* call release when you're finished with it.
// returns local reference.
const char* jstring2char(JNIEnv *env, jstring str) {
    return (*env)->GetStringUTFChars(env, str, 0);
}


// release memory allocated by jstring2char
void release_utf_char(JNIEnv *env, jstring str, const char *v) {
    (*env)->ReleaseStringUTFChars(env, str, v);
}


// in order to call methods that return primitives,
// we have to know they're return type. that's easy,
// i'm simply using the reflection api to call getReturnType().
// 
// however, jni requires us to use Call<type>Method.
// so, here we fetch the primitive Class objects
// (i.e. Integer.TYPE)
//
// returns 1 if successful, 0 if failed.
// doesn't process java exceptions.
int cache_primitive_classes(JNIEnv *env) {
    jclass   clazz, tmpclazz = NULL;
    jfieldID fieldId;
    jobject  tmpobj          = NULL;

    // ------------------------------ get Integer.TYPE

    if(JINT_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Integer");
        if((*env)->ExceptionOccurred(env))
            return 0;

        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if((*env)->ExceptionOccurred(env))
            return 0;

        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                                                         clazz,
                                                         fieldId);
        if((*env)->ExceptionOccurred(env))
            return 0;

        JINT_TYPE = (*env)->NewGlobalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpobj);
        (*env)->DeleteLocalRef(env, clazz);
    }
    
    if(JSHORT_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Short");
        if((*env)->ExceptionOccurred(env))
            return 0;

        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if((*env)->ExceptionOccurred(env))
            return 0;

        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                                                         clazz,
                                                         fieldId);
        if((*env)->ExceptionOccurred(env))
            return 0;

        JSHORT_TYPE = (*env)->NewGlobalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpobj);
        (*env)->DeleteLocalRef(env, clazz);
    }

    if(JDOUBLE_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Double");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if((*env)->ExceptionOccurred(env))
            return 0;

        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                                                         clazz,
                                                         fieldId);
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        JDOUBLE_TYPE = (*env)->NewGlobalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpobj);
        (*env)->DeleteLocalRef(env, clazz);
    }

    if(JFLOAT_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Float");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if((*env)->ExceptionOccurred(env))
            return 0;

        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                                                         clazz,
                                                         fieldId);
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        JFLOAT_TYPE = (*env)->NewGlobalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpobj);
        (*env)->DeleteLocalRef(env, clazz);
    }

    if(JLONG_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Long");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if((*env)->ExceptionOccurred(env))
            return 0;

        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                                                         clazz,
                                                         fieldId);
        if((*env)->ExceptionOccurred(env))
            return 0;

        JLONG_TYPE = (*env)->NewGlobalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpobj);
        (*env)->DeleteLocalRef(env, clazz);
    }

    if(JBOOLEAN_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Boolean");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                                                         clazz,
                                                         fieldId);
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        JBOOLEAN_TYPE = (*env)->NewGlobalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpobj);
        (*env)->DeleteLocalRef(env, clazz);
    }

    if(JVOID_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Void");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                                                         clazz,
                                                         fieldId);
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        JVOID_TYPE = (*env)->NewGlobalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpobj);
        (*env)->DeleteLocalRef(env, clazz);
    }

    if(JOBJECT_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Object");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        JOBJECT_TYPE = (*env)->NewGlobalRef(env, clazz);
        (*env)->DeleteLocalRef(env, clazz);
    }
    
    if(JSTRING_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/String");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        JSTRING_TYPE = (*env)->NewGlobalRef(env, clazz);
        (*env)->DeleteLocalRef(env, clazz);
    }

    return 1;
}


// remove global references setup in above function.
void unref_cache_primitive_classes(JNIEnv *env) {
    if(JINT_TYPE != NULL) {
        (*env)->DeleteGlobalRef(env, JINT_TYPE);
        JINT_TYPE = NULL;
    }
    if(JSHORT_TYPE != NULL) {
        (*env)->DeleteGlobalRef(env, JSHORT_TYPE);
        JSHORT_TYPE = NULL;
    }
    if(JDOUBLE_TYPE != NULL) {
        (*env)->DeleteGlobalRef(env, JDOUBLE_TYPE);
        JDOUBLE_TYPE = NULL;
    }
    if(JFLOAT_TYPE != NULL) {
        (*env)->DeleteGlobalRef(env, JFLOAT_TYPE);
        JFLOAT_TYPE = NULL;
    }
    if(JLONG_TYPE != NULL) {
        (*env)->DeleteGlobalRef(env, JLONG_TYPE);
        JLONG_TYPE = NULL;
    }
    if(JBOOLEAN_TYPE != NULL) {
        (*env)->DeleteGlobalRef(env, JBOOLEAN_TYPE);
        JBOOLEAN_TYPE = NULL;
    }
    if(JOBJECT_TYPE != NULL) {
        (*env)->DeleteGlobalRef(env, JOBJECT_TYPE);
        JOBJECT_TYPE = NULL;
    }
    if(JSTRING_TYPE != NULL) {
        (*env)->DeleteGlobalRef(env, JSTRING_TYPE);
        JSTRING_TYPE = NULL;
    }
    if(JVOID_TYPE != NULL) {
        (*env)->DeleteGlobalRef(env, JVOID_TYPE);
        JVOID_TYPE = NULL;
    }
}


// given the Class object, return the const ID.
// -1 on error.
// doesn't process errors!
int get_jtype(JNIEnv *env, jobject obj, jclass clazz) {
    jboolean equals = JNI_FALSE;

    // have to find the equals() method.
    if(objectEquals == 0) {
        jobject super = NULL;

        super = (*env)->GetSuperclass(env, clazz);
        if((*env)->ExceptionCheck(env) || !super) {
            (*env)->DeleteLocalRef(env, super);
            return -1;
        }
        
        objectEquals = (*env)->GetMethodID(env,
                                           super,
                                           "equals",
                                           "(Ljava/lang/Object;)Z");
        (*env)->DeleteLocalRef(env, super);
        if((*env)->ExceptionCheck(env) || !objectEquals)
            return -1;
    }

    // int
    equals = (*env)->CallBooleanMethod(env, obj, objectEquals, JINT_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JINT_ID;
    
    // short
    equals = (*env)->CallBooleanMethod(env, obj, objectEquals, JSHORT_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JSHORT_ID;

    // double
    equals = (*env)->CallBooleanMethod(env, obj, objectEquals, JDOUBLE_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JDOUBLE_ID;

    // float
    equals = (*env)->CallBooleanMethod(env, obj, objectEquals, JFLOAT_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JFLOAT_ID;

    // boolean
    equals = (*env)->CallBooleanMethod(env, obj, objectEquals, JBOOLEAN_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JBOOLEAN_ID;

    // long
    equals = (*env)->CallBooleanMethod(env, obj, objectEquals, JLONG_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JLONG_ID;

    // string
    equals = (*env)->CallBooleanMethod(env, obj, objectEquals, JSTRING_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JSTRING_ID;

    // void
    equals = (*env)->CallBooleanMethod(env, obj, objectEquals, JVOID_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JVOID_ID;
    
    // object
    if((*env)->IsAssignableFrom(env, clazz, JOBJECT_TYPE))
        return JOBJECT_ID;
    
    return -1;
}


// returns if the type of python object matches jclass
int pyarg_matches_jtype(JNIEnv *env,
                        PyObject *param,
                        jclass paramType,
                        int paramTypeId) {
    
    switch(paramTypeId) {
        
    case JSTRING_ID:
        
        if(param == Py_None)
            return 1;

        if(PyString_Check(param))
            return 1;
        
        if(pyjobject_check(param)) {
            // check if the object itself can cast to parameter type.
            if((*env)->IsAssignableFrom(env,
                                        ((PyJobject_Object *) param)->clazz,
                                        paramType))
                return 1;
        }
        
        break;
        
    case JOBJECT_ID:
        
        if(param == Py_None)
            return 1;
        
        if(pyjobject_check(param)) {
            // check if the object itself can cast to parameter type.
            if((*env)->IsAssignableFrom(env,
                                        ((PyJobject_Object *) param)->clazz,
                                        paramType))
                return 1;
        }

        if(PyString_Check(param)) {
            if((*env)->IsAssignableFrom(env,
                                        JSTRING_TYPE,
                                        paramType))
                return 1;
        }
        
        break;

    case JSHORT_ID:
    case JINT_ID:
        if(PyInt_Check(param))
            return 1;
        break;

    case JFLOAT_ID:
    case JDOUBLE_ID:
        if(PyFloat_Check(param))
            return 1;
        break;

    case JLONG_ID:
        if(PyLong_Check(param))
            return 1;
        break;
        
    case JBOOLEAN_ID:
        if(PyInt_Check(param))
            return 1;
        break;
    }

    // no match
    return 0;
}


// for parsing args.
// takes a python object and sets the right jvalue member for the given java type.
// returns uninitialized on error and raises a python exception.
jvalue convert_pyarg_jvalue(JNIEnv *env,
                            PyObject *param,
                            jclass paramType,
                            int paramTypeId,
                            int pos) {
    jvalue ret;
    ret.l = NULL;
    
    switch(paramTypeId) {

    case JSTRING_ID: {
        jstring   jstr;
        char     *val;
            
        // none is okay, we'll set a null
        if(param == Py_None)
            ret.l = NULL;
        else {
            // we could just convert it to a string...
            if(!PyString_Check(param)) {
                PyErr_Format(PyExc_ValueError,
                             "Expected string parameter at %i.",
                             pos + 1);
                return ret;
            }
                
            val   = PyString_AsString(param);
            jstr  = (*env)->NewStringUTF(env, (const char *) val);
            
            ret.l = jstr;
        }
        
        return ret;
    }

    case JOBJECT_ID: { 
        jobject obj = NULL;
        // none is okay, we'll translate to null
        if(param == Py_None)
            ;
        else if(PyString_Check(param)) {
            // strings count as objects here
            if(!(*env)->IsAssignableFrom(env,
                                         JSTRING_TYPE,
                                         paramType)) {
                PyErr_Format(
                    PyExc_ValueError,
                    "Tried to set a string on an incomparable parameter %i.",
                    pos + 1);
                return ret;
            }

            char *val = PyString_AsString(param);
            obj = (*env)->NewStringUTF(env, (const char *) val);
        }
        else {
            if(!pyjobject_check(param)) {
                PyErr_Format(PyExc_ValueError,
                             "Expected object parameter at %i.",
                             pos + 1);
                return ret;
            }

            // check object itself is assignable to that type.
            if(!(*env)->IsAssignableFrom(env,
                                         ((PyJobject_Object *) param)->clazz,
                                         paramType)) {
                PyErr_Format(PyExc_ValueError,
                             "Incorrect object type at %i.",
                             pos + 1);
                return ret;
            }

            obj = ((PyJobject_Object *) param)->object;
        }
        
        ret.l = obj;
        return ret;
    }

    case JSHORT_ID:
    case JINT_ID: {
        if(param == Py_None || !PyInt_Check(param)) {
            PyErr_Format(PyExc_ValueError,
                         "Expected int parameter at %i.",
                         pos + 1);
            return ret;
        }
        
        // precision loss...
        ret.s = (jshort) PyInt_AsLong(param);
        return ret;
    }

    case JDOUBLE_ID: {
        if(param == Py_None || !PyFloat_Check(param)) {
            PyErr_Format(PyExc_ValueError,
                         "Expected double parameter at %i.",
                         pos + 1);
            return ret;
        }
            
        ret.d = (jdouble) PyFloat_AsDouble(param);
        return ret;
    }

    case JFLOAT_ID: {
        if(param == Py_None || !PyFloat_Check(param)) {
            PyErr_Format(PyExc_ValueError,
                         "Expected float parameter at %i.",
                         pos + 1);
            return ret;
        }
        
        // precision loss
        ret.f = (jfloat) PyFloat_AsDouble(param);
        return ret;
    }

    case JLONG_ID: {
        if(param == Py_None || !PyLong_Check(param)) {
            PyErr_Format(PyExc_ValueError,
                         "Expected long parameter at %i.",
                         pos + 1);
            return ret;
        }
            
        ret.j = (jlong) PyLong_AsLongLong(param);
        return ret;
    }

    case JBOOLEAN_ID: {
        long bvalue;
        
        if(param == Py_None || !PyInt_Check(param)) {
            PyErr_Format(PyExc_ValueError,
                         "Expected boolean parameter at %i.",
                         pos + 1);
            return ret;
        }
        
        bvalue = PyInt_AsLong(param);
        if(bvalue > 0)
            ret.z = JNI_TRUE;
        else
            ret.z = JNI_FALSE;
        return ret;
    }
        
    } // switch
    
    PyErr_Format(PyExc_TypeError, "Unknown java type at %i.",
                 pos + 1);
    return ret;
}


// convenience function to pull a value from a list of tuples.
// expects tuples to be key, value format.
// parameters cannot be invalid!
// steals all references.
// returns new reference, new reference to Py_None if not found
PyObject* tuplelist_getitem(PyObject *list, PyObject *pyname) {
    int       i;
    int       listSize = 0;
    PyObject *ret      = NULL;
    
    listSize = PyList_GET_SIZE(list);
    for(i = 0; i < listSize; i++) {
        PyObject *tuple = PyList_GetItem(list, i);   /* borrowed */
        
        if(!tuple || !PyTuple_Check(tuple))
            continue;
        
        if(PyTuple_Size(tuple) == 2) {
            PyObject *key = PyTuple_GetItem(tuple, 0);    /* borrowed */
            if(!key || !PyString_Check(key))
                continue;
            
            if(PyObject_Compare(key, pyname) == 0) {
                ret   = PyTuple_GetItem(tuple, 1);        /* borrowed */
                break;
            }
        }
    }
    
    if(!ret)
        ret = Py_None;
    
    Py_INCREF(ret);
    return ret;
}


// reflect and retrieve Class array of exception types.
// register what we find as python exceptions in modjep.
int register_exceptions(JNIEnv *env,
                        jobject langClass,
                        jobject reflectObj,
                        jobjectArray exceptions) {
#if USE_MAPPED_EXCEPTIONS
    int       len, i;
    
    len = (*env)->GetArrayLength(env, exceptions);
    for(i = 0; i < len; i++) {
        jobject   exceptionClass;
        jclass    exceptionClazz;
        PyObject *str, *pyexc, *_jep, *tmp;
        char     *className, *jep;
        
        exceptionClass = (*env)->GetObjectArrayElement(env,
                                                       exceptions,
                                                       i);
        if(process_java_exception(env) || !exceptionClass)
            return 0;
        
        exceptionClazz = (*env)->GetObjectClass(env,
                                                exceptionClass);
        if(process_java_exception(env) || !exceptionClazz)
            return 0;
        
        str = jobject_topystring(env, exceptionClass, exceptionClazz);
        if((*env)->ExceptionCheck(env) || !str)
            return 0;
        
        // let string handle parsing the class name, i'm lazy.
        // (okay, i don't want to add more includes.)
        
        if((tmp = pystring_split_last(str, ".")) == NULL) {
            Py_DECREF(str);
            (*env)->DeleteLocalRef(env, exceptionClazz);
            (*env)->DeleteLocalRef(env, exceptionClass);
            continue;
        }
        Py_DECREF(str);
        
        // don't add more
        if(pyembed_modjep_has(tmp)) {
            Py_DECREF(tmp);
            (*env)->DeleteLocalRef(env, exceptionClazz);
            (*env)->DeleteLocalRef(env, exceptionClass);
            continue;
        }
        
        className = PyString_AsString(tmp);
        
        _jep = PyString_FromFormat("jep.%s", className);
        jep = PyString_AsString(_jep);
        
        pyexc = PyErr_NewException(jep, NULL, NULL);
        if(!pyembed_modjep_add(className, pyexc))               /* steals ref */
            PyErr_Warn(PyExc_Warning, "Failed to add exception.");
        
        Py_DECREF(tmp);
        Py_DECREF(_jep);
        (*env)->DeleteLocalRef(env, exceptionClazz);
        (*env)->DeleteLocalRef(env, exceptionClass);
    }
    
#endif // #if USE_MAPPED_EXCEPTIONS
    return 1;
}
