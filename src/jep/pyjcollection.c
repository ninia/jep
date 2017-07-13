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


PyJObject* PyJCollection_New()
{
    // PyJObject will have already initialized PyJCollection_Type
    return (PyJObject*) PyObject_NEW(PyJCollectionObject, &PyJCollection_Type);
}


int PyJCollection_Check(PyObject *obj)
{
    if (PyObject_TypeCheck(obj, &PyJCollection_Type)) {
        return 1;
    }
    return 0;
}

/*
 * Gets the size of the collection.
 */
static Py_ssize_t pyjcollection_len(PyObject* self)
{
    Py_ssize_t    len   = 0;
    PyJObject    *pyjob = (PyJObject*) self;
    JNIEnv       *env   = pyembed_get_env();

    len = java_util_Collection_size(env, pyjob->object);
    if (process_java_exception(env)) {
        return -1;
    }
    return len;
}


/*
 * Method for the __contains__() method on pyjcollection, frequently used by the
 * in operator.  For example, if v in o:
 */
static int pyjcollection_contains(PyObject *o, PyObject *v)
{
    int           result   = -1;
    jboolean      jresult  = JNI_FALSE;
    PyJObject    *obj      = (PyJObject*) o;
    JNIEnv       *env      = pyembed_get_env();
    jobject       value    = NULL;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return -1;
    }
    value = PyObject_As_jobject(env, v, JOBJECT_TYPE);
    if (!value && PyErr_Occurred()) {
        goto FINALLY;
    }

    jresult = java_util_Collection_contains(env, obj->object, value);
    if (process_java_exception(env)) {
        goto FINALLY;
    }

    if (jresult == JNI_TRUE) {
        result = 1;
    } else {
        result = 0;
    }
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}


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
 * Inherits from PyJIterable_Type
 */
PyTypeObject PyJCollection_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJCollection",
    sizeof(PyJCollectionObject),
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
    0,                                        /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0, // &PyJIterable_Type                   /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
