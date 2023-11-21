/*
   jep - Java Embedded Python

   Copyright (c) 2016-2022 JEP AUTHORS.

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


/*
 * All constructors are named <init> so keep a single PyString around to use
 * as the methodName
 */
static PyObject* initMethodName = NULL;


static int pyjconstructor_init(JNIEnv *env, PyJMethodObject *self)
{
    jobjectArray paramArray = NULL;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return 0;
    }

    self->methodId = (*env)->FromReflectedMethod(env, self->rmethod);

    paramArray = java_lang_reflect_Executable_getParameterTypes(env,
                 self->rmethod);
    if (process_java_exception(env) || !paramArray) {
        goto EXIT_ERROR;
    }

    self->parameters    = (*env)->NewGlobalRef(env, paramArray);
    self->lenParameters = (*env)->GetArrayLength(env, paramArray);
    jobject jpymethod = java_lang_reflect_AnnotatedElement_getAnnotation(env,
                        self->rmethod, JPYMETHOD_TYPE);
    if (jpymethod) {
        self->isVarArgs = jep_PyMethod_varargs(env, jpymethod);
        if (process_java_exception(env)) {
            goto EXIT_ERROR;
        }
        self->isKwArgs = jep_PyMethod_kwargs(env, jpymethod);
        if (process_java_exception(env)) {
            goto EXIT_ERROR;
        }
    } else {
        if (process_java_exception(env)) {
            goto EXIT_ERROR;
        }
        self->isVarArgs = java_lang_reflect_Executable_isVarArgs(env, self->rmethod);
        if (process_java_exception(env)) {
            goto EXIT_ERROR;
        }
        self->isKwArgs = 0;
    }
    (*env)->PopLocalFrame(env, NULL);
    return 1;

EXIT_ERROR:
    (*env)->PopLocalFrame(env, NULL);
    return 0;
}


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
        initMethodName = PyUnicode_FromString("<init>");
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


/*
 * This function contains much of the same logic as pyjmethod_call. Refer
 * to the top level comment on that function for a summary of the behavior
 * here. Also ensure any changes here are done there if needed. If you
 * reading this and see a way to reduce the redundancy please do.
 */
