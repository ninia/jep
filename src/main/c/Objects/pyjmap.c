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
 * Gets the size of the map.
 */
static Py_ssize_t pyjmap_len(PyObject *self)
{
    Py_ssize_t    len   = 0;
    PyJObject    *pyjob = (PyJObject*) self;
    JNIEnv       *env   = pyembed_get_env();

    len = java_util_Map_size(env, pyjob->object);
    if (process_java_exception(env)) {
        return -1;
    }
    return len;
}

/*
 * Method for checking if a key is in the dictionary.  For example,
 * if key in o:
 */
static int pyjmap_contains_key(PyObject *self, PyObject *key)
{
    jboolean     jresult      = JNI_FALSE;
    PyJObject    *obj         = (PyJObject*) self;
    JNIEnv       *env         = pyembed_get_env();
    jobject       jkey        = NULL;
    int           result   = -1;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return -1;
    }
    jkey = PyObject_As_jobject(env, key, JOBJECT_TYPE);
    if (!jkey && PyErr_Occurred()) {
        goto FINALLY;
    }

    jresult = java_util_Map_containsKey(env, obj->object, jkey);
    if (process_java_exception(env)) {
        goto FINALLY;
    }

    if (jresult) {
        result = 1;
    } else {
        result = 0;
    }
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}

/*
 * Method for the getting items with the [key] operator on pyjmap.  For
 * example, result = o[key]
 */
static PyObject* pyjmap_getitem(PyObject *o, PyObject *key)
{
    jobject       jkey   = NULL;
    jobject       val    = NULL;
    PyJObject    *obj    = (PyJObject*) o;
    JNIEnv       *env    = pyembed_get_env();
    PyObject     *result = NULL;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return NULL;
    }
    jkey = PyObject_As_jobject(env, key, JOBJECT_TYPE);
    if (!jkey && PyErr_Occurred()) {
        goto FINALLY;
    }

    val = java_util_Map_get(env, obj->object, jkey);
    if (process_java_exception(env)) {
        goto FINALLY;
    }

    if (!val) {
        /*
         * Python docs indicate KeyError should be set if the key is not in the
         * container, but some Maps allow null values. So we have to check.
         */
        if (!pyjmap_contains_key(o, key)) {
            PyObject *pystr = PyObject_Str(key);
            PyErr_Format(PyExc_KeyError,
                         "KeyError: %s",
                         PyUnicode_AsUTF8(pystr));
            Py_XDECREF(pystr);
            goto FINALLY;
        }
    }

    result = jobject_As_PyObject(env, val);
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}

/*
 * Method for the setting items with the [key] operator on pyjmap.  For example,
 * o[key] = v.  Also supports del o[key]
 */
static int pyjmap_setitem(PyObject *o, PyObject *key, PyObject *v)
{
    jobject       jkey     = NULL;
    jobject       value    = NULL;
    PyJObject    *obj      = (PyJObject*) o;
    JNIEnv       *env      = pyembed_get_env();
    int           result   = -1;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return -1;
    }

    if (v == NULL) {
        // this is a del PyJMap[key] statement
        if (!pyjmap_contains_key(o, key)) {
            PyObject *pystr = PyObject_Str(key);
            PyErr_Format(PyExc_KeyError,
                         "KeyError: %s",
                         PyUnicode_AsUTF8(pystr));
            Py_XDECREF(pystr);
            goto FINALLY;
        }

        jkey = PyObject_As_jobject(env, key, JOBJECT_TYPE);
        if (!jkey && PyErr_Occurred()) {
            goto FINALLY;
        }

        java_util_Map_remove(env, obj->object, jkey);
        if (process_java_exception(env)) {
            goto FINALLY;
        }
    } else {
        value = PyObject_As_jobject(env, v, JOBJECT_TYPE);
        if (!value && PyErr_Occurred()) {
            goto FINALLY;
        }

        jkey = PyObject_As_jobject(env, key, JOBJECT_TYPE);
        if (!jkey && PyErr_Occurred()) {
            return -1;
        }

        java_util_Map_put(env, obj->object, jkey, value);
        if (process_java_exception(env)) {
            goto FINALLY;
        }
    }
    // have to return 0 on success even though it's not documented
    result = 0;
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}

/*
 * Method for iterating over the keys of the dictionary.  For example,
 * for key in o:
 */
static PyObject* pyjmap_getiter(PyObject* obj)
{
    jobject       set      = NULL;
    jobject       iter     = NULL;
    PyJObject    *pyjob    = (PyJObject*) obj;
    PyObject     *result   = NULL;
    JNIEnv       *env      = pyembed_get_env();

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return NULL;
    }
    set = java_util_Map_keySet(env, pyjob->object);
    if (process_java_exception(env) || !set) {
        goto FINALLY;
    }

    iter = java_lang_Iterable_iterator(env, set);
    if (process_java_exception(env) || !iter) {
        goto FINALLY;
    }

    result = jobject_As_PyObject(env, iter);
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}

