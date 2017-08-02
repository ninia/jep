/*
   jep - Java Embedded Python

   Copyright (c) 2017 JEP AUTHORS.

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


PyJObject* PyJAutoCloseable_New()
{
    // PyJObject will have already initialized PyJAutoCloseable_Type
    return (PyJObject*) PyObject_NEW(PyJAutoCloseableObject,
                                     &PyJAutoCloseable_Type);
}


int PyJAutoCloseable_Check(PyObject *obj)
{
    if (PyObject_TypeCheck(obj, &PyJAutoCloseable_Type)) {
        return 1;
    }
    return 0;
}

/*
 * Enters the Python ContextManager.
 */
static PyObject* pyjautocloseable_enter(PyObject* self, PyObject* args)
{
    Py_INCREF(self);
    return self;
}

/*
 * Exits the Python ContextManager and calls java.lang.AutoCloseable.close().
 */
static PyObject* pyjautocloseable_exit(PyObject* self, PyObject* args)
{
    PyJObject    *pyjob    = (PyJObject*) self;
    JNIEnv       *env      = pyembed_get_env();

    java_lang_AutoCloseable_close(env, pyjob->object);
    if (process_java_exception(env)) {
        return NULL;
    }

    Py_RETURN_NONE;
}


static PyMethodDef pyjautocloseable_methods[] = {
    {
        "__enter__",
        pyjautocloseable_enter,
        METH_NOARGS,
        "__enter__ for Python ContextManager"
    },

    {
        "__exit__",
        pyjautocloseable_exit,
        METH_VARARGS,
        "__exit__ for Python ContextManager"
    },

    { NULL, NULL }
};


/*
 * Inherits from PyJObject_Type
 */
PyTypeObject PyJAutoCloseable_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJAutoCloseable",
    sizeof(PyJAutoCloseableObject),
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
    "jautocloseable",                         /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    pyjautocloseable_methods,                 /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0, // &PyJObject_Type                     /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
