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
#include <jni.h>

// shut up the compiler
#ifdef _POSIX_C_SOURCE
#  undef _POSIX_C_SOURCE
#endif
#ifdef _FILE_OFFSET_BITS
# undef _FILE_OFFSET_BITS
#endif
#include "Python.h"

#include "pyembed.h"
#include "pyjfield.h"
#include "pyjclass.h"
#include "pyjobject.h"
#include "pyjmethod.h"
#include "pyjarray.h"
#include "util.h"

static PyObject* pyjclass_add_inner_classes(JNIEnv*, PyJobject_Object*);
static void pyjclass_dealloc(PyJclass_Object*);

static jmethodID classGetConstructors    = 0;
static jmethodID classGetParmTypes       = 0;
static jmethodID classGetDeclaredClasses = 0;
static jmethodID classGetModifiers       = 0;
static jmethodID classGetSimpleName      = 0;
static jmethodID modifierIsPublic        = 0;



int pyjclass_init(JNIEnv *env, PyObject *pyjob) {
    PyJclass_Object  *pyc         = NULL;
    jobject           langClass   = NULL;
    jobjectArray      initArray   = NULL;
    PyJobject_Object *pyjobject   = NULL;
    jobject           constructor = NULL;
    jclass            initClass   = NULL;
    jobjectArray      parmArray   = NULL;
    int               i;

    pyc = (PyJclass_Object*) pyjob;
    pyc->initArray  = NULL;
    
    pyjobject = (PyJobject_Object *) pyjob;

    (*env)->PushLocalFrame(env, 5);
    if(process_java_exception(env))
        return 0;
    
    // ------------------------------ call Class.getConstructors()

    // well, first call getClass()
    if(classGetConstructors == 0) {
        jmethodID methodId;
        
        methodId = (*env)->GetMethodID(env,
                                       pyjobject->clazz,
                                       "getClass",
                                       "()Ljava/lang/Class;");
        if(process_java_exception(env) || !methodId)
            goto EXIT_ERROR;
        
        langClass = (*env)->CallObjectMethod(env, pyjobject->clazz, methodId);
        if(process_java_exception(env) || !langClass)
            goto EXIT_ERROR;
        
        // then, find getContructors()
        classGetConstructors =
            (*env)->GetMethodID(env,
                                langClass,
                                "getConstructors",
                                "()[Ljava/lang/reflect/Constructor;");
        if(process_java_exception(env) || !classGetConstructors)
            goto EXIT_ERROR;
    }
    
    // then, call getConstructors()
    initArray = (jobjectArray) (*env)->CallObjectMethod(env,
                                                        pyjobject->clazz,
                                                        classGetConstructors);
    if(process_java_exception(env) || !initArray)
        goto EXIT_ERROR;

    pyc->initArray = (*env)->NewGlobalRef(env, initArray);
    pyc->initLen   = (*env)->GetArrayLength(env, pyc->initArray);

    /*
     * Optimization for faster performance. Cache number of arguments
     * for each constructor to avoid repeated reflection lookups.
     */
    pyc->numArgsPerInit = malloc(sizeof(int) * pyc->initLen);
    for(i = 0; i < pyc->initLen; i++) {
        constructor = (*env)->GetObjectArrayElement(env,
                                                    pyc->initArray,
                                                    i);
        if(process_java_exception(env) || !constructor)
            goto EXIT_ERROR;


        // we need to get the class java.lang.reflect.Constructor first
        initClass = (*env)->GetObjectClass(env, constructor);
        if(process_java_exception(env) || !initClass)
            goto EXIT_ERROR;

        // next, get parameters for constructor
        if(classGetParmTypes == 0) {
            classGetParmTypes = (*env)->GetMethodID(env,
                                                    initClass,
                                                    "getParameterTypes",
                                                    "()[Ljava/lang/Class;");
            if(process_java_exception(env) || !classGetParmTypes)
                goto EXIT_ERROR;
        }
        parmArray = (jobjectArray) (*env)->CallObjectMethod(env,
                                                            constructor,
                                                            classGetParmTypes);
        if(process_java_exception(env) || !parmArray)
            goto EXIT_ERROR;


        // now we know how many parameters this constructor receives
        pyc->numArgsPerInit[i] = (*env)->GetArrayLength(env, parmArray);
    } // end of optimization

    /*
     * attempt to add public inner classes as attributes since lots of people
     * code with public enum.  Note this will not allow the inner class to be
     * imported separately, it must be accessed through the enclosing class.
     */
    if(!pyjclass_add_inner_classes(env, pyjobject)) {
        /*
         * let's just print the error to stderr and continue on without
         * inner class support, it's not the end of the world
         */
        if(PyErr_Occurred()) {
            PyErr_PrintEx(0);
        }
    }

    (*env)->PopLocalFrame(env, NULL);
    return 1;

EXIT_ERROR:
    (*env)->PopLocalFrame(env, NULL);
    if(pyc)
        pyjclass_dealloc(pyc);
    
    return 0;
}

/*
 * Adds a Java class's public inner classes as attributes to the pyjclass.
 *
 * @param env the JNI environment
 * @param topClz the pyjobject of the top/outer Class
 *
 * @return topClz if successful, otherwise NULL
 */
