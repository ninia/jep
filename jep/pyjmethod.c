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

#ifdef WIN32
# include "winconfig.h"
#endif

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
#ifdef _FILE_OFFSET_BITS
# undef _FILE_OFFSET_BITS
#endif
#include <jni.h>

// shut up the compiler
#ifdef _POSIX_C_SOURCE
#  undef _POSIX_C_SOURCE
#endif
#include "Python.h"

#include "pyjmethod.h"
#include "util.h"
#include "pyembed.h"
#include "pyjarray.h"

extern PyTypeObject PyJmethod_Type;
extern PyMethodDef  pyjmethod_methods[];

static void pyjmethod_dealloc(PyJmethod_Object *self);

// cache methodIds
static jmethodID classGetName        = 0;
static jmethodID methodGetType       = 0;
static jmethodID methodGetParmTypes  = 0;
static jmethodID methodGetExceptions = 0;
static jmethodID methodGetModifiers  = 0;

// called internally to make new PyJmethod_Object instances.
// throws java exception and returns NULL on error.
PyJmethod_Object* pyjmethod_new(JNIEnv *env,
                                jobject rmethod,
                                PyJobject_Object *pyjobject) {
    PyJmethod_Object *pym          = NULL;
    jclass            rmethodClass = NULL;
    const char       *methodName   = NULL;
    jstring           jstr         = NULL;

    if(PyType_Ready(&PyJmethod_Type) < 0)
        return NULL;

    pym                = PyObject_NEW(PyJmethod_Object, &PyJmethod_Type);
    pym->rmethod       = (*env)->NewGlobalRef(env, rmethod);
    pym->pyjobject     = pyjobject;
    pym->env           = env;
    pym->parameters    = NULL;
    pym->lenParameters = 0;
    pym->pyMethodName  = NULL;
    pym->isStatic      = -1;
    pym->returnTypeId  = -1;
    
    // we reference it. make sure it doesn't go away.
    Py_INCREF(pyjobject);
    
    // ------------------------------ get method name
    
    rmethodClass = (*env)->GetObjectClass(env, rmethod);
    if(process_java_exception(env) || !rmethodClass)
        goto EXIT_ERROR;
    
    if(classGetName == 0) {
        classGetName = (*env)->GetMethodID(env,
                                           rmethodClass,
                                           "getName",
                                           "()Ljava/lang/String;");
        if(process_java_exception(env) || !classGetName)
            goto EXIT_ERROR;
    }
    
    jstr = (jstring) (*env)->CallObjectMethod(env,
                                              rmethod,
                                              classGetName);
    if(process_java_exception(env) || !jstr)
        goto EXIT_ERROR;
    
    methodName        = (*env)->GetStringUTFChars(env, jstr, 0);
    pym->pyMethodName = PyString_FromString(methodName);
    (*env)->ReleaseStringUTFChars(env, jstr, methodName);
    (*env)->DeleteLocalRef(env, jstr);
    
    (*env)->DeleteLocalRef(env, rmethodClass);
    return pym;

    
EXIT_ERROR:
    if(rmethodClass)
        (*env)->DeleteLocalRef(env, rmethodClass);
    
    if(pym)
        pyjmethod_dealloc(pym);
    return NULL;
}


