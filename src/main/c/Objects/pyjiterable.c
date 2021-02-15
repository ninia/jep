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

/*
 * Gets the iterator for the object.
 */
static PyObject* pyjiterable_getiter(PyObject* obj)
{
    jobject       iter     = NULL;
    PyJObject    *pyjob    = (PyJObject*) obj;
    JNIEnv       *env      = pyembed_get_env();
    PyObject     *result   = NULL;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return NULL;
    }

    iter = java_lang_Iterable_iterator(env, pyjob->object);
    if (process_java_exception(env)) {
        goto FINALLY;
    } else if (!iter) {
        PyErr_SetString(PyExc_TypeError,
                        "java.lang.Iterable returned a null value from iterator()");
        goto FINALLY;
    }
    result = jobject_As_PyObject(env, iter);
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}


/*
 * Inherits from PyJObject_Type
 */
PyTypeObject *PyJIterable_Type;
int jep_jiterable_type_ready() {
    static PyType_Slot slots[] = {
            {Py_tp_doc, "jiterable"},
            {Py_tp_iter, (void*) pyjiterable_getiter},
            {Py_tp_new, NULL},
            {0, NULL}
    };
    PyType_Spec spec = {
            .name = "jep.PyJIterable",
            .basicsize = sizeof(PyJObject),
            .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
            .slots = slots
    };
    PyJIterable_Type = (PyTypeObject*) PyType_FromSpecWithBases(&spec, (PyObject*) PyJObject_Type);
    return PyType_Ready(PyJIterable_Type);
};
