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


PyObject* PyJMonitor_New(jobject obj)
{
    PyJMonitorObject *monitor = NULL;
    JNIEnv           *env     = pyembed_get_env();

    if (PyType_Ready(&PyJMonitor_Type) < 0) {
        return NULL;
    }

    monitor = PyObject_NEW(PyJMonitorObject,
                           &PyJMonitor_Type);
    monitor->lock = (*env)->NewGlobalRef(env, obj);
    if (process_java_exception(env)) {
        return NULL;
    }
    return (PyObject*) monitor;
}


int PyJMonitor_Check(PyObject *obj)
{
    if (PyObject_TypeCheck(obj, &PyJMonitor_Type)) {
        return 1;
    }
    return 0;
}


/*
 * Enters the Python ContextManager and intrinsically locks on the object.
 * Will wait for the lock if it is locked by something else, just like
 * a Java synchronized block.
 */
static PyObject* pyjmonitor_enter(PyObject* self, PyObject* args)
{
    PyJMonitorObject *monitor = (PyJMonitorObject*) self;
    JNIEnv           *env     = env = pyembed_get_env();
    int               failed  = 0;

    /*
     * We absolutely cannot have the GIL when we attempt to synchronize on the
     * intrinsic lock. Otherwise we can potentially deadlock if this locking
     * operation is blocked but holds the GIL while another thread has the lock
     * but is awaiting the GIL.
     */
    Py_BEGIN_ALLOW_THREADS
    if ((*env)->MonitorEnter(env, monitor->lock) < 0) {
        process_java_exception(env);
        failed = 1;
    }
    Py_END_ALLOW_THREADS
    if (failed) {
        return NULL;
    }

    Py_INCREF(self);
    return self;
}

/*
 * Exits the Python ContextManager and releases the intrinsic lock on the
 * object.
 */
static PyObject* pyjmonitor_exit(PyObject* self, PyObject* args)
{
    PyJMonitorObject *monitor  = (PyJMonitorObject*) self;
    JNIEnv           *env      = env = pyembed_get_env();

    if ((*env)->MonitorExit(env, monitor->lock) < 0) {
        process_java_exception(env);
        return NULL;
    }

    Py_RETURN_NONE;
}

void pyjmonitor_dealloc(PyJMonitorObject *self)
{
#if USE_DEALLOC
    JNIEnv *env = pyembed_get_env();
    if (env) {
        if (self->lock) {
            (*env)->DeleteGlobalRef(env, self->lock);
        }
    }

    PyObject_Del(self);
#endif
}


static PyMethodDef pyjmonitor_methods[] = {
    {
        "__enter__",
        pyjmonitor_enter,
        METH_NOARGS,
        "__enter__ for Python ContextManager that locks"
    },

    {
        "__exit__",
        pyjmonitor_exit,
        METH_VARARGS,
        "__exit__ for Python ContextManager that unlocks"
    },

    { NULL, NULL }
};


/*
 * Inherits from PyJObject_Type
 */
PyTypeObject PyJMonitor_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJMonitor",
    sizeof(PyJMonitorObject),
    0,
    (destructor) pyjmonitor_dealloc,          /* tp_dealloc */
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
    PyObject_GenericGetAttr,                  /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    "jmonitor",                               /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    pyjmonitor_methods,                       /* tp_methods */
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
