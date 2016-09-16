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

PyObject* PyJMultiMethod_New(PyObject* method1, PyObject* method2)
{
    PyJMultiMethodObject* mm = NULL;

    if (PyType_Ready(&PyJMultiMethod_Type) < 0) {
        return NULL;
    }
    if (!PyJMethod_Check(method1) || !PyJMethod_Check(method2)) {
        PyErr_SetString(PyExc_TypeError, "PyJMultiMethod can only hold PyJMethods");
        return NULL;
    }

    mm = PyObject_NEW(PyJMultiMethodObject, &PyJMultiMethod_Type);
    if (mm == NULL) {
        return NULL;
    }
    mm->methodList = PyList_New(2);
    if (mm->methodList == NULL) {
        PyObject_Del(mm);
        return NULL;
    }
    Py_INCREF(method1);
    PyList_SET_ITEM(mm->methodList, 0, method1);
    Py_INCREF(method2);
    PyList_SET_ITEM(mm->methodList, 1, method2);
    return (PyObject *) mm;
}

int PyJMultiMethod_Append(PyObject* multimethod, PyObject* method)
{
    PyJMultiMethodObject* mm = NULL;
    if (!PyJMultiMethod_Check(multimethod)) {
        PyErr_SetString(PyExc_TypeError,
                        "PyJMultiMethod_Append received incorrect type");
        return -1;
    }
    if (!PyJMethod_Check(method)) {
        PyErr_SetString(PyExc_TypeError, "PyJMultiMethod can only hold PyJMethods");
        return -1;
    }
    mm = (PyJMultiMethodObject*) multimethod;
    return PyList_Append(mm->methodList, method);
}

int PyJMultiMethod_Check(PyObject* object)
{
    return PyObject_TypeCheck(object, &PyJMultiMethod_Type);
}

PyObject* PyJMultiMethod_GetName(PyObject* multimethod)
{
    PyJMultiMethodObject* mm         = NULL;
    PyJMethodObject*     method     = NULL;
    PyObject*             methodName = NULL;
    if (!PyJMultiMethod_Check(multimethod)) {
        PyErr_SetString(PyExc_TypeError,
                        "PyJMultiMethod_GetName received incorrect type");
        return NULL;
    }
    mm = (PyJMultiMethodObject*) multimethod;
    method = (PyJMethodObject*) PyList_GetItem(mm->methodList, 0);
    methodName = method->pyMethodName;
    Py_INCREF(methodName);
    return methodName;
}

static PyObject* pyjmultimethod_call(PyObject *multimethod,
                                     PyObject *args,
                                     PyObject *keywords)
{
    PyJMultiMethodObject* mm         = NULL;
    PyObject* methodName             = NULL;
    /*
     * cand is a candidate method that passes the simple compatiblity check but
     * the complex check may not have been run.
     */
    PyJMethodObject* cand           = NULL;
    /*
     * If multiple methods have the same number of args then a complex check
     * is done to find the best match, in this case candMatch has the current
     * match value for the current candidate method.
     */
    int               candMatch      = 0;
    Py_ssize_t        methodCount    = 0;
    Py_ssize_t        methodPosition = 0;
    Py_ssize_t        argsSize       = 0;
    JNIEnv*           env            = NULL;

    if (keywords != NULL) {
        PyErr_Format(PyExc_RuntimeError, "Keywords are not supported.");
        return NULL;
    }

    if (!PyJMultiMethod_Check(multimethod)) {
        PyErr_SetString(PyExc_TypeError,
                        "pyjmultimethod_call_internal received incorrect type");
        return NULL;
    }

    mm = (PyJMultiMethodObject*) multimethod;
    methodName = PyJMultiMethod_GetName(multimethod);
    methodCount = PyList_Size(mm->methodList);
    argsSize = PyTuple_Size(args) - 1;
    env = pyembed_get_env();

    for (methodPosition = 0; methodPosition < methodCount; methodPosition += 1) {
        PyJMethodObject* method = (PyJMethodObject*) PyList_GetItem(mm->methodList,
                                  methodPosition);
        if (PyJMethod_GetParameterCount(method, env) == argsSize) {
            if (cand) {
                if (!candMatch) {
                    candMatch = PyJMethod_CheckArguments(cand, env, args);
                }
                if (PyErr_Occurred()) {
                    cand = NULL;
                    break;
                } else if (!candMatch) {
                    // cand was not compatible, replace it with method.
                    cand = method;
                } else {
                    int methodMatch = PyJMethod_CheckArguments(method, env, args);
                    if (methodMatch > candMatch) {
                        cand = method;
                        candMatch = methodMatch;
                    }
                }
            } else if (PyErr_Occurred()) {
                break;
            } else {
                cand = method;
            }
        }
    }

    Py_DECREF(methodName);

    if (cand) {
        return PyObject_Call((PyObject*) cand, args, keywords);
    } else {
        if (!PyErr_Occurred()) {
            PyErr_SetString(PyExc_NameError, "No such Method.");
        }
        return NULL;
    }
}

/* returns internal list as tuple since its not safe to modify the list*/
PyObject* pyjmultimethod_getmethods(PyObject* multimethod)
{
    PyJMultiMethodObject* mm         = NULL;
    if (!PyJMultiMethod_Check(multimethod)) {
        PyErr_SetString(PyExc_TypeError,
                        "PyJMultiMethod_GetName received incorrect type");
        return NULL;
    }
    mm = (PyJMultiMethodObject*) multimethod;
    return PyList_AsTuple(mm->methodList);
}

static void pyjmultimethod_dealloc(PyJMultiMethodObject *self)
{
    Py_CLEAR(self->methodList);
    PyObject_Del(self);
}

static PyGetSetDef pyjmultimethod_getsetlist[] = {
    {"__name__", (getter) PyJMultiMethod_GetName, NULL},
    {"__methods__", (getter) pyjmultimethod_getmethods, NULL},
    {NULL} /* Sentinel */
};

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
    (ternaryfunc) pyjmultimethod_call,        /* tp_call */
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
    pyjmultimethod_getsetlist,                /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