// called internally to make new PyJmethod_Object instances.
// throws java exception and returns NULL on error.
PyJmethod_Object* pyjmethod_new_static(JNIEnv *env,
                                       jobject rmethod,
                                       PyJclass_Object *pyjclass) {
    PyJmethod_Object *pym          = NULL;
    jclass            rmethodClass = NULL;
    const char       *methodName   = NULL;
    jstring           jstr         = NULL;

    pym                = PyObject_NEW(PyJmethod_Object, &PyJmethod_Type);
    pym->rmethod       = (*env)->NewGlobalRef(env, rmethod);
    pym->pyjobject     = NULL;
    pym->env           = env;
    pym->parameters    = NULL;
    pym->lenParameters = 0;
    pym->pyMethodName  = NULL;
    pym->isStatic      = 1;
    pym->returnTypeId  = -1;

    // ------------------------------ get method name
    
    rmethodClass = (*env)->GetObjectClass(env, rmethod);
    if(process_java_exception(env) || !rmethodClass)
        goto EXIT_ERROR;
    
    if(classGetName == 0) {
        classGetName = (*env)->GetMethodID(env,
                                           rmethodClass,
                                           "getName",
                                           "()Ljava/lang/String;");
        if(process_java_exception(env) || !classGetName)
            goto EXIT_ERROR;
    }
    
    jstr = (jstring) (*env)->CallObjectMethod(env,
                                              rmethod,
                                              classGetName);
    if(process_java_exception(env) || !jstr)
        goto EXIT_ERROR;
    
    methodName        = (*env)->GetStringUTFChars(env, jstr, 0);
    pym->pyMethodName = PyString_FromString(methodName);
    (*env)->ReleaseStringUTFChars(env, jstr, methodName);
    (*env)->DeleteLocalRef(env, jstr);
    
    (*env)->DeleteLocalRef(env, rmethodClass);
    return pym;

    
EXIT_ERROR:
    if(rmethodClass)
        (*env)->DeleteLocalRef(env, rmethodClass);
    
    if(pym)
        pyjmethod_dealloc(pym);
    return NULL;
}