static PyObject* pyjconstructor_call(PyJMethodObject *self, PyObject *args,
                                     PyObject *keywords)
{
    JNIEnv        *env              = NULL;
    Py_ssize_t     lenPyArgsGiven   = 0;
    int            lenJArgsExpected = 0;
    /* The number of normal arguments before any varargs or kwargs */
    int            lenJArgsNormal   = 0;
    PyObject      *firstArg         = NULL;
    PyJObject     *clazz            = NULL;
    int            pos              = 0;
    jvalue        *jargs            = NULL;
    /* if params includes pyjarray instance */
    int            foundArray       = 0;
    jobject        obj              = NULL;
    PyObject      *pobj             = NULL;
    /*
     * Whether varargs need special processing. When one vararg is given it
     * might be processed as a normal arg so this is a bit more complicated
     * than self->isVarArgs
     */
    int            needToDoVarArgs  = 0;

    lenPyArgsGiven = PyTuple_Size(args);

    env = pyembed_get_env();
    lenJArgsExpected = PyJMethod_GetParameterCount(self, env);
    if (lenJArgsExpected == -1) {
        return NULL;
    }
    lenJArgsNormal = lenJArgsExpected;
    if (self->isKwArgs) {
        lenJArgsNormal -= 1;
    }
    /* Python gives one more arg than java expects for self/this. */
    if (lenJArgsExpected != lenPyArgsGiven - 1) {
        if (!self->isVarArgs || lenJArgsNormal > lenPyArgsGiven) {
            PyErr_Format(PyExc_RuntimeError,
                         "Invalid number of arguments: %i, expected %i.",
                         (int) lenPyArgsGiven,
                         lenJArgsNormal + 1);
            return NULL;
        }
        /* The last argument will be handled as varargs, so not a normal arg */
        lenJArgsNormal -= 1;
        needToDoVarArgs = 1;
    }
    if (!self->isKwArgs && keywords != NULL && PyDict_Size(keywords) > 0) {
        PyErr_SetString(PyExc_RuntimeError, "Keywords are not supported.");
        return NULL;
    }

    firstArg = PyTuple_GetItem(args, 0);
    if (!PyJClass_Check(firstArg)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "First argument to a java constructor must be a java class.");
        return NULL;

    }
    clazz = (PyJObject*) firstArg;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS + lenJArgsExpected) != 0) {
        process_java_exception(env);
        return NULL;
    }

    jargs = (jvalue *) PyMem_Malloc(sizeof(jvalue) * lenJArgsExpected);
    if (jargs == NULL) {
        (*env)->PopLocalFrame(env, NULL);
        return PyErr_NoMemory();
    }
    for (pos = 0; pos < lenJArgsNormal; pos++) {
        PyObject *param = NULL;
        int paramTypeId = -1;
        jclass paramType = (jclass) (*env)->GetObjectArrayElement(env,
                           self->parameters, pos);

        param = PyTuple_GetItem(args, pos + 1);
        if (PyErr_Occurred()) {
            goto EXIT_ERROR;
        }

        paramTypeId = get_jtype(env, paramType);
        if (paramTypeId == JARRAY_ID) {
            foundArray = 1;
        }

        jargs[pos] = convert_pyarg_jvalue(env, param, paramType, paramTypeId, pos);
        if (PyErr_Occurred()) {
            if (pos == (lenJArgsExpected - 1)
                    && PyErr_ExceptionMatches(PyExc_TypeError)) {
                if (self->isVarArgs) {
                    /* Retry the last arg as array for varargs */
                    PyErr_Clear();
                    lenJArgsNormal -= 1;
                    needToDoVarArgs = 1;
                } else {
                    goto EXIT_ERROR;
                }
            }
        }

        (*env)->DeleteLocalRef(env, paramType);
    }
    if (needToDoVarArgs) {
        /* Need to process last arg as varargs. */
        PyObject *param = NULL;
        jclass paramType = (jclass) (*env)->GetObjectArrayElement(env,
                           self->parameters, lenJArgsNormal);
        if (lenPyArgsGiven == (lenJArgsNormal + 1)) {
            /*
             * Python args are normally one longer than expected to allow for
             * this/self so if it isn't then nothing was given for the varargs
             */
            param = PyTuple_New(0);
        } else {
            param = PyTuple_GetSlice(args, lenJArgsNormal + 1, lenPyArgsGiven);
        }
        if (PyErr_Occurred()) {
            goto EXIT_ERROR;
        }

        jargs[lenJArgsNormal] = convert_pyarg_jvalue(env, param, paramType,
                                JARRAY_ID,
                                lenJArgsNormal);
        Py_DecRef(param);
        if (PyErr_Occurred()) {
            goto EXIT_ERROR;
        }
        (*env)->DeleteLocalRef(env, paramType);
    }
    if (self->isKwArgs) {
        if (keywords) {
            jclass paramType = (jclass) (*env)->GetObjectArrayElement(env,
                               self->parameters, lenJArgsExpected - 1);
            jargs[lenJArgsExpected - 1] = convert_pyarg_jvalue(env, keywords, paramType,
                                          JOBJECT_ID, lenJArgsExpected - 1);
            if (PyErr_Occurred()) {
                goto EXIT_ERROR;
            }
            (*env)->DeleteLocalRef(env, paramType);
        } else {
            jargs[lenJArgsExpected - 1].l = NULL;
        }
    }

    Py_BEGIN_ALLOW_THREADS;
    obj = (*env)->NewObjectA(env,
                             clazz->clazz,
                             self->methodId,
                             jargs);
    Py_END_ALLOW_THREADS;
    if (process_java_exception(env) || !obj) {
        goto EXIT_ERROR;
    }

    // finally, make pyjobject and return
    pobj = jobject_As_PyJObject(env, obj, clazz->clazz);

    // we already closed the local frame, so make
    // sure to delete this local ref.
    PyMem_Free(jargs);

    // re pin array if needed
    if (foundArray) {
        for (pos = 0; pos < lenJArgsNormal; pos++) {
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