static PyObject* pyjclass_add_inner_classes(JNIEnv *env,
                                            PyJobject_Object *topClz) {
    jobjectArray      innerArray    = NULL;
    jsize             innerSize     = 0;

    if(classGetDeclaredClasses == 0) {
        classGetDeclaredClasses = (*env)->GetMethodID(env,
                                                      JCLASS_TYPE,
                                                      "getDeclaredClasses",
                                                      "()[Ljava/lang/Class;");
        if(process_java_exception(env) || !classGetDeclaredClasses)
            return NULL;
    }

    if(classGetModifiers == 0) {
        classGetModifiers = (*env)->GetMethodID(env, JCLASS_TYPE, "getModifiers",
                "()I");
        if(process_java_exception(env) || !classGetModifiers)
            return NULL;
    }

    innerArray = (*env)->CallObjectMethod(env,
                                        topClz->clazz,
                                        classGetDeclaredClasses);
    if(process_java_exception(env) || !innerArray)
        return NULL;

    innerSize = (*env)->GetArrayLength(env, innerArray);
    if(innerSize > 0) {
        jclass    modClz     = NULL;
        int i;

        // setup to verify this inner class should be available
        modClz = (*env)->FindClass(env, "java/lang/reflect/Modifier");
        if(process_java_exception(env) || !modClz)
            return NULL;
        if(modifierIsPublic == 0) {
            modifierIsPublic = (*env)->GetStaticMethodID(env, modClz, "isPublic", "(I)Z");
            if(process_java_exception(env) || !modifierIsPublic)
                return NULL;
        }

        // check each inner class to see if it's public
        for(i=0; i < innerSize; i++) {
            jclass    innerClz = NULL;
            jint      mods;
            jboolean  public;

            innerClz = (*env)->GetObjectArrayElement(env, innerArray, i);
            if(process_java_exception(env) || !innerClz)
                return NULL;
            mods = (*env)->CallIntMethod(env, innerClz, classGetModifiers);
            if(process_java_exception(env))
                return NULL;
            public = (*env)->CallBooleanMethod(env, modClz, modifierIsPublic, mods);
            if(process_java_exception(env))
                return NULL;

            if(public) {
                PyObject        *attrClz    = NULL;
                jstring          shortName  = NULL;
                const char      *charName;

                attrClz = pyjobject_new_class(env, innerClz);
                if(process_java_exception(env) || !attrClz)
                    return NULL;
                if(classGetSimpleName == 0) {
                    classGetSimpleName = (*env)->GetMethodID(env, JCLASS_TYPE, "getSimpleName", "()Ljava/lang/String;");
                    if(process_java_exception(env) || !classGetSimpleName)
                        return NULL;
                }

                shortName = (*env)->CallObjectMethod(env, innerClz, classGetSimpleName);
                if(process_java_exception(env) || !shortName)
                    return NULL;
                charName = jstring2char(env, shortName);

                if(PyObject_SetAttrString((PyObject*) topClz, charName, attrClz) == -1) {
                    printf("Error adding inner class %s\n", charName);
                } else {
                    PyObject *pyname = PyString_FromString(charName);
                    pyjobject_addfield((PyJobject_Object*) topClz, pyname);
                    Py_DECREF(pyname);
                }
                Py_DECREF(attrClz); // parent class will hold the reference
                release_utf_char(env, shortName, charName);
            }
            (*env)->DeleteLocalRef(env, innerClz);
        }
    }

    (*env)->DeleteLocalRef(env, innerArray);

    return (PyObject*) topClz;
}


int pyjclass_check(PyObject *obj) {
    if(PyObject_TypeCheck(obj, &PyJclass_Type)) {
        return 1;
    }
    return 0;
}


static void pyjclass_dealloc(PyJclass_Object *self) {
#if USE_DEALLOC
    JNIEnv *env = pyembed_get_env();
    if(env) {
        if(self->initArray)
            (*env)->DeleteGlobalRef(env, self->initArray);
    }
    free(self->numArgsPerInit);
    pyjobject_dealloc((PyJobject_Object*) self);
#endif
}


