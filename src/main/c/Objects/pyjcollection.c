/*
   jep - Java Embedded Python

   Copyright (c) 2015-2022 JEP AUTHORS.

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

static PyType_Slot slots[] = {
    // NOTE: Inherited `tp_iter` from PyJIterable
    {Py_tp_doc, "Jep java.util.Collection"},
    // sequence slots
    {Py_sq_length, (void*) pyjcollection_len},
    {Py_sq_contains, (void*) pyjcollection_contains},
    {0, NULL},
};
PyType_Spec PyJCollection_Spec = {
    .name = "java.util.Collection",
    .basicsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = slots,
};
