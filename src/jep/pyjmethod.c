/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/* 
   jep - Java Embedded Python

   Copyright (c) 2015 JEP AUTHORS.

   This file is licenced under the the zlib/libpng License.

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

#include "pyembed.h"
#include "pyjmethod.h"
#include "pyjarray.h"
#include "util.h"
#include "pyembed.h"

static void pyjmethod_dealloc(PyJmethod_Object *self);

// cache methodIds
static jmethodID classGetName        = 0;
static jmethodID methodGetType       = 0;
static jmethodID methodGetParmTypes  = 0;
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
    pym->parameters    = NULL;
    pym->lenParameters = 0;
    pym->pyMethodName  = NULL;
    pym->isStatic      = -1;
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


// called internally to make new PyJmethod_Object instances.
// throws java exception and returns NULL on error.
PyJmethod_Object* pyjmethod_new_static(JNIEnv *env,
                                       jobject rmethod,
                                       PyJobject_Object *pyjobject) {
    PyJmethod_Object *pym          = NULL;
    jclass            rmethodClass = NULL;
    const char       *methodName   = NULL;
    jstring           jstr         = NULL;

    pym                = PyObject_NEW(PyJmethod_Object, &PyJmethod_Type);
    pym->rmethod       = (*env)->NewGlobalRef(env, rmethod);
    pym->parameters    = NULL;
    pym->lenParameters = 0;
    pym->pyMethodName  = NULL;
    pym->isStatic      = -1;
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
int pyjmethod_init(JNIEnv *env, PyJmethod_Object *self) {
    jmethodID         methodId;
    jobject           returnType             = NULL;
    jobjectArray      paramArray             = NULL;
    jclass            modClass               = NULL;
    jint              modifier               = -1;
    jboolean          isStatic               = JNI_FALSE;
    jclass            rmethodClass           = NULL;
    
    // use a local frame so we don't have to worry too much about local refs.
    // make sure if this method errors out, that this is poped off again
    (*env)->PushLocalFrame(env, 20);
    if(process_java_exception(env))
        return 0;
    
    rmethodClass = (*env)->GetObjectClass(env, self->rmethod);
    if(process_java_exception(env) || !rmethodClass)
        goto EXIT_ERROR;
    
    // ------------------------------ get methodid
    
    methodId = (*env)->FromReflectedMethod(env,
                                           self->rmethod);
    if(process_java_exception(env) || !methodId)
        goto EXIT_ERROR;
    
    self->methodId = methodId;
    
    
    // ------------------------------ get return type
    
    if(methodGetType == 0) {
        methodGetType = (*env)->GetMethodID(env,
                                            rmethodClass,
                                            "getReturnType",
                                            "()Ljava/lang/Class;");
        if(process_java_exception(env) || !methodGetType)
            goto EXIT_ERROR;
    }
    
    returnType = (*env)->CallObjectMethod(env,
                                          self->rmethod,
                                          methodGetType);
    if(process_java_exception(env) || !returnType)
        goto EXIT_ERROR;
    
    self->returnTypeId = get_jtype(env, returnType);
    if(process_java_exception(env))
        goto EXIT_ERROR;

    
    // ------------------------------ get parameter array

    if(methodGetParmTypes == 0) {
        methodGetParmTypes = (*env)->GetMethodID(env,
                                                 rmethodClass,
                                                 "getParameterTypes",
                                                 "()[Ljava/lang/Class;");
        if(process_java_exception(env) || !methodGetParmTypes)
            goto EXIT_ERROR;
    }
    
    paramArray = (jobjectArray) (*env)->CallObjectMethod(env,
                                                         self->rmethod,
                                                         methodGetParmTypes);
    if(process_java_exception(env) || !paramArray)
        goto EXIT_ERROR;
    
    self->parameters    = (*env)->NewGlobalRef(env, paramArray);
    self->lenParameters = (*env)->GetArrayLength(env, paramArray);
    
    // ------------------------------ get isStatic
    if(self->isStatic != 1) { // may already know that
        
        // call getModifers()
        if(methodGetModifiers == 0) {
            methodGetModifiers = (*env)->GetMethodID(env,
                                                     rmethodClass,
                                                     "getModifiers",
                                                     "()I");
            if(process_java_exception(env) || !methodGetModifiers)
                goto EXIT_ERROR;
        }
        
        modifier = (*env)->CallIntMethod(env,
                                         self->rmethod,
                                         methodGetModifiers);
        if(process_java_exception(env) || !modifier)
            goto EXIT_ERROR;
        
        modClass = (*env)->FindClass(env, "java/lang/reflect/Modifier");
        if(process_java_exception(env) || !modClass)
            goto EXIT_ERROR;
        
        // caching this methodid caused a crash on the mac
        methodId = (*env)->GetStaticMethodID(env,
                                             modClass,
                                             "isStatic",
                                             "(I)Z");
        if(process_java_exception(env) || !methodId)
            goto EXIT_ERROR;
        
        isStatic = (*env)->CallStaticBooleanMethod(env,
                                                   modClass,
                                                   methodId,
                                                   modifier);
        if(process_java_exception(env))
            goto EXIT_ERROR;
        
        if(isStatic == JNI_TRUE)
            self->isStatic = 1;
        else
            self->isStatic = 0;
    } // is static
    
    
    (*env)->PopLocalFrame(env, NULL);
    return 1;
    
    
EXIT_ERROR:
    (*env)->PopLocalFrame(env, NULL);
    
    if(!PyErr_Occurred())
        PyErr_SetString(PyExc_RuntimeError, "Unknown");
    return -1;
}


static void pyjmethod_dealloc(PyJmethod_Object *self) {
#if USE_DEALLOC
    JNIEnv *env  = pyembed_get_env();
    if(env) {
        if(self->parameters)
            (*env)->DeleteGlobalRef(env, self->parameters);
        if(self->rmethod)
            (*env)->DeleteGlobalRef(env, self->rmethod);
    }

    Py_CLEAR(self->pyMethodName);
    
    PyObject_Del(self);
#endif
}


int pyjmethod_check(PyObject *obj) {
    if(PyObject_TypeCheck(obj, &PyJmethod_Type))
        return 1;
    return 0;
}


// pyjmethod_call_internal. where the magic happens.
// 
// okay, some of the magic -- we already the methodId, so we don't have
// to reflect. we just have to parse the arguments from python,
// check them against the java args, and call the java function.
// 
// easy. :-)
PyObject* pyjmethod_call_internal(PyJmethod_Object *self,
                                  PyJobject_Object *instance,
                                  PyObject *args) {
    PyObject      *result     = NULL;
    const char    *str        = NULL;
    JNIEnv        *env        = NULL;
    int            pos        = 0;
    jvalue        *jargs      = NULL;
    int            foundArray = 0;   /* if params includes pyjarray instance */
    PyThreadState *_save;
    
    env = pyembed_get_env();
    
    if(!self->parameters) {
        if(!pyjmethod_init(env, self) || PyErr_Occurred())
            return NULL;
        return pyjmethod_call_internal(self, instance, args);
    }
    
    // validate we can call this method
    if(!instance->object && self->isStatic != JNI_TRUE) {
        PyErr_Format(PyExc_RuntimeError,
                     "Instantiate this class before "
                     "calling an object method.");
        return NULL;
    }
    
    // shouldn't happen
    if(self->lenParameters != PyTuple_GET_SIZE(args)) {
        PyErr_Format(PyExc_RuntimeError,
                     "Invalid number of arguments: %i, expected %i.",
                     (int) PyTuple_GET_SIZE(args),
                     self->lenParameters);
        return NULL;
    }

    jargs = (jvalue *) PyMem_Malloc(sizeof(jvalue) * self->lenParameters);
    
    // ------------------------------ build jargs off python values

    // hopefully 40 local references are enough per method call
    (*env)->PushLocalFrame(env, 40);
    for(pos = 0; pos < self->lenParameters; pos++) {
        PyObject *param = NULL;
        int paramTypeId = -1;
        jclass paramType = (jclass) (*env)->GetObjectArrayElement(env,
                self->parameters, pos);

        param = PyTuple_GetItem(args, pos);                   /* borrowed */
        if(PyErr_Occurred()) {                                /* borrowed */
            goto EXIT_ERROR;
        }

        paramTypeId = get_jtype(env, paramType);
        if(paramTypeId == JARRAY_ID)
            foundArray = 1;
        
        jargs[pos] = convert_pyarg_jvalue(env,
                                          param,
                                          paramType,
                                          paramTypeId,
                                          pos);
        if(PyErr_Occurred()) {                                /* borrowed */
            goto EXIT_ERROR;
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
                instance->clazz,
                self->methodId,
                jargs);
        else {
            // not static, a method on class then.
            if(!instance->object)
                jstr = (jstring) (*env)->CallObjectMethodA(
                    env,
                    instance->clazz,
                    self->methodId,
                    jargs);
            else
                jstr = (jstring) (*env)->CallObjectMethodA(
                    env,
                    instance->object,
                    self->methodId,
                    jargs);
        }
        
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
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                obj = (jobjectArray) (*env)->CallObjectMethodA(
                    env,
                    instance->clazz,
                    self->methodId,
                    jargs);
            else
                obj = (jobjectArray) (*env)->CallObjectMethodA(
                    env,
                    instance->object,
                    self->methodId,
                    jargs);
        }
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env) && obj != NULL)
            result = pyjarray_new(env, obj);
        
        break;
    }

    case JCLASS_ID: {
        jobject obj;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            obj = (*env)->CallStaticObjectMethodA(
                env,
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                obj = (*env)->CallObjectMethodA(env,
                                                instance->clazz,
                                                self->methodId,
                                                jargs);
            else
                obj = (*env)->CallObjectMethodA(env,
                                                instance->object,
                                                self->methodId,
                                                jargs);
        }
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env) && obj != NULL)
            result = pyjobject_new_class(env, obj);
        
        break;
    }

    case JOBJECT_ID: {
        jobject obj;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            obj = (*env)->CallStaticObjectMethodA(
                env,
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                obj = (*env)->CallObjectMethodA(env,
                                                instance->clazz,
                                                self->methodId,
                                                jargs);
            else
                obj = (*env)->CallObjectMethodA(env,
                                                instance->object,
                                                self->methodId,
                                                jargs);
        }

        Py_BLOCK_THREADS;
        if(!process_java_exception(env) && obj != NULL) {
            result = pyjobject_new(env, obj);
        }
        
        break;
    }

    case JINT_ID: {
        jint ret;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            ret = (*env)->CallStaticIntMethodA(
                env,
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                ret = (*env)->CallIntMethodA(env,
                                             instance->clazz,
                                             self->methodId,
                                             jargs);
            else
                ret = (*env)->CallIntMethodA(env,
                                             instance->object,
                                             self->methodId,
                                             jargs);
        }
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env))
            result = Py_BuildValue("i", ret);
        
        break;
    }

    case JBYTE_ID: {
        jbyte ret;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            ret = (*env)->CallStaticByteMethodA(
                env,
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                ret = (*env)->CallByteMethodA(env,
                                              instance->clazz,
                                              self->methodId,
                                              jargs);
            else
                ret = (*env)->CallByteMethodA(env,
                                              instance->object,
                                              self->methodId,
                                              jargs);
        }
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env))
            result = Py_BuildValue("i", ret);
        
        break;
    }

    case JCHAR_ID: {
        jchar ret;
        char  val[2];
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            ret = (*env)->CallStaticCharMethodA(
                env,
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                ret = (*env)->CallCharMethodA(env,
                                              instance->clazz,
                                              self->methodId,
                                              jargs);
            else
                ret = (*env)->CallCharMethodA(env,
                                              instance->object,
                                              self->methodId,
                                              jargs);
        }
        
        Py_BLOCK_THREADS;
        if(!process_java_exception(env)) {
            val[0] = (char) ret;
            val[1] = '\0';
            result = PyString_FromString(val);
        }
        break;
    }

    case JSHORT_ID: {
        jshort ret;
        Py_UNBLOCK_THREADS;
        
        if(self->isStatic)
            ret = (*env)->CallStaticShortMethodA(
                env,
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                ret = (*env)->CallShortMethodA(env,
                                               instance->clazz,
                                               self->methodId,
                                               jargs);
            else
                ret = (*env)->CallShortMethodA(env,
                                               instance->object,
                                               self->methodId,
                                               jargs);
        }
        
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
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                ret = (*env)->CallDoubleMethodA(env,
                                                instance->clazz,
                                                self->methodId,
                                                jargs);
            else
                ret = (*env)->CallDoubleMethodA(env,
                                                instance->object,
                                                self->methodId,
                                                jargs);
        }
        
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
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                ret = (*env)->CallFloatMethodA(env,
                                               instance->clazz,
                                               self->methodId,
                                               jargs);
            else
                ret = (*env)->CallFloatMethodA(env,
                                               instance->object,
                                               self->methodId,
                                               jargs);
        }
        
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
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                ret = (*env)->CallLongMethodA(env,
                                              instance->clazz,
                                              self->methodId,
                                              jargs);
            else
                ret = (*env)->CallLongMethodA(env,
                                              instance->object,
                                              self->methodId,
                                              jargs);
        }
        
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
                instance->clazz,
                self->methodId,
                jargs);
        else {
            if(!instance->object)
                ret = (*env)->CallBooleanMethodA(env,
                                                 instance->clazz,
                                                 self->methodId,
                                                 jargs);
            else
                ret = (*env)->CallBooleanMethodA(env,
                                                 instance->object,
                                                 self->methodId,
                                                 jargs);
        }
        
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
                                          instance->clazz,
                                          self->methodId,
                                          jargs);
        else
            (*env)->CallVoidMethodA(env,
                                    instance->object,
                                    self->methodId,
                                    jargs);

        Py_BLOCK_THREADS;
        process_java_exception(env);
        break;
    }
    
    PyMem_Free(jargs);
    (*env)->PopLocalFrame(env, NULL);
    
    if(PyErr_Occurred())
        return NULL;
    
    // re pin array objects if needed
    if(foundArray) {
        for(pos = 0; pos < self->lenParameters; pos++) {
            PyObject *param = PyTuple_GetItem(args, pos);     /* borrowed */
            if(param && pyjarray_check(param))
                pyjarray_pin((PyJarray_Object *) param);
        }
    }
    
    if(result == NULL) {
        Py_RETURN_NONE;
    }
    
    return result;

EXIT_ERROR:
   PyMem_Free(jargs);
   (*env)->PopLocalFrame(env, NULL);
   return NULL;
}


static PyMethodDef pyjmethod_methods[] = {
    {NULL, NULL, 0, NULL}
};


PyTypeObject PyJmethod_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
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
    0,                                        /* tp_call */
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