// 1 if successful, 0 if failed.
int pyjmethod_init(PyJmethod_Object *self) {
    jmethodID         methodId;
    jobject           returnType             = NULL;
    jobjectArray      paramArray             = NULL;
    jobjectArray      exceptions             = NULL;
    jclass            modClass               = NULL;
    jint              modifier               = -1;
    jboolean          isStatic               = JNI_FALSE;
    jclass            rmethodClass           = NULL;
    JNIEnv           *env;
    PyThreadState    *_save;
    
    env = self->env;
    
    Py_UNBLOCK_THREADS;
    
    // use a local frame so we don't have to worry too much about local refs.
    // make sure if this method errors out, that this is poped off again
    (*env)->PushLocalFrame(env, 20);
    if(process_java_exception(env))
        return 0;
    
    rmethodClass = (*env)->GetObjectClass(env, self->rmethod);
    PROCESS_JAVA_EXCEPTION(env);
    
    // ------------------------------ get methodid
    
    methodId = (*env)->FromReflectedMethod(env,
                                           self->rmethod);
    PROCESS_JAVA_EXCEPTION(env);
    
    self->methodId = methodId;
    
    
    // ------------------------------ get return type
    
    if(methodGetType == 0) {
        methodGetType = (*env)->GetMethodID(env,
                                            rmethodClass,
                                            "getReturnType",
                                            "()Ljava/lang/Class;");
        PROCESS_JAVA_EXCEPTION(env);
    }
    
    returnType = (*env)->CallObjectMethod(env,
                                          self->rmethod,
                                          methodGetType);
    PROCESS_JAVA_EXCEPTION(env);
    
    {
        jclass rclazz;

        rclazz = (*env)->GetObjectClass(env, returnType);
        PROCESS_JAVA_EXCEPTION(env);

        self->returnTypeId = get_jtype(env, returnType, rclazz);
        PROCESS_JAVA_EXCEPTION(env);
    }
    
    // ------------------------------ get parameter array

    if(methodGetParmTypes == 0) {
        methodGetParmTypes = (*env)->GetMethodID(env,
                                                 rmethodClass,
                                                 "getParameterTypes",
                                                 "()[Ljava/lang/Class;");
        PROCESS_JAVA_EXCEPTION(env);
    }
    
    paramArray = (jobjectArray) (*env)->CallObjectMethod(env,
                                                         self->rmethod,
                                                         methodGetParmTypes);
    PROCESS_JAVA_EXCEPTION(env);
    
    self->parameters    = (*env)->NewGlobalRef(env, paramArray);
    self->lenParameters = (*env)->GetArrayLength(env, paramArray);
    
    
    // ------------------------------ get exceptions declared thrown

#if USE_MAPPED_EXCEPTIONS

    if(methodGetExceptions == 0) {
        methodGetExceptions = (*env)->GetMethodID(env,
                                                  rmethodClass,
                                                  "getExceptionTypes",
                                                  "()[Ljava/lang/Class;");
        PROCESS_JAVA_EXCEPTION(env);
    }
    
    exceptions = (jobjectArray) (*env)->CallObjectMethod(env,
                                                         self->rmethod,
                                                         methodGetExceptions);
    PROCESS_JAVA_EXCEPTION(env);

    Py_BLOCK_THREADS;
    if(!register_exceptions(env,
                            rmethodClass,
                            self->rmethod,
                            exceptions)) {
        Py_UNBLOCK_THREADS;
        goto EXIT_ERROR;
    }
    Py_UNBLOCK_THREADS;

#endif    
    
    // ------------------------------ get isStatic
    
    if(self->isStatic != 1) { // may already know that
        
        // call getModifers()
        if(methodGetModifiers == 0) {
            methodGetModifiers = (*env)->GetMethodID(env,
                                                     rmethodClass,
                                                     "getModifiers",
                                                     "()I");
            PROCESS_JAVA_EXCEPTION(env);
        }
        
        modifier = (*env)->CallIntMethod(env,
                                         self->rmethod,
                                         methodGetModifiers);
        PROCESS_JAVA_EXCEPTION(env);
        
        modClass = (*env)->FindClass(env, "java/lang/reflect/Modifier");
        PROCESS_JAVA_EXCEPTION(env);
        
        // caching this methodid caused a crash on the mac
        methodId = (*env)->GetStaticMethodID(env,
                                             modClass,
                                             "isStatic",
                                             "(I)Z");
        PROCESS_JAVA_EXCEPTION(env);
        
        isStatic = (*env)->CallStaticBooleanMethod(env,
                                                   modClass,
                                                   methodId,
                                                   modifier);
        PROCESS_JAVA_EXCEPTION(env);
        
        if(isStatic == JNI_TRUE)
            self->isStatic = 1;
        else
            self->isStatic = 0;
    } // is static
    
    
    (*env)->PopLocalFrame(env, NULL);
    Py_BLOCK_THREADS;
    return 1;
    
    
EXIT_ERROR:
    Py_BLOCK_THREADS;
    (*env)->PopLocalFrame(env, NULL);
    
    if(!PyErr_Occurred())
        PyErr_SetString(PyExc_RuntimeError, "Unknown");
    return -1;
}


static void pyjmethod_dealloc(PyJmethod_Object *self) {
#if USE_DEALLOC
    JNIEnv *env  = self->env;
    if(env) {
        if(self->parameters)
            (*env)->DeleteGlobalRef(env, self->parameters);
        if(self->rmethod)
            (*env)->DeleteGlobalRef(env, self->rmethod);
        
        if(self->pyMethodName)
            Py_DECREF(self->pyMethodName);
        if(self->pyjobject)
            Py_DECREF(self->pyjobject);
    }
    
    PyObject_Del(self);
#endif
}


int pyjmethod_check(PyObject *obj) {
    if(PyObject_TypeCheck(obj, &PyJmethod_Type))
        return 1;
    return 0;
}


// called by python for __call__.
// we pass off processing to pyjobject's _find_method.
// sortof a dirty hack to get method overloading...
static PyObject* pyjmethod_call(PyJmethod_Object *self,
                                PyObject *args,
                                PyObject *keywords) {
    
    if(!PyTuple_Check(args)) {
        PyErr_Format(PyExc_RuntimeError, "args is not a valid tuple");
        return NULL;
    }
    
    if(keywords != NULL) {
        PyErr_Format(PyExc_RuntimeError, "Keywords are not supported.");
        return NULL;
    }
    
    return pyjobject_find_method(self->pyjobject, self->pyMethodName, args);
}


