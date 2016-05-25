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

jmethodID mapSize        = 0;
jmethodID mapGet         = 0;
jmethodID mapPut         = 0;
jmethodID mapRemove      = 0;
jmethodID mapContainsKey = 0;
jmethodID mapKeySet      = 0;
jmethodID mapKeyItr      = 0;

static Py_ssize_t pyjmap_len(PyObject*);
static PyObject* pyjmap_getitem(PyObject*, PyObject*);
static int pyjmap_setitem(PyObject*, PyObject*, PyObject*);


/*
 * News up a pyjmap, which is just a pyjobject with some mapping methods
 * attached to it.  This should only be called from pyjobject_new().
 */
PyJMapObject* pyjmap_new()
{
    // pyjobject will have already initialized PyJMap_Type
    return PyObject_NEW(PyJMapObject, &PyJMap_Type);
}

/*
 * Checks if the object is a pyjmap.
 */
int pyjmap_check(PyObject *obj)
{
    if (PyObject_TypeCheck(obj, &PyJMap_Type)) {
        return 1;
    }
    return 0;
}

/*
 * Gets the size of the map.
 */
static Py_ssize_t pyjmap_len(PyObject *self)
{
    Py_ssize_t    len   = 0;
    PyJObject    *pyjob = (PyJObject*) self;
    JNIEnv       *env   = pyembed_get_env();

    if (mapSize == 0) {
        mapSize = (*env)->GetMethodID(env, JMAP_TYPE, "size", "()I");
        if (process_java_exception(env) || !mapSize) {
            return -1;
        }
    }

    len = (*env)->CallIntMethod(env, pyjob->object, mapSize);
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
    jboolean      result      = JNI_FALSE;
    PyJObject    *obj         = (PyJObject*) self;
    JNIEnv       *env         = pyembed_get_env();
    jobject       jkey        = NULL;

    if (key == Py_None) {
        jkey = NULL;
    } else {
        jkey = pyembed_box_py(env, key);
        if (process_java_exception(env)) {
            return -1;
        } else if (!jkey) {
            /*
             * with the way pyembed_box_py is currently implemented, shouldn't
             * be able to get here
             */
            PyObject *pystring = PyObject_Str((PyObject*) Py_TYPE(key));
            PyErr_Format(PyExc_TypeError,
                         "__contains__ received an incompatible type: %s",
                         PyString_AsString(pystring));
            Py_XDECREF(pystring);
            return -1;
        }
    }

    if (mapContainsKey == 0) {
        mapContainsKey = (*env)->GetMethodID(env, JMAP_TYPE, "containsKey",
                                             "(Ljava/lang/Object;)Z");
        if (process_java_exception(env) || !mapContainsKey) {
            return -1;
        }
    }

    result = (*env)->CallBooleanMethod(env, obj->object, mapContainsKey, jkey);
    if (process_java_exception(env)) {
        return -1;
    }

    if (result) {
        return 1;
    } else {
        return 0;
    }
}


/*
 * Method for the getting items with the [key] operator on pyjmap.  For
 * example, result = o[key]
 */
