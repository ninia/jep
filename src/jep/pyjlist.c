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

#include "pyjlist.h"
#include "pyjobject.h"
#include "pyembed.h"

static PyObject* pyjlist_add(PyObject*, PyObject*);
static PyObject* pyjlist_fill(PyObject*, Py_ssize_t);
static PyObject* pyjlist_getitem(PyObject*, Py_ssize_t);
static PyObject* pyjlist_getslice(PyObject*, Py_ssize_t, Py_ssize_t);
static int pyjlist_setitem(PyObject*, Py_ssize_t, PyObject*);
static int pyjlist_setslice(PyObject*, Py_ssize_t, Py_ssize_t, PyObject*);
static PyObject* pyjlist_inplace_add(PyObject*, PyObject*);
static PyObject* pyjlist_inplace_fill(PyObject*, Py_ssize_t);



/*
 * News up a pyjlist, which is just a pyjobject with some sequence methods
 * attached to it.  This should only be called from pyjobject_new().
 */
PyJlist_Object* pyjlist_new() {
    // pyjobject will have already initialized PyJlist_Type
    return PyObject_NEW(PyJlist_Object, &PyJlist_Type);
}

/*
 * Convenience method to copy a list's items into a new java.util.List of the
 * same type.
 */
PyObject* pyjlist_new_copy(PyObject *toCopy) {
    jmethodID         newInstance = NULL;
    jobject           newList     = NULL;
    jmethodID         addAll      = NULL;
    PyJobject_Object *obj         = (PyJobject_Object*) toCopy;
    JNIEnv           *env         = pyembed_get_env();


    if(!pyjlist_check(toCopy)) {
        PyErr_Format(PyExc_RuntimeError, "pyjlist_new_copy() must receive a pyjlist");
        return NULL;
    }

    newInstance = (*env)->GetMethodID(env, JCLASS_TYPE, "newInstance", "()Ljava/lang/Object;");
    if(process_java_exception(env) || !newInstance) {
        return NULL;
    }

    newList = (*env)->CallObjectMethod(env, obj->clazz, newInstance);
    if(process_java_exception(env) || !newList) {
        return NULL;
    }

    addAll = (*env)->GetMethodID(env, obj->clazz, "addAll", "(Ljava/util/Collection;)Z");
    if(process_java_exception(env) || !addAll) {
        return NULL;
    }

    (*env)->CallBooleanMethod(env, newList, addAll, obj->object);
    if(process_java_exception(env)) {
        return NULL;
    }

    return pyjobject_new(env, newList);
}

/*
 * Checks if the object is a pyjlist.
 */
int pyjlist_check(PyObject *obj) {
    if(PyObject_TypeCheck(obj, &PyJlist_Type))
        return 1;
    return 0;
}


/*
 * Method for the + operator on pyjlist.  For example, result = o1 + o2, where
 * o1 is a pyjlist and result is a new pyjlist.
 */
static PyObject* pyjlist_add(PyObject *o1, PyObject *o2) {
    PyObject *result = NULL;
    PyObject *copy   = NULL;

    copy = pyjlist_new_copy(o1);
    if(copy == NULL) {
        // error indicators already set
        return NULL;
    }
    result = pyjlist_inplace_add(copy, o2);
    // both pyjlist_new_copy() and pyjlist_inplace_add() increfed it
    Py_XDECREF(result);
    return result;
}

/*
 * Method for * operator on pyjlist.  For example, result = o * 5, where o is
 * a pyjlist and result is a new pyjlist.
 */
static PyObject* pyjlist_fill(PyObject *o, Py_ssize_t count) {
    PyObject *result = NULL;
    PyObject *copy   = NULL;

    copy = pyjlist_new_copy(o);
    if(copy == NULL) {
        // error indicators already set
        return NULL;
    }
    result = pyjlist_inplace_fill(copy, count);
    // both pyjlist_new_copy() and pyjlist_inplace_fill() increfed it
    Py_XDECREF(result);
    return result;
}