// pyjmethod_call_internal. where the magic happens.
// 
// okay, some of the magic -- we already the methodId, so we don't have
// to reflect. we just have to parse the arguments from python,
// check them against the java args, and call the java function.
// 
// easy. :-)
PyObject* pyjmethod_call_internal(PyJmethod_Object *self,
                                  PyObject *args) {
    PyObject      *result = NULL;
    const char    *str    = NULL;
    JNIEnv        *env    = NULL;
    int            pos    = 0;
    jvalue        *jargs  = NULL;
    PyThreadState *_save;
    
    if(!self->parameters) {
        if(!pyjmethod_init(self) || PyErr_Occurred())
            return NULL;
        return pyjmethod_call_internal(self, args);
    }
    
    env = self->env;
    
    // shouldn't happen
    if(self->lenParameters != PyTuple_GET_SIZE(args)) {
        PyErr_Format(PyExc_RuntimeError,
                     "Invalid number of arguments: %i, expected %i.",
                     PyTuple_GET_SIZE(args),
                     self->lenParameters);
        return NULL;
    }

    jargs = (jvalue *) PyMem_Malloc(sizeof(jvalue) * self->lenParameters);
    
    // ------------------------------ build jargs off python values

    for(pos = 0; pos < self->lenParameters; pos++) {
        PyObject *param       = NULL;
        int       paramTypeId = -1;
        jclass    pclazz;
        jclass    paramType   =
            (jclass) (*env)->GetObjectArrayElement(env,
                                                   self->parameters,
                                                   pos);

        param = PyTuple_GetItem(args, pos);                   /* borrowed */
        if(PyErr_Occurred()) {                                /* borrowed */
            PyMem_Free(jargs);
            return NULL;
        }
        
        pclazz = (*env)->GetObjectClass(env, paramType);
        if(process_java_exception(env) || !pclazz) {
            PyMem_Free(jargs);
            return NULL;
        }
        
        paramTypeId = get_jtype(env, paramType, pclazz);
        (*env)->DeleteLocalRef(env, pclazz);
        
        jargs[pos] = convert_pyarg_jvalue(env,
                                          param,
                                          paramType,
                                          paramTypeId,
                                          pos);
        if(PyErr_Occurred()) {                                /* borrowed */
            PyMem_Free(jargs);
            return NULL;
        }

        (*env)->DeleteLocalRef(env, paramType);
    } // for parameters

    
    // ------------------------------ call based off return type

    switch(self->returnTypeId) {

    case JSTRING_ID: {
        jstring jstr;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            jstr = (jstring) (*env)->CallStaticObjectMethodA(
                env,
                self->pyjobject->clazz,
                self->methodId,
                jargs);
        else
            jstr = (jstring) (*env)->CallObjectMethodA(env,
                                                       self->pyjobject->object,
                                                       self->methodId,
                                                       jargs);
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env) && jstr != NULL) {
            str    = (*env)->GetStringUTFChars(env, jstr, 0);
            result = PyString_FromString(str);
            
            (*env)->ReleaseStringUTFChars(env, jstr, str);
            (*env)->DeleteLocalRef(env, jstr);
        }
        
        break;
    }

    case JARRAY_ID: {
        jobjectArray obj;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            obj = (jobjectArray) (*env)->CallStaticObjectMethodA(
                env,
                self->pyjobject->clazz,
                self->methodId,
                jargs);
        else
            obj = (jobjectArray) (*env)->CallObjectMethodA(
                env,
                self->pyjobject->object,
                self->methodId,
                jargs);
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env) && obj != NULL)
            result = pyjarray_new(env, obj);
        
        break;
    }

    case JOBJECT_ID: {
        jobject obj;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            obj = (*env)->CallStaticObjectMethodA(
                env,
                self->pyjobject->clazz,
                self->methodId,
                jargs);
        else
            obj = (*env)->CallObjectMethodA(env,
                                            self->pyjobject->object,
                                            self->methodId,
                                            jargs);
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env) && obj != NULL)
            result = pyjobject_new(env, obj);
        
        break;
    }

    case JINT_ID: {
        jint ret;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            ret = (*env)->CallStaticIntMethodA(
                env,
                self->pyjobject->clazz,
                self->methodId,
                jargs);
        else
            ret = (*env)->CallIntMethodA(env,
                                         self->pyjobject->object,
                                         self->methodId,
                                         jargs);
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env))
            result = Py_BuildValue("i", ret);
        
        break;
    }

    case JSHORT_ID: {
        jshort ret;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            ret = (*env)->CallStaticShortMethodA(
                env,
                self->pyjobject->clazz,
                self->methodId,
                jargs);
        else
            ret = (*env)->CallShortMethodA(env,
                                           self->pyjobject->object,
                                           self->methodId,
                                           jargs);
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env))
            result = Py_BuildValue("i", (int) ret);
        
        break;
    }

    case JDOUBLE_ID: {
        jdouble ret;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            ret = (*env)->CallStaticDoubleMethodA(
                env,
                self->pyjobject->clazz,
                self->methodId,
                jargs);
        else
            ret = (*env)->CallDoubleMethodA(env,
                                            self->pyjobject->object,
                                            self->methodId,
                                            jargs);
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env))
            result = PyFloat_FromDouble(ret);
        
        break;
    }

    case JFLOAT_ID: {
        jfloat ret;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            ret = (*env)->CallStaticFloatMethodA(
                env,
                self->pyjobject->clazz,
                self->methodId,
                jargs);
        else
            ret = (*env)->CallFloatMethodA(env,
                                           self->pyjobject->object,
                                           self->methodId,
                                           jargs);
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env))
            result = PyFloat_FromDouble((double) ret);
        
        break;
    }

    case JLONG_ID: {
        jlong ret;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            ret = (*env)->CallStaticLongMethodA(
                env,
                self->pyjobject->clazz,
                self->methodId,
                jargs);
        else
            ret = (*env)->CallLongMethodA(env,
                                          self->pyjobject->object,
                                          self->methodId,
                                          jargs);
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env))
            result = PyLong_FromLongLong(ret);
        
        break;
    }

    case JBOOLEAN_ID: {
        jboolean ret;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            ret = (*env)->CallStaticBooleanMethodA(
                env,
                self->pyjobject->clazz,
                self->methodId,
                jargs);
        else
            ret = (*env)->CallBooleanMethodA(env,
                                             self->pyjobject->object,
                                             self->methodId,
                                             jargs);
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env))
            result = Py_BuildValue("i", ret);
        
        break;
    }

    default:
        Py_UNBLOCK_THREADS;

        // i hereby anoint thee a void method
        if(self->isStatic)
            (*env)->CallStaticVoidMethodA(env,
                                          self->pyjobject->clazz,
                                          self->methodId,
                                          jargs);
        else
            (*env)->CallVoidMethodA(env,
                                    self->pyjobject->object,
                                    self->methodId,
                                    jargs);

        Py_BLOCK_THREADS;
        process_java_exception(env);
    }
    
    PyMem_Free(jargs);
    
    if(PyErr_Occurred())
        return NULL;
    if(result == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    
    return result;
}


PyMethodDef pyjmethod_methods[] = {
    {NULL, NULL, 0, NULL}
};


PyTypeObject PyJmethod_Type = {
    PyObject_HEAD_INIT(0)
    0,
    "jep.PyJmethod",
    sizeof(PyJmethod_Object),
    0,
    (destructor) pyjmethod_dealloc,           /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    (ternaryfunc) pyjmethod_call,             /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    "jmethod",                                /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    pyjmethod_methods,                        /* tp_methods */
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
