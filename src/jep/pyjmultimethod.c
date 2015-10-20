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

#include "Python.h"
#ifdef _FILE_OFFSET_BITS
# undef _FILE_OFFSET_BITS
#endif
#include <jni.h>


#include "pyjmultimethod.h"
#include "pyjobject.h"
#include "pyembed.h"

PyObject* PyJMultiMethod_New(PyObject* method1, PyObject* method2) {
    PyJMultiMethodObject* mm = NULL;

    if(PyType_Ready(&PyJMultiMethod_Type) < 0){
        return NULL;
    }
    if(!pyjmethod_check(method1) || !pyjmethod_check(method2)){
        PyErr_SetString(PyExc_TypeError, "PyJMultiMethod can only hold PyJmethods");
        return NULL;
    }

    mm = PyObject_NEW(PyJMultiMethodObject, &PyJMultiMethod_Type);
    mm->methodList = PyList_New(2);
    PyList_SET_ITEM(mm->methodList, 0, method1);
    PyList_SET_ITEM(mm->methodList, 1, method2);
    return (PyObject *) mm;
}

int PyJMultiMethod_Append(PyObject* multimethod, PyObject* method){
    PyJMultiMethodObject* mm = NULL;
    if(!PyJMultiMethod_Check(multimethod)){
        PyErr_SetString(PyExc_TypeError, "PyJMultiMethod_Append received incorrect type");
        return -1;
    }
    if(!pyjmethod_check(method)){
        PyErr_SetString(PyExc_TypeError, "PyJMultiMethod can only hold PyJmethods");
        return -1;
    }
    mm = (PyJMultiMethodObject*) multimethod;
    return PyList_Append(mm->methodList, method);
}

int PyJMultiMethod_Check(PyObject* object){
    return PyObject_TypeCheck(object, &PyJMultiMethod_Type);
}

PyObject* PyJMultiMethod_GetName(PyObject* multimethod) {
    PyJMultiMethodObject* mm     = NULL; 
    PyJmethod_Object*     method = NULL; 
    if(!PyJMultiMethod_Check(multimethod)){
        PyErr_SetString(PyExc_TypeError, "pyjmultimethod_call_internal received incorrect type");
        return NULL;
    }
    mm = (PyJMultiMethodObject*) multimethod;
    method = (PyJmethod_Object*) PyList_GetItem(mm->methodList, 0);
    return method->pyMethodName;
}

PyObject* pyjmultimethod_call_internal(PyObject* multimethod, PyJobject_Object* pyjobject, PyObject* args){
    PyJMultiMethodObject* mm         = NULL;
    PyObject* methodName             = NULL;
    /* 
     * cand is a method that passes the simple compatiblity check but the
     * complex check has not run yet.
     */
    PyJmethod_Object* cand           = NULL;
    Py_ssize_t        methodCount    = 0;
    Py_ssize_t        methodPosition = 0;
    Py_ssize_t        argsSize       = 0;
    JNIEnv*           env            = NULL;
    if(!PyJMultiMethod_Check(multimethod)){
        PyErr_SetString(PyExc_TypeError, "pyjmultimethod_call_internal received incorrect type");
        return NULL;
    }
    mm = (PyJMultiMethodObject*) multimethod;
    methodName = PyJMultiMethod_GetName(multimethod);
    methodCount = PyList_Size(mm->methodList);
    argsSize = PyTuple_Size(args);
    env = pyembed_get_env();
    for(methodPosition = 0; methodPosition < methodCount; methodPosition += 1){
        PyJmethod_Object* method = (PyJmethod_Object*) PyList_GetItem(mm->methodList, methodPosition);
        if(pyjmethod_check_simple_compat(method, env, methodName, argsSize)){
            if(cand){
                if(pyjmethod_check_complex_compat(cand, env, args)){
                    // cand is completly compatible, call it
                    break;
                }else if(PyErr_Occurred()){
                    return NULL;
                }else{
                    // cand was not compatible, replace it with method.
                    cand = method;
                }
            }else if(PyErr_Occurred()){
                return NULL;
            }else{
                cand = method;
            }
        }
    }
    if(cand){
        return pyjmethod_call_internal(cand, pyjobject, args);
    }else{
        if(!PyErr_Occurred()){
            PyErr_SetString(PyExc_NameError, "No such Method.");
        }
        return NULL;
    }
}

static void pyjmultimethod_dealloc(PyJMultiMethodObject *self) {
    Py_CLEAR(self->methodList);
    PyObject_Del(self);
}

PyDoc_STRVAR(pyjmultimethod_doc,
"PyJMultiMethod wraps multiple java methods from the same class with the same\n\
name as a single callable python object.");

PyTypeObject PyJMultiMethod_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJMultiMethod",
    sizeof(PyJMultiMethodObject),
    0,
    (destructor) pyjmultimethod_dealloc,      /* tp_dealloc */
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
    pyjmultimethod_doc,                       /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    0,                                        /* tp_methods */
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
