/*
   jep - Java Embedded Python

   Copyright (c) 2017-2022 JEP AUTHORS.

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

static PyType_Slot slots[] = {
    {Py_tp_doc, "Jep java.lang.AutoCloseable"},
    {Py_tp_methods, (void*) pyjautocloseable_methods},
    {0, NULL}
};
PyType_Spec PyJAutoCloseable_Spec = {
    .name = "java.lang.AutoCloseable",
    .basicsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = slots
};
