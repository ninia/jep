/*
   jep - Java Embedded Python

   Copyright (c) 2015-2019 JEP AUTHORS.

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

static PyObject* pyjiterator_next(PyObject* self)
{
    jboolean      nextAvail = JNI_FALSE;
    PyJObject    *pyjob     = (PyJObject*) self;
    JNIEnv       *env       = pyembed_get_env();

    nextAvail = java_util_Iterator_hasNext(env, pyjob->object);
    if (process_java_exception(env)) {
        return NULL;
    }

    if (nextAvail) {
        jobject   nextItem;
        PyObject* result;

        if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
            process_java_exception(env);
            return NULL;
        }
        nextItem = java_util_Iterator_next(env, pyjob->object);
        if (process_java_exception(env)) {
            (*env)->PopLocalFrame(env, NULL);
            return NULL;
        }

        result = jobject_As_PyObject(env, nextItem);
        (*env)->PopLocalFrame(env, NULL);
        return result;
    }

    return NULL;
}


/*
 * Inherits from PyJObject_Type
 */
PyTypeObject *PyJIterator_Type;
int jep_jiterator_type_ready() {
    static PyType_Slot slots[] = {
            {Py_tp_doc, "jiterator"},
            {Py_tp_iter, (void*) PyObject_SelfIter},
            {Py_tp_iternext, (void*) pyjiterator_next},
            {0, NULL},
    };
    PyType_Spec spec = {
            .name = "jep.PyJIterator",
            .basicsize = sizeof(PyJObject),
            .flags = Py_TPFLAGS_DEFAULT,
            .slots = &[
            {Py_tp_dealloc, (void*) pyjclass_dealloc},
            {Py_tp_class, (void*) pyjclass_call},
            {Py_tp_doc, "jclass"},
            {0, NULL}
            ]
    };
    PyJIterator_Type = PyType_FromSpecWithBases(&spec, (PyObject*) PyJObject_Type);
    return PyType_Ready(PyJIterator_Type);
}