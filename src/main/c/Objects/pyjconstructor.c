/*
   jep - Java Embedded Python

   Copyright (c) 2016-2019 JEP AUTHORS.

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
#include "object.h"
#include "pyport.h"


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

    paramArray = java_lang_reflect_Constructor_getParameterTypes(env,
                 self->rmethod);
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


PyObject* PyJConstructor_New(JNIEnv *env, jobject constructor)
{
    PyJMethodObject* pym = NULL;

    if (PyJMethod_Type == NULL && jep_jmethod_type_ready() < 0) {
        return NULL;
    }
    if (PyJConstructor_Type == NULL && jep_jconstructor_type_ready() < 0) {
        return NULL;
    }

    pym = PyObject_NEW(PyJMethodObject, PyJConstructor_Type);
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
    return PyObject_TypeCheck(object, PyJConstructor_Type);
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

    Py_ssize_t actualNumArgs = PyTuple_Size(args);
    if (self->lenParameters != actualNumArgs - 1) {
        PyErr_Format(PyExc_RuntimeError,
                     "Invalid number of arguments: %i, expected %i.", (int) actualNumArgs,
                     self->lenParameters + 1);
        return NULL;
    }

    firstArg = PyTuple_GetItem(args, 0);
    if (!PyJClass_Check(firstArg)) {
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
    pobj = jobject_As_PyJObject(env, obj, clazz->clazz);

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


PyTypeObject *PyJConstructor_Type;
int jep_jconstructor_type_ready() {
    static PyType_Slot SLOTS[] = {
        {Py_tp_call, (void*) pyjconstructor_call},
        {Py_tp_doc, "jconstructor"},
        {0, NULL}
    };
    PyType_Spec spec = {
        .name = "jep.PyJConstructor",
        .basicsize = sizeof(PyJMethodObject),
        .flags = Py_TPFLAGS_DEFAULT,
        .slots = SLOTS
    };
    return jep_util_type_ready(&PyJConstructor_Type, &spec, PyJMethod_Type);
}