/*
 * Method for the getting items with the [int] operator on pyjlist.  For
 * example, result = o[i]
 */
static PyObject* pyjlist_getitem(PyObject *o, Py_ssize_t i) {
    jmethodID         get  = NULL;
    jobject           val  = NULL;
    Py_ssize_t        size = 0;
    PyJobject_Object *obj  = (PyJobject_Object*) o;
    JNIEnv           *env  = pyembed_get_env();

    get = (*env)->GetMethodID(env, obj->clazz, "get", "(I)Ljava/lang/Object;");
    if(process_java_exception(env) || !get) {
        return NULL;
    }

    size = PyObject_Size(o);
    if((i > size-1) || (i < 0)) {
        PyErr_Format(PyExc_IndexError, "list index %i out of range, size %i", (int) i, (int) size);
        return NULL;
    }

    val = (*env)->CallObjectMethod(env, obj->object, get, (jint) i);
    if(process_java_exception(env)) {
        return NULL;
    }

    if(val == NULL) {
        Py_RETURN_NONE;
    } else {
        return pyjobject_new(env, val);
    }
}

/*
 * Method for getting slices with the [int:int] operator on pyjlist.  For
 * example, result = o[i1:i2]
 */
static PyObject* pyjlist_getslice(PyObject *o, Py_ssize_t i1, Py_ssize_t i2) {
    jmethodID         sublist = NULL;
    jobject           result  = NULL;
    PyJobject_Object *obj     = (PyJobject_Object*) o;
    JNIEnv           *env     = pyembed_get_env();

    sublist = (*env)->GetMethodID(env, obj->clazz, "subList", "(II)Ljava/util/List;");
    if(process_java_exception(env) || !sublist) {
        return NULL;
    }

    result = (*env)->CallObjectMethod(env, obj->object, sublist, (jint) i1, (jint) i2);
    if(process_java_exception(env)) {
        return NULL;
    }

    return pyjobject_new(env, result);
}

/*
 * Method for the setting items with the [int] operator on pyjlist.  For example,
 * o[i] = v
 */
static int pyjlist_setitem(PyObject *o, Py_ssize_t i, PyObject *v) {
    jmethodID         set      = NULL;
    PyJobject_Object *obj      = (PyJobject_Object*) o;
    JNIEnv           *env      = pyembed_get_env();
    jobject           value    = NULL;

    if (v == NULL) {
        PyErr_Format(PyExc_NotImplementedError,
                     "del from PyJlist not supported yet");
        return -1;
    } else if(v == Py_None) {
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
                        "__setitem__ received an incompatible type: %s",
                        PyString_AsString(pystring));
            Py_XDECREF(pystring);
            return -1;
        }
    }

    set = (*env)->GetMethodID(env, obj->clazz, "set", "(ILjava/lang/Object;)Ljava/lang/Object;");
    if(process_java_exception(env) || !set) {
        return -1;
    }

    (*env)->CallObjectMethod(env, obj->object, set, (jint) i, value);
    if(process_java_exception(env)) {
        return -1;
    }

    // have to return 0 on success even though it's not documented
    return 0;
}

/*
 * Method for setting slices with the [int:int] operator on pyjlist.  For
 * example, o[i1:i2] = v where v is a sequence.
 */
