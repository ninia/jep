/*
   jep - Java Embedded Python

   Copyright (c) 2015-2025 JEP AUTHORS.

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


PyObject* PyJMultiMethod_New(JepModuleState* modState, PyObject* method1,
                             PyObject* method2)
{
    PyJMultiMethodObject* mm = NULL;

    if (!PyJMethod_Check(modState, method1)
            || !PyJMethod_Check(modState, method2)) {
        PyErr_SetString(PyExc_TypeError, "PyJMultiMethod can only hold PyJMethods");
        return NULL;
    }
    PyTypeObject *tp = modState->PyJMultiMethod_Type;
    mm = (PyJMultiMethodObject*) tp->tp_alloc(tp, 0);
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

int PyJMultiMethod_Append(JepModuleState* modState, PyObject* multimethod,
                          PyObject* method)
{
    PyJMultiMethodObject* mm = NULL;
    if (!PyJMultiMethod_Check(modState, multimethod)) {
        PyErr_SetString(PyExc_TypeError,
                        "PyJMultiMethod_Append received incorrect type");
        return -1;
    }
    if (!PyJMethod_Check(modState, method)) {
        PyErr_SetString(PyExc_TypeError, "PyJMultiMethod can only hold PyJMethods");
        return -1;
    }
    mm = (PyJMultiMethodObject*) multimethod;
    return PyList_Append(mm->methodList, method);
}

int PyJMultiMethod_Check(JepModuleState* modState, PyObject* object)
{
    return PyObject_TypeCheck(object, modState->PyJMultiMethod_Type);
}

PyObject* PyJMultiMethod_GetName(PyObject* multimethod)
{
    PyJMultiMethodObject* mm         = NULL;
    PyJMethodObject*     method     = NULL;
    PyObject*             methodName = NULL;
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

    mm = (PyJMultiMethodObject*) multimethod;
    methodCount = PyList_Size(mm->methodList);
    argsSize = PyTuple_Size(args) - 1;
    env = pyembed_get_env();

    for (methodPosition = 0; methodPosition < methodCount; methodPosition += 1) {
        PyJMethodObject* method = (PyJMethodObject*) PyList_GetItem(mm->methodList,
                                  methodPosition);
        int parameterCount = PyJMethod_GetParameterCount(method, env);
        if (keywords && PyDict_Size(keywords) > 0 && !method->isKwArgs) {
            /*
             * keywords were passed in but this method does not support
             * keywords so keep looking at other methods.
             */
            continue;
        } else if (method->isKwArgs) {
            parameterCount -= 1;
        }
        if ((parameterCount == argsSize) || (method->isVarArgs
                                             && argsSize >= parameterCount - 1)) {
            if (cand) {
                if (!candMatch) {
                    candMatch = PyJMethod_CheckArguments(cand, env, args, keywords);
                }
                if (PyErr_Occurred()) {
                    cand = NULL;
                    break;
                } else if (!candMatch) {
                    // cand was not compatible, replace it with method.
                    cand = method;
                } else {
                    int methodMatch = PyJMethod_CheckArguments(method, env, args, keywords);
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
static PyObject* pyjmultimethod_getmethods(PyObject* multimethod)
{
    PyJMultiMethodObject* mm = (PyJMultiMethodObject*) multimethod;
    return PyList_AsTuple(mm->methodList);
}

static void pyjmultimethod_dealloc(PyJMultiMethodObject *self)
{
    PyObject_GC_UnTrack(self);
    PyTypeObject *tp = Py_TYPE(self);
    Py_CLEAR(self->methodList);
    tp->tp_free((PyObject*) self);
    Py_DECREF(tp);
}

static int pyjmultimethod_traverse(PyJMultiMethodObject *self, visitproc visit, void *arg)
{
    Py_VISIT(self->methodList);
    Py_VISIT(Py_TYPE(self));
    return 0;
}

static PyObject* pyjmultimethod_descr_get(PyObject *func, PyObject *obj,
        PyObject *type)
{
    if (obj == Py_None || obj == NULL) {
        Py_INCREF(func);
        return func;
    }
    return PyMethod_New(func, obj);
}

static PyGetSetDef pyjmultimethod_getsetlist[] = {
    {"__name__", (getter) PyJMultiMethod_GetName, NULL},
    {"__methods__", (getter) pyjmultimethod_getmethods, NULL},
    {NULL} /* Sentinel */
};

PyDoc_STRVAR(pyjmultimethod_doc,
             "PyJMultiMethod wraps multiple java methods from the same class with the same\n\
name as a single callable python object.");

static PyType_Slot slots[] = {
    {Py_tp_doc, (void*) pyjmultimethod_doc},
    {Py_tp_dealloc, pyjmultimethod_dealloc},
    {Py_tp_traverse, pyjmultimethod_traverse},
    {Py_tp_call, pyjmultimethod_call},
    {Py_tp_getset, pyjmultimethod_getsetlist},
    {Py_tp_descr_get, pyjmultimethod_descr_get},
    {0, NULL},
};

PyType_Spec PyJMultiMethod_Spec = {
    .name = "jep.PyJMultiMethod",
    .basicsize = sizeof(PyJMultiMethodObject),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .slots = slots
};