static PyObject* pyjmap_getitem(PyObject *o, PyObject *key)
{
    jobject       jkey = NULL;
    jobject       val  = NULL;
    PyJObject    *obj  = (PyJObject*) o;
    JNIEnv       *env  = pyembed_get_env();

    if (mapGet == 0) {
        mapGet = (*env)->GetMethodID(env, JMAP_TYPE, "get",
                                     "(Ljava/lang/Object;)Ljava/lang/Object;");
        if (process_java_exception(env) || !mapGet) {
            return NULL;
        }
    }

    if (pyjobject_check(key)) {
        jkey = ((PyJObject*) key)->object;
    } else {
        /*
         * convert_pyarg_jvalue will leave jkey as NULL and set PyExc_TypeError
         * if we can't handle the key type, which matches python's guidelines
         */
        jvalue jvkey = convert_pyarg_jvalue(env, key, JOBJECT_TYPE, JOBJECT_ID, 1);
        jkey = jvkey.l;
        if (process_java_exception(env) || !jkey) {
            return NULL;
        }
    }

    val = (*env)->CallObjectMethod(env, obj->object, mapGet, jkey);
    if (process_java_exception(env)) {
        return NULL;
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
                         PyString_AsString(pystr));
            Py_XDECREF(pystr);
            return NULL;
        }
    }

    return convert_jobject_pyobject(env, val);
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

    if (v == NULL) {
        // this is a del PyJMap[key] statement
        if (!pyjmap_contains_key(o, key)) {
            PyObject *pystr = PyObject_Str(key);
            PyErr_Format(PyExc_KeyError,
                         "KeyError: %s",
                         PyString_AsString(pystr));
            Py_XDECREF(pystr);
            return -1;
        }

        if (mapRemove == 0) {
            mapRemove = (*env)->GetMethodID(env, JMAP_TYPE, "remove",
                                            "(Ljava/lang/Object;)Ljava/lang/Object;");
            if (process_java_exception(env) || !mapRemove) {
                return -1;
            }
        }

        if (pyjobject_check(key)) {
            jkey = ((PyJObject*) key)->object;
        } else {
            jvalue jvkey = convert_pyarg_jvalue(env, key, JOBJECT_TYPE, JOBJECT_ID, 1);
            jkey = jvkey.l;
            if (process_java_exception(env) || !jkey) {
                return -1;
            }
        }

        (*env)->CallObjectMethod(env, obj->object, mapRemove, jkey);
        if (process_java_exception(env)) {
            return -1;
        }

        // have to return 0 on success even though it's not documented
        return 0;
    } else if (v == Py_None) {
        value = NULL;
    } else {
        value = pyembed_box_py(env, v);
        if (process_java_exception(env)) {
            return -1;
        } else if (!value) {
            /*
             * with the way pyembed_box_py is currently implemented, shouldn't
             * be able to get here
             */
            PyObject *pystring = PyObject_Str((PyObject*) Py_TYPE(v));
            PyErr_Format(PyExc_TypeError,
                         "__setitem__ received an incompatible type: %s",
                         PyString_AsString(pystring));
            Py_XDECREF(pystring);
            return -1;
        }
    }

    if (pyjobject_check(key)) {
        jkey = ((PyJObject*) key)->object;
    } else {
        jvalue jvkey = convert_pyarg_jvalue(env, key, JOBJECT_TYPE, JOBJECT_ID, 1);
        jkey = jvkey.l;
        if (process_java_exception(env) || !jkey) {
            return -1;
        }
    }

    if (mapPut == 0) {
        mapPut = (*env)->GetMethodID(env, JMAP_TYPE, "put",
                                     "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
        if (process_java_exception(env) || !mapPut) {
            return -1;
        }
    }

    (*env)->CallObjectMethod(env, obj->object, mapPut, jkey, value);
    if (process_java_exception(env)) {
        return -1;
    }

    // have to return 0 on success even though it's not documented
    return 0;
}


/*
 * Method for iterating over the keys of the dictionary.  For example,
 * for key in o:
 */
PyObject* pyjmap_getiter(PyObject* obj)
{
    jobject       set      = NULL;
    jobject       iter     = NULL;
    PyJObject    *pyjob    = (PyJObject*) obj;
    JNIEnv       *env      = pyembed_get_env();

    if (mapKeySet == 0) {
        mapKeySet = (*env)->GetMethodID(env, JMAP_TYPE, "keySet", "()Ljava/util/Set;");
        if (process_java_exception(env) || !mapKeySet) {
            return NULL;
        }
    }

    set = (*env)->CallObjectMethod(env, pyjob->object, mapKeySet);
    if (process_java_exception(env) || !set) {
        return NULL;
    }

    if (mapKeyItr == 0) {
        mapKeyItr = (*env)->GetMethodID(env, JCOLLECTION_TYPE, "iterator",
                                        "()Ljava/util/Iterator;");
        if (process_java_exception(env) || !mapKeyItr) {
            return NULL;
        }
    }

    iter = (*env)->CallObjectMethod(env, set, mapKeyItr);
    if (process_java_exception(env) || !iter) {
        return NULL;
    }

    return pyjobject_new(env, iter);
}


static PyMethodDef pyjmap_methods[] = {
    {NULL, NULL, 0, NULL}
};

static PySequenceMethods pyjmap_seq_methods = {
    0,                          /* sq_length */
    0,                          /* sq_concat */
    0,                          /* sq_repeat */
    0,                          /* sq_item */
    0,                          /* sq_slice */
    0,                          /* sq_ass_item */
    0,                          /* sq_ass_slice */
    pyjmap_contains_key,        /* sq_contains */
    0,                          /* sq_inplace_concat */
    0,                          /* sq_inplace_repeat */
};

static PyMappingMethods pyjmap_map_methods = {
    pyjmap_len,           /* mp_length */
    pyjmap_getitem,       /* mp_subscript */
    pyjmap_setitem        /* mp_ass_subscript */
};


/*
 * Inherits from PyJObject_Type
 */
PyTypeObject PyJMap_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJMap",
    sizeof(PyJMapObject),
    0,
    0,                                        /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    &pyjmap_seq_methods,                      /* tp_as_sequence */
    &pyjmap_map_methods,                      /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    "jmap",                                   /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    (getiterfunc) pyjmap_getiter,             /* tp_iter */
    0,                                        /* tp_iternext */
    pyjmap_methods,                           /* tp_methods */
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