static int pyjlist_setslice(PyObject *o, Py_ssize_t i1, Py_ssize_t i2, PyObject *v) {
   Py_ssize_t oSize;
   Py_ssize_t vSize;
   Py_ssize_t diff;
   Py_ssize_t i, vi;

    if(!PySequence_Check(v)) {
        PyErr_Format(PyExc_TypeError,
                "pyjlist can only slice assign a sequence");
        return -1;
    }

    oSize = PySequence_Size(o);
    vSize = PySequence_Size(v);
    if(i1 < 0) {
        i1 = 0;
    }
    if(i2 > oSize) {
        i2 = oSize;
    }
    if(i1 >= i2) {
        PyErr_Format(PyExc_IndexError, "invalid slice indices: %i:%i",
                (int) i1, (int) i2);
        return -1;
    }
    diff = i2 - i1;
    if(diff != vSize) {
        /*
         * TODO: Python lists support slice assignment of a different length,
         * but that gets complicated, so not planning on supporting it until
         * requested.  For inspiration look at python's listobject.c's
         * list_ass_slice().
         */
        PyErr_Format(PyExc_IndexError,
                "pyjlist only supports assigning a sequence of the same size as the slice, slice = [%i:%i], value size=%i",
                (int) i1, (int) i2, (int) vSize);
        return -1;
    }

    vi = 0;
    for(i = i1; i < i2; i++) {
        PyObject *vVal = PySequence_GetItem(v, vi);
        if(pyjlist_setitem(o, i, vVal) == -1) {
            /*
             * TODO This is not transactional if it fails partially through.
             * Not sure how to make that safe short of making a copy of o
             * and then replacing o's underlying jobject on success.  That
             * would slow it down though....
             */
            return -1;
        }
        vi++;
    }

    return 0;
}


/*
 * Method for the += operator on pyjlist.  For example, o1 += o2, where
 * o1 is a pyjlist.
 */
static PyObject* pyjlist_inplace_add(PyObject *o1, PyObject *o2) {
    jobject               value   = NULL;
    jclass           collection   = NULL;
    JNIEnv               *env     = pyembed_get_env();
    PyJobject_Object     *self    = (PyJobject_Object*) o1;

    if(pyjlist_check(o2)) {
        value                     = ((PyJobject_Object*) o2)->object;
    } else {
        value                     = pyembed_box_py(env, o2);
    }

    collection = (*env)->FindClass(env, "java/util/Collection");
    if(process_java_exception(env) || !collection) {
        return NULL;
    }

    if((*env)->IsInstanceOf(env, value, collection)) {
        /*
         * it's a Collection so we need to simulate a python + and combine the
         * two collections
         */
        jmethodID addAll = (*env)->GetMethodID(env, self->clazz, "addAll", "(Ljava/util/Collection;)Z");
        if(process_java_exception(env) || !addAll) {
            return NULL;
        }

        (*env)->CallBooleanMethod(env, self->object, addAll, value);
        if(process_java_exception(env)) {
            return NULL;
        }
    } else {
        // not a collection, add it as a single object
        jmethodID add = (*env)->GetMethodID(env, self->clazz, "add", "(Ljava/lang/Object;)Z");
        if(process_java_exception(env) || !add) {
            return NULL;
        }

        (*env)->CallBooleanMethod(env, self->object, add, value);
        if(process_java_exception(env)) {
            return NULL;
        }
    }

    Py_INCREF(o1);

    return o1;
}

/*
 * Method for *= operator on pyjlist.  For example, o *= 5, where o is
 * a pyjlist.
 */
static PyObject* pyjlist_inplace_fill(PyObject *o, Py_ssize_t count) {
    PyJobject_Object     *self    = (PyJobject_Object*) o;
    JNIEnv               *env     = pyembed_get_env();

    if(count < 1) {
        jmethodID         clear   = (*env)->GetMethodID(env, self->clazz, "clear", "()V");
        if(process_java_exception(env) || !clear) {
            return NULL;
        }

        (*env)->CallVoidMethod(env, self->object, clear);
        if(process_java_exception(env)) {
            return NULL;
        }
    } else if (count > 1) {
        int               i     = 0;
        PyObject         *copy  = pyjlist_new_copy(o);
        if(copy == NULL) {
            // error indicators already set
            return NULL;
        }

        // TODO there's probably a better way to do this
        for(i = 1; i < count; i++) {
            PyObject    *result = pyjlist_inplace_add(o, copy);
            if(!result) {
                // error indicators already set
                return NULL;
            } else {
                // result and o are the same object, pyjlist_inplace_add increfed it
                Py_DECREF(result);
            }
        }
        Py_DECREF(copy);
    }

    Py_INCREF(o);
    return o;
}

