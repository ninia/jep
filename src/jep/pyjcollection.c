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
#include "pyjcollection.h"
#include "pyembed.h"

static Py_ssize_t pyjcollection_len(PyObject*);
static int pyjcollection_contains(PyObject*, PyObject*);


/*
 * News up a pyjcollection, which is just a pyjiterable with a few methods
 * attached to it.  This should only be called from pyjobject_new().
 */
PyJcollection_Object* pyjcollection_new() {
    // pyjobject will have already initialized PyJcollection_Type
    return PyObject_NEW(PyJcollection_Object, &PyJcollection_Type);
}


/*
 * Checks if the object is a pyjcollection.
 */
int pyjcollection_check(PyObject *obj) {
    if(PyObject_TypeCheck(obj, &PyJcollection_Type))
        return 1;
    return 0;
}

/*
 * Gets the size of the collection.
 */
static Py_ssize_t pyjcollection_len(PyObject* self) {
    jmethodID         size  = NULL;
    Py_ssize_t        len   = 0;
    PyJobject_Object *pyjob = (PyJobject_Object*) self;
    JNIEnv           *env   = pyembed_get_env();

    size = (*env)->GetMethodID(env, pyjob->clazz, "size", "()I");
    if(process_java_exception(env) || !size) {
        return -1;
    }

    len = (*env)->CallIntMethod(env, pyjob->object, size);
    if(process_java_exception(env)) {
        return -1;
    }
    return len;
}


/*
 * Method for the __contains__() method on pyjcollection, frequently used by the
 * in operator.  For example, if v in o:
 */
static int pyjcollection_contains(PyObject *o, PyObject *v) {
    jmethodID         contains = NULL;
    jboolean          result   = JNI_FALSE;
    PyJobject_Object *obj      = (PyJobject_Object*) o;
    JNIEnv           *env      = pyembed_get_env();
    jobject           value    = NULL;

    if(v == Py_None) {
        value = NULL;
    } else {
        value = pyembed_box_py(env, v);
        if(process_java_exception(env)) {
            return -1;
        } else if(!value) {
            /*
             * with the way pyembed_box_py is currently implemented, shouldn't
             * be able to get here
             */
            PyObject *pystring = PyObject_Str((PyObject*) Py_TYPE(v));
            PyErr_Format(PyExc_TypeError,
                        "__contains__ received an incompatible type: %s",
                        PyString_AsString(pystring));
            Py_XDECREF(pystring);
            return -1;
        }
    }

    contains = (*env)->GetMethodID(env, obj->clazz, "contains", "(Ljava/lang/Object;)Z");
    if(process_java_exception(env) || !contains) {
        return -1;
    }

    result = (*env)->CallBooleanMethod(env, obj->object, contains, value);
    if(process_java_exception(env)) {
        return -1;
    }

    if(result) {
        return 1;
    } else {
        return 0;
    }
}


static PyMethodDef pyjcollection_methods[] = {
    {NULL, NULL, 0, NULL}
};

static PySequenceMethods pyjcollection_seq_methods = {
        pyjcollection_len,      /* sq_length */
        0,                      /* sq_concat */
        0,                      /* sq_repeat */
        0,                      /* sq_item */
        0,                      /* sq_slice */
        0,                      /* sq_ass_item */
        0,                      /* sq_ass_slice */
        pyjcollection_contains, /* sq_contains */
        0,                      /* sq_inplace_concat */
        0,                      /* sq_inplace_repeat */
};


/*
 * Inherits from PyJiterable_Type
 */
PyTypeObject PyJcollection_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJcollection",
    sizeof(PyJcollection_Object),
    0,
    0,                                        /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    &pyjcollection_seq_methods,               /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |                      /* tp_flags */
    Py_TPFLAGS_BASETYPE,
    "jcollection",                            /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0, // inherited                           /* tp_iter */
    0,                                        /* tp_iternext */
    pyjcollection_methods,                    /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0, // &PyJiterable_Type                   /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
