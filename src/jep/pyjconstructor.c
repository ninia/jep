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

static int pyjconstructor_init(JNIEnv*, PyJMethodObject*);

static jmethodID constructorGetParamTypes = 0;

/*
 * All constructors are named <init> so keep a single PyString around to use
 * as the methodName
 */
static PyObject* initMethodName = NULL;

PyObject* PyJConstructor_New(JNIEnv *env, jobject constructor)
{
    PyJMethodObject* pym = NULL;

    if (PyType_Ready(&PyJMethod_Type) < 0) {
        return NULL;
    }
    if (!PyJConstructor_Type.tp_base) {
        PyJConstructor_Type.tp_base = &PyJMethod_Type;
    }
    if (PyType_Ready(&PyJConstructor_Type) < 0) {
        return NULL;
    }

    pym = PyObject_NEW(PyJMethodObject, &PyJConstructor_Type);
    pym->rmethod       = (*env)->NewGlobalRef(env, constructor);
    pym->parameters    = NULL;
    pym->lenParameters = 0;
    pym->isStatic      = 1;
    pym->returnTypeId  = JOBJECT_ID;
    if (!initMethodName) {
        initMethodName = PyString_FromString("<init>");
    }
    Py_INCREF(initMethodName);
    pym->pyMethodName = initMethodName;

    /*
     * PyJConstructor does not currently initialize lazily because PyJMethod
     * does not provide a mechanism to override the lazy loading and
     * pyjmethod_init does not work for a PyJConstructor. There isn't much
     * value in lazy loading anyway since constructors aren't created until
     * a class has been called.
     */
    if (!pyjconstructor_init(env, pym)) {
        Py_DECREF(pym);
        return NULL;
    }

    return (PyObject*) pym;
}

int PyJConstructor_Check(PyObject* object)
{
    return PyObject_TypeCheck(object, &PyJConstructor_Type);
}

static int pyjconstructor_init(JNIEnv *env, PyJMethodObject *self)
{
    jobjectArray paramArray = NULL;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return 0;
    }

    self->methodId = (*env)->FromReflectedMethod(env, self->rmethod);

    if (constructorGetParamTypes == 0) {
        jclass initClass = (*env)->GetObjectClass(env, self->rmethod);
        constructorGetParamTypes = (*env)->GetMethodID(env, initClass,
                                   "getParameterTypes", "()[Ljava/lang/Class;");
        if (!constructorGetParamTypes) {
            process_java_exception(env);
            goto EXIT_ERROR;
        }
        (*env)->DeleteLocalRef(env, initClass);
    }

    paramArray = (jobjectArray) (*env)->CallObjectMethod(env, self->rmethod,
                 constructorGetParamTypes);
    if (process_java_exception(env) || !paramArray) {
        goto EXIT_ERROR;
    }

    self->parameters    = (*env)->NewGlobalRef(env, paramArray);
    self->lenParameters = (*env)->GetArrayLength(env, paramArray);
    (*env)->PopLocalFrame(env, NULL);
    return 1;

EXIT_ERROR:
    (*env)->PopLocalFrame(env, NULL);
    return 0;
}

static PyObject* pyjconstructor_call(PyJMethodObject *self, PyObject *args,
                                     PyObject *keywords)
{
    PyObject      *firstArg    = NULL;
    PyJObject     *clazz       = NULL;
    JNIEnv        *env         = NULL;
    int            pos         = 0;
    jvalue        *jargs       = NULL;
    int           foundArray   = 0; /* if params includes pyjarray instance */
    PyThreadState *_save       = NULL;
    jobject   obj  = NULL;
    PyObject *pobj = NULL;

    if (keywords != NULL) {
        PyErr_Format(PyExc_TypeError, "Keywords are not supported.");
        return NULL;
    }

    if (self->lenParameters != PyTuple_GET_SIZE(args) - 1) {
        PyErr_Format(PyExc_RuntimeError,
                     "Invalid number of arguments: %i, expected %i.", (int) PyTuple_GET_SIZE(args),
                     self->lenParameters + 1);
        return NULL;
    }

    firstArg = PyTuple_GetItem(args, 0);
    if (!pyjclass_check(firstArg)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "First argument to a java constructor must be a java class.");
        return NULL;

    }
    clazz = (PyJObject*) firstArg;


    // ------------------------------ build jargs off python values
    env = pyembed_get_env();
    if ((*env)->PushLocalFrame(env, JLOCAL_REFS + self->lenParameters) != 0) {
        process_java_exception(env);
        return NULL;
    }

    jargs = (jvalue *) PyMem_Malloc(sizeof(jvalue) * self->lenParameters);

    for (pos = 0; pos < self->lenParameters; pos++) {
        PyObject *param = NULL;
        int paramTypeId = -1;
        jclass paramType = (jclass) (*env)->GetObjectArrayElement(env,
                           self->parameters, pos);

        param = PyTuple_GetItem(args, pos + 1); /* borrowed */
        if (PyErr_Occurred()) {
            goto EXIT_ERROR;
        }

        paramTypeId = get_jtype(env, paramType);
        if (paramTypeId == JARRAY_ID) {
            foundArray = 1;
        }

        jargs[pos] = convert_pyarg_jvalue(env, param, paramType, paramTypeId, pos);
        if (PyErr_Occurred()) {
            goto EXIT_ERROR;
        }

        (*env)->DeleteLocalRef(env, paramType);
    }

    Py_UNBLOCK_THREADS;
    obj = (*env)->NewObjectA(env,
                             clazz->clazz,
                             self->methodId,
                             jargs);
    Py_BLOCK_THREADS;
    if (process_java_exception(env) || !obj) {
        goto EXIT_ERROR;
    }

    // finally, make pyjobject and return
    pobj = pyjobject_new(env, obj);

    // we already closed the local frame, so make
    // sure to delete this local ref.
    PyMem_Free(jargs);

    // re pin array if needed
    if (foundArray) {
        for (pos = 0; pos < self->lenParameters; pos++) {
            PyObject *param = PyTuple_GetItem(args, pos);
            if (param && pyjarray_check(param)) {
                pyjarray_pin((PyJArrayObject *) param);
            }
        }
    }

    (*env)->PopLocalFrame(env, NULL);
    return pobj;

EXIT_ERROR:
    PyMem_Free(jargs);
    (*env)->PopLocalFrame(env, NULL);
    return NULL;
}


PyTypeObject PyJConstructor_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJConstructor",
    sizeof(PyJMethodObject),
    0,
    0,                                       /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    (ternaryfunc) pyjconstructor_call,        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    "jconstructor",                           /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    0,                                        /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0, // &PyJMethod_Type                     /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
