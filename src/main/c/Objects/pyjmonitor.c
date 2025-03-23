/*
   jep - Java Embedded Python

   Copyright (c) 2017-2025 JEP AUTHORS.

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

    JepModuleState* modState = pyembed_get_module_state();
    if (!modState) {
        return NULL;
    }

    PyTypeObject* tp = modState->PyJMonitor_Type;
    monitor = (PyJMonitorObject*) tp->tp_alloc(tp, 0);
    monitor->lock = (*env)->NewGlobalRef(env, obj);
    if (process_java_exception(env)) {
        return NULL;
    }
    return (PyObject*) monitor;
}


/*
 * Enters the Python ContextManager and intrinsically locks on the object.
 * Will wait for the lock if it is locked by something else, just like
 * a Java synchronized block.
 */
static PyObject* pyjmonitor_enter(PyObject* self, PyObject* args)
{
    PyJMonitorObject *monitor = (PyJMonitorObject*) self;
    JNIEnv           *env     = pyembed_get_env();
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
    JNIEnv           *env      = pyembed_get_env();

    if ((*env)->MonitorExit(env, monitor->lock) < 0) {
        process_java_exception(env);
        return NULL;
    }

    Py_RETURN_NONE;
}

void pyjmonitor_dealloc(PyJMonitorObject *self)
{
#if USE_DEALLOC
    PyObject_GC_UnTrack(self);
    PyTypeObject *tp = Py_TYPE(self);
    JNIEnv *env = pyembed_get_env();
    if (env) {
        if (self->lock) {
            (*env)->DeleteGlobalRef(env, self->lock);
        }
    }

    tp->tp_free((PyObject*) self);
    Py_DECREF(tp);
#endif
}


static int pyjmonitor_traverse(PyJMonitorObject *self, visitproc visit, void *arg)
{
    Py_VISIT(Py_TYPE(self));
    return 0;
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

static PyType_Slot slots[] = {
    {Py_tp_doc, "jmonitor"},
    {Py_tp_dealloc, pyjmonitor_dealloc},
    {Py_tp_traverse, pyjmonitor_traverse},
    {Py_tp_getattro, PyObject_GenericGetAttr},
    {Py_tp_methods, pyjmonitor_methods},
    {0, NULL},
};

PyType_Spec PyJMonitor_Spec = {
    .name = "jep.PyJMonitor",
    .basicsize = sizeof(PyJMonitorObject),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .slots = slots
};