static PyObject* pyjlist_subscript(PyObject *self, PyObject *item) {
    if(PyInt_Check(item)) {
        long i = PyInt_AS_LONG(item);
        if (i < 0)
            i += (long) PyObject_Size(self);
        return pyjlist_getitem(self, (Py_ssize_t) i);
    }
    else if(PyLong_Check(item)) {
        long i = PyLong_AsLong(item);
        if (i == -1 && PyErr_Occurred())
            return NULL;
        if (i < 0)
            i += (long) PyObject_Size(self);
        return pyjlist_getitem(self, (Py_ssize_t) i);
    } else if(PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slicelength;
        /*
         * ignore compile warning on the next line, they fixed the
         * method signature in python 3.2
         */
        if(PySlice_GetIndicesEx(item, PyObject_Size(self), &start, &stop, &step, &slicelength) < 0) {
            // error will already be set
            return NULL;
        }

        if(slicelength <= 0) {
            return pyjlist_getslice(self, 0, 0);
        } else if(step != 1) {
            PyErr_SetString(PyExc_TypeError, "pyjlist slices must have step of 1");
            return NULL;
        } else {
            return pyjlist_getslice(self, start, stop);
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "list indices must be integers, longs, or slices");
        return NULL;
    }
}

static int pyjlist_set_subscript(PyObject* self, PyObject* item, PyObject* value) {
    if(PyInt_Check(item)) {
        long i = PyInt_AS_LONG(item);
        if (i < 0)
            i += (long) PyObject_Size(self);
        return pyjlist_setitem(self, (Py_ssize_t) i, value);
    }
    else if(PyLong_Check(item)) {
        long i = PyLong_AsLong(item);
        if (i == -1 && PyErr_Occurred())
            return -1;
        if (i < 0)
            i += (long) PyObject_Size(self);
        return pyjlist_setitem(self, (Py_ssize_t) i, value);
    } else if(PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slicelength;
        /*
         * ignore compile warning on the next line, they fixed the
         * method signature in python 3.2
         */
        if(PySlice_GetIndicesEx(item, PyObject_Size(self), &start, &stop, &step, &slicelength) < 0) {
            // error will already be set
            return -1;
        }

        if(slicelength <= 0) {
            return 0;
        } else if(step != 1) {
            PyErr_SetString(PyExc_TypeError, "pyjlist slices must have step of 1");
            return -1;
        } else {
            return pyjlist_setslice(self, start, stop, value);
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "list indices must be integers, longs, or slices");
        return -1;
    }

}

static PyMethodDef pyjlist_methods[] = {
    {NULL, NULL, 0, NULL}
};

static PySequenceMethods pyjlist_seq_methods = {
        0, // inherited       /* sq_length */
        pyjlist_add,          /* sq_concat */
        pyjlist_fill,         /* sq_repeat */
        pyjlist_getitem,      /* sq_item */
        pyjlist_getslice,     /* sq_slice */
        pyjlist_setitem,      /* sq_ass_item */
        pyjlist_setslice,     /* sq_ass_slice */
        0, // inherited       /* sq_contains */
        pyjlist_inplace_add,  /* sq_inplace_concat */
        pyjlist_inplace_fill, /* sq_inplace_repeat */
};

static PyMappingMethods pyjlist_map_methods = {
    0,                                        /* mp_length */
    (binaryfunc) pyjlist_subscript,           /* mp_subscript */
    (objobjargproc) pyjlist_set_subscript,    /* mp_ass_subscript */
};


/*
 * Inherits from PyJcollection_Type
 */
PyTypeObject PyJlist_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJlist",
    sizeof(PyJlist_Object),
    0,
    0,                                        /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    &pyjlist_seq_methods,                     /* tp_as_sequence */
    &pyjlist_map_methods,                     /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    "jlist",                                  /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0, // inherited                           /* tp_iter */
    0,                                        /* tp_iternext */
    pyjlist_methods,                          /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0, // &PyJcollection_Type                 /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