// call constructor as a method and return pyjobject.
PyObject* pyjclass_call(PyJclass_Object *self,
                        PyObject *args,
                        PyObject *keywords) {
    int            initPos     = 0;
    int            parmPos     = 0;
    int            parmLen     = 0;
    jobjectArray   parmArray   = NULL;
    JNIEnv        *env;
    jclass         initClass   = NULL;
    jobject        constructor = NULL;
    jvalue        *jargs       = NULL;
    int            foundArray  = 0;
    PyThreadState *_save;
    Py_ssize_t     pyArgLength = 0;

    if(!PyTuple_Check(args)) {
        PyErr_Format(PyExc_RuntimeError, "args is not a valid tuple");
        return NULL;
    }
    
    if(keywords != NULL) {
        PyErr_Format(PyExc_RuntimeError, "Keywords are not supported.");
        return NULL;
    }

    env = pyembed_get_env();
    
    // use a local frame so we don't have to worry too much about references.
    // make sure if this method errors out, that this is popped off again
    (*env)->PushLocalFrame(env, 20);
    if(process_java_exception(env))
        return NULL;

    pyArgLength = PyTuple_Size(args);
    for(initPos = 0; initPos < self->initLen; initPos++) {
        parmLen = self->numArgsPerInit[initPos];
        // skip constructors that don't match the correct number of args
        if(parmLen != pyArgLength) {
            continue;
        }
        
        constructor = (*env)->GetObjectArrayElement(env,
                                                    self->initArray,
                                                    initPos);
        if(process_java_exception(env) || !constructor)
            goto EXIT_ERROR;
        
        // get the class java.lang.reflect.Constructor first
        initClass = (*env)->GetObjectClass(env, constructor);
        if(process_java_exception(env) || !initClass)
            goto EXIT_ERROR;
        
        // next, get parameters for constructor
        if(classGetParmTypes == 0) {
            classGetParmTypes = (*env)->GetMethodID(env,
                                                    initClass,
                                                    "getParameterTypes",
                                                    "()[Ljava/lang/Class;");
            if(process_java_exception(env) || !classGetParmTypes)
                goto EXIT_ERROR;
        }
        
        parmArray = (jobjectArray) (*env)->CallObjectMethod(env,
                                                            constructor,
                                                            classGetParmTypes);
        if(process_java_exception(env) || !parmArray)
            goto EXIT_ERROR;

        // next, find matching constructor for args
        // the counts match but maybe not the args themselves.
        jargs = (jvalue *) PyMem_Malloc(sizeof(jvalue) * parmLen);
        if(!jargs) {
            THROW_JEP(env, "Out of memory.");
            goto EXIT_ERROR;
        }
        
        for(parmPos = 0; parmPos < parmLen; parmPos++) {
            PyObject *param       = PyTuple_GetItem(args, parmPos);
            int       paramTypeId = -1;
            jobject   paramType   =
                (jclass) (*env)->GetObjectArrayElement(env,
                                                       parmArray,
                                                       parmPos);
            
            if(process_java_exception(env) || !paramType)
                break;
            
            paramTypeId = get_jtype(env, paramType);
            if(PyErr_Occurred() || process_java_exception(env))
                goto EXIT_ERROR;
            
            if(paramTypeId == JARRAY_ID)
                foundArray = 1;
            
            // if java and python agree, continue checking
            if(pyarg_matches_jtype(env, param, paramType, paramTypeId)) {
                jargs[parmPos] = convert_pyarg_jvalue(env,
                                                      param,
                                                      paramType,
                                                      paramTypeId,
                                                      parmPos);
                if(PyErr_Occurred() || process_java_exception(env))
                    goto EXIT_ERROR;
                
                continue; // continue checking parameters
            }
            
            break;
                
        } // for each parameter type
            
        // did they match?
        if(parmPos == parmLen) {
            jmethodID methodId = NULL;
            jobject   obj  = NULL;
            PyObject *pobj = NULL;
                
            if(PyErr_Occurred() || process_java_exception(env))
                goto EXIT_ERROR;
                
            // worked out, create new object
            methodId = (*env)->FromReflectedMethod(env,
                                                   constructor);
            if(process_java_exception(env) || !methodId)
                goto EXIT_ERROR;
                
            Py_UNBLOCK_THREADS;
            obj = (*env)->NewObjectA(env,
                                     ((PyJobject_Object*) self)->clazz,
                                     methodId,
                                     jargs);
            Py_BLOCK_THREADS;
            if(process_java_exception(env) || !obj)
                goto EXIT_ERROR;
                
            // finally, make pyjobject and return
            pobj = pyjobject_new(env, obj);
            
            // we already closed the local frame, so make
            // sure to delete this local ref.
            PyMem_Free(jargs);
            
            // re pin array if needed
            if(foundArray) {
                for(parmPos = 0; parmPos < parmLen; parmPos++) {
                    PyObject *param = PyTuple_GetItem(args, parmPos);
                    if(param && pyjarray_check(param))
                        pyjarray_pin((PyJarray_Object *) param);
                }
            }
            
            (*env)->PopLocalFrame(env, NULL);
            return pobj;
        }

        // prevent memory leak
        if(jargs) {
            PyMem_Free(jargs);
        }
        foundArray = 0;
    } // for each constructor
    
    
    (*env)->PopLocalFrame(env, NULL);
    PyErr_Format(PyExc_RuntimeError, "Couldn't find matching constructor.");
    return NULL;
    
    
EXIT_ERROR:
    if(jargs)
        PyMem_Free(jargs);
    
    (*env)->PopLocalFrame(env, NULL);
    
    return NULL;
}


static PyMethodDef pyjclass_methods[] = {
    {NULL, NULL, 0, NULL}
};


PyTypeObject PyJclass_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "PyJclass",
    sizeof(PyJclass_Object),
    0,
    (destructor) pyjclass_dealloc,            /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    (ternaryfunc) pyjclass_call,              /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    "jclass",                                 /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    pyjclass_methods,                         /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0, // PyJobject_Type                      /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