static PyObject* pyjmap_keys(PyObject* self, PyObject* args)
{
    jobject    keyset = NULL;
    PyObject  *result = NULL;
    PyJObject *pyjob  = (PyJObject*) self;
    JNIEnv    *env    = pyembed_get_env();

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return NULL;
    }

    keyset = java_util_Map_keySet(env, pyjob->object);
    if (process_java_exception(env)) {
        goto FINALLY;
    }

    result = jobject_As_PyObject(env, keyset);
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}

static PyObject* pyjmap_items(PyObject* self, PyObject* args)
{
    jobject    entrySet = NULL;
    jobject    itr      = NULL;
    PyObject  *pylist   = NULL;
    PyObject  *result   = NULL;
    int        size     = 0;
    int        index    = 0;
    PyJObject *pyjob    = (PyJObject*) self;
    JNIEnv    *env      = pyembed_get_env();

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
    }

    entrySet = java_util_Map_entrySet(env, pyjob->object);
    if (!entrySet) {
        if (!process_java_exception(env)) {
            PyErr_SetString(PyExc_RuntimeError, "Map.entrySet() returned null");
        }
        goto FINALLY;
    }

    size = java_util_Map_size(env, pyjob->object);
    if (process_java_exception(env)) {
        goto FINALLY;
    }

    itr = java_lang_Iterable_iterator(env, entrySet);
    if (!itr) {
        if (!process_java_exception(env)) {
            PyErr_SetString(PyExc_RuntimeError, "Map.entrySet().iterator() returned null");
        }
        goto FINALLY;
    }

    pylist = PyList_New(size);
    for (index = 0;  index < size; index++) {
        jobject  next;
        jobject  key;
        jobject  value;
        PyObject *pykey;
        PyObject *pyval;
        PyObject *pytuple;

        next = java_util_Iterator_next(env, itr);
        if (!next) {
            if (!process_java_exception(env)) {
                PyErr_SetString(PyExc_RuntimeError,
                                "Map.entrySet().iterator().next() returned null");
            }
            Py_DECREF(pylist);
            goto FINALLY;
        }

        // convert Map.Entry's key to a PyObject*
        key = java_util_Map_Entry_getKey(env, next);
        if (process_java_exception(env)) {
            Py_DECREF(pylist);
            goto FINALLY;
        }
        pykey = jobject_As_PyObject(env, key);
        if (!pykey) {
            Py_DECREF(pylist);
            goto FINALLY;
        }

        // convert Map.Entry's value to a PyObject*
        value = java_util_Map_Entry_getValue(env, next);
        if (process_java_exception(env)) {
            Py_DECREF(pykey);
            Py_DECREF(pylist);
            goto FINALLY;
        }
        pyval = jobject_As_PyObject(env, value);
        if (!pyval) {
            Py_DECREF(pykey);
            Py_DECREF(pylist);
            goto FINALLY;
        }

        pytuple = PyTuple_Pack(2, pykey, pyval);
        if (!pytuple) {
            Py_DECREF(pykey);
            Py_DECREF(pyval);
            Py_DECREF(pylist);
            goto FINALLY;
        }
        Py_DECREF(pykey);
        Py_DECREF(pyval);

        if (PyList_SetItem(pylist, index, pytuple) != 0) {
            Py_DECREF(pytuple);
            Py_DECREF(pylist);
            goto FINALLY;
        }

        (*env)->DeleteLocalRef(env, next);
        if (key) {
            (*env)->DeleteLocalRef(env, key);
        }
        if (value) {
            (*env)->DeleteLocalRef(env, value);
        }

    }  // end of for loop

    result = pylist;
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}

static PyMethodDef pyjmap_methods[] = {
    {
        "keys",
        pyjmap_keys,
        METH_NOARGS,
        "Returns a list of keys in the map"
    },
    {
        "items",
        pyjmap_items,
        METH_NOARGS,
        "Returns a list of the items in the map, where each item is a tuple containing a key-value pair"
    },
    { NULL, NULL }
};

static PyType_Slot slots[] = {
    {Py_tp_doc, "Jep java.util.Map"},
    {Py_tp_iter, (void*) pyjmap_getiter},
    // sequence slots
    {Py_sq_contains, (void*) pyjmap_contains_key},
    // mapping slots
    {Py_mp_length, (void*) pyjmap_len},
    {Py_mp_subscript, (void*) pyjmap_getitem},
    {Py_mp_ass_subscript, (void*) pyjmap_setitem},
    // methods slot
    {Py_tp_methods, (void*) pyjmap_methods},
    {0, NULL},
};
PyType_Spec PyJMap_Spec = {
    .name = "java.util.Map",
    .basicsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = slots,
};
