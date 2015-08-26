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

#include "pyjmethodwrapper.h"
#include "pyjobject.h"
#include "pyembed.h"



static void pyjmethodwrapper_dealloc(PyJmethodWrapper_Object *self);

/*
 * Makes a new pyjmethodwrapper.  This is required to reuse the pyjmethod
 * objects across multiple instances, otherwise we'd lose track of which
 * pyjobject is doing the calling.
 */
PyJmethodWrapper_Object* pyjmethodwrapper_new(PyJobject_Object *pyjobject, // the instance doing the calling
        PyJmethod_Object *pyjmethod) { // the method to be called
    PyJmethodWrapper_Object *pym = NULL;

    if(PyType_Ready(&PyJmethodWrapper_Type) < 0)
        return NULL;

    pym = PyObject_NEW(PyJmethodWrapper_Object, &PyJmethodWrapper_Type);
    pym->method = pyjmethod;
    pym->object = pyjobject;
    Py_INCREF(pyjobject);
    return pym;
}

static void pyjmethodwrapper_dealloc(PyJmethodWrapper_Object *self) {
#if USE_DEALLOC
    Py_CLEAR(self->object);
    PyObject_Del(self);
#endif
}



/*
 * Called by python for pyjobject.__call__.  Since we reuse pyjmethod objects,
 * a unique instance of pyjmethodwrapper is created for every method on a
 * pyjobject to ensure the correct instance is used.  We then pass off the
 * actual invocation to the pyjmethod.
 */
static PyObject* pyjmethodwrapper_call(PyJmethodWrapper_Object *self,
                                PyObject *args,
                                PyObject *keywords) {
    PyObject *ret;
    PyJobject_Object* obj;
    
    if(!PyTuple_Check(args)) {
        PyErr_Format(PyExc_RuntimeError, "args is not a valid tuple");
        return NULL;
    }
    
    if(keywords != NULL) {
        PyErr_Format(PyExc_RuntimeError, "Keywords are not supported.");
        return NULL;
    }

    obj = self->object;
    // pyjobject_find_method will actually call the method
    ret = pyjobject_find_method(obj, self->method->pyMethodName, args);

    return ret;
}


static PyMethodDef pyjmethodwrapper_methods[] = {
    {NULL, NULL, 0, NULL}
};


PyTypeObject PyJmethodWrapper_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJmethodWrapper",
    sizeof(PyJmethodWrapper_Object),
    0,
    (destructor) pyjmethodwrapper_dealloc,    /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    (ternaryfunc) pyjmethodwrapper_call,      /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    "jmethodwrapper",                         /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    pyjmethodwrapper_methods,                 /* tp_methods */
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
