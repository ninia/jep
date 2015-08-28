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

#include "pyjiterable.h"
#include "pyjobject.h"
#include "pyembed.h"


/*
 * News up a pyjiterable, which is just a pyjobject that supports iteration.
 * This should only be called from pyjobject_new().
 */
PyJiterable_Object* pyjiterable_new() {
    // pyjobject will have already initialized PyJiterable_Type
    return PyObject_NEW(PyJiterable_Object, &PyJiterable_Type);
}

/*
 * Checks if the object is a pyjiterable.
 */
int pyjiterable_check(PyObject *obj) {
    if(PyObject_TypeCheck(obj, &PyJiterable_Type))
        return 1;
    return 0;
}

/*
 * Gets the iterator for the object.
 */
PyObject* pyjiterable_getiter(PyObject* obj) {
    jmethodID         getiter  = NULL;
    jobject           iter     = NULL;
    PyJobject_Object *pyjob    = (PyJobject_Object*) obj;
    JNIEnv           *env      = pyembed_get_env();

    getiter = (*env)->GetMethodID(env, pyjob->clazz, "iterator", "()Ljava/util/Iterator;");
    if(process_java_exception(env) || !getiter) {
        return NULL;
    }

    iter = (*env)->CallObjectMethod(env, pyjob->object, getiter);
    if(process_java_exception(env) || !iter) {
        return NULL;
    }
    return pyjobject_new(env, iter);
}


static PyMethodDef pyjiterable_methods[] = {
    {NULL, NULL, 0, NULL}
};


/*
 * Inherits from PyJobject_Type
 */
PyTypeObject PyJiterable_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJiterable",
    sizeof(PyJiterable_Object),
    0,
    0,                                        /* tp_dealloc */
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
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    "jiterable",                              /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    (getiterfunc) pyjiterable_getiter,        /* tp_iter */
    0,                                        /* tp_iternext */
    pyjiterable_methods,                      /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0, // &PyJobject_Type                     /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
