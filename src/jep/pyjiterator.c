/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/*
   jep - Java Embedded Python

   Copyright (c) 2016 JEP AUTHORS.

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

jmethodID itrHasNext = 0;
jmethodID itrNext    = 0;

/*
 * News up a pyjiterator, which is just a pyjobject for iterators.
 */
PyJIteratorObject* pyjiterator_new()
{
    /*
     * MSVC requires tp_base to be set here
     * See https://docs.python.org/2/extending/newtypes.html
     */
    if (!PyJIterator_Type.tp_base) {
        PyJIterator_Type.tp_base = &PyJObject_Type;
    }

    if (PyType_Ready(&PyJIterator_Type) < 0) {
        return NULL;
    }

    return PyObject_NEW(PyJIteratorObject, &PyJIterator_Type);
}

/*
 * Checks if the object is a pyjiterator.
 */
int pyjiterator_check(PyObject *obj)
{
    if (PyObject_TypeCheck(obj, &PyJIterator_Type)) {
        return 1;
    }
    return 0;
}

PyObject* pyjiterator_next(PyObject* self)
{
    jboolean      nextAvail = JNI_FALSE;
    PyJObject    *pyjob     = (PyJObject*) self;
    JNIEnv       *env       = pyembed_get_env();

    if (!JNI_METHOD(itrHasNext, env, JITERATOR_TYPE, "hasNext", "()Z")) {
        process_java_exception(env);
        return NULL;
    }

    nextAvail = (*env)->CallBooleanMethod(env, pyjob->object, itrHasNext);
    if (process_java_exception(env)) {
        return NULL;
    }

    if (nextAvail) {
        jobject   nextItem;
        PyObject* result;

        if (!JNI_METHOD(itrNext, env, JITERATOR_TYPE, "next", "()Ljava/lang/Object;")) {
            process_java_exception(env);
            return NULL;
        }
        if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
            process_java_exception(env);
            return NULL;
        }
        nextItem = (*env)->CallObjectMethod(env, pyjob->object, itrNext);
        if (process_java_exception(env)) {
            (*env)->PopLocalFrame(env, NULL);
            return NULL;
        }

        result = convert_jobject_pyobject(env, nextItem);
        (*env)->PopLocalFrame(env, NULL);
        return result;
    }

    return NULL;
}


static PyMethodDef pyjiterator_methods[] = {
    {NULL, NULL, 0, NULL}
};


/*
 * Inherits from PyJObject_Type
 */
PyTypeObject PyJIterator_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJIterator",
    sizeof(PyJIteratorObject),
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
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    "jiterator",                              /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    PyObject_SelfIter,                        /* tp_iter */
    (iternextfunc) pyjiterator_next,          /* tp_iternext */
    pyjiterator_methods,                      /* tp_methods */
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
