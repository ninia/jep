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

static PyObject* pyjlist_add(PyObject*, PyObject*);
static PyObject* pyjlist_fill(PyObject*, Py_ssize_t);
static PyObject* pyjlist_getitem(PyObject*, Py_ssize_t);
static PyObject* pyjlist_getslice(PyObject*, Py_ssize_t, Py_ssize_t);
static int pyjlist_setitem(PyObject*, Py_ssize_t, PyObject*);
static int pyjlist_setslice(PyObject*, Py_ssize_t, Py_ssize_t, PyObject*);
static PyObject* pyjlist_inplace_add(PyObject*, PyObject*);
static PyObject* pyjlist_inplace_fill(PyObject*, Py_ssize_t);

/*
 * Convenience method to copy a list's items into a new java.util.List of the
 * same type.
 */
static PyObject* pyjlist_new_copy(PyObject *toCopy)
{
    jobject       newList     = NULL;
    PyJObject    *obj         = (PyJObject*) toCopy;
    JNIEnv       *env         = pyembed_get_env();
    PyObject     *result      = NULL;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return NULL;
    }

    newList = java_lang_Class_newInstance(env, obj->clazz);
    if (process_java_exception(env) || !newList) {
        goto FINALLY;
    }

    java_util_List_addAll(env, newList, obj->object);
    if (process_java_exception(env)) {
        goto FINALLY;
    }

    result = jobject_As_PyJObject(env, newList, obj->clazz);
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}

/*
 * Method for the + operator on pyjlist.  For example, result = o1 + o2, where
 * o1 is a pyjlist and result is a new pyjlist.
 */
static PyObject* pyjlist_add(PyObject *o1, PyObject *o2)
{
    PyObject *result = NULL;
    PyObject *copy   = NULL;

    copy = pyjlist_new_copy(o1);
    if (copy == NULL) {
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
static PyObject* pyjlist_fill(PyObject *o, Py_ssize_t count)
{
    PyObject *result = NULL;
    PyObject *copy   = NULL;

    copy = pyjlist_new_copy(o);
    if (copy == NULL) {
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
static PyObject* pyjlist_getitem(PyObject *o, Py_ssize_t i)
{
    jobject       val  = NULL;
    Py_ssize_t    size = 0;
    PyJObject    *obj  = (PyJObject*) o;
    JNIEnv       *env  = pyembed_get_env();

    size = PyObject_Size(o);
    if ((i > size - 1) || (i < 0)) {
        PyErr_Format(PyExc_IndexError, "list index %i out of range, size %i", (int) i,
                     (int) size);
        return NULL;
    }

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return NULL;
    }

    val = java_util_List_get(env, obj->object, (jint) i);
    if (process_java_exception(env)) {
        (*env)->PopLocalFrame(env, NULL);
        return NULL;
    }

    if (val == NULL) {
        (*env)->PopLocalFrame(env, NULL);
        Py_RETURN_NONE;
    } else {
        PyObject *result = jobject_As_PyObject(env, val);
        (*env)->PopLocalFrame(env, NULL);
        return result;
    }
}

/*
 * Method for getting slices with the [int:int] operator on pyjlist.  For
 * example, result = o[i1:i2]
 */
static PyObject* pyjlist_getslice(PyObject *o, Py_ssize_t i1, Py_ssize_t i2)
{
    jobject       result  = NULL;
    PyJObject    *obj     = (PyJObject*) o;
    JNIEnv       *env     = pyembed_get_env();
    PyObject     *pyres   = NULL;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return NULL;
    }

    result = java_util_List_subList(env, obj->object, (jint) i1, (jint) i2);
    if (process_java_exception(env)) {
        goto FINALLY;
    }

    pyres = jobject_As_PyObject(env, result);
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return pyres;
}

/*
 * Method for the setting items with the [int] operator on pyjlist.  For example,
 * o[i] = v.  Also supports del o[i]
 */
static int pyjlist_setitem(PyObject *o, Py_ssize_t i, PyObject *v)
{
    PyJObject    *obj      = (PyJObject*) o;
    JNIEnv       *env      = pyembed_get_env();
    jobject       value    = NULL;
    int           result   = -1;

    if (v == NULL) {
        // this is a del PyJList[index] statement

        java_util_List_remove(env, obj->object, (jint) i);
        if (process_java_exception(env)) {
            return -1;
        }

        // have to return 0 on success even though it's not documented
        return 0;
    }

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return -1;
    }

    value = PyObject_As_jobject(env, v, JOBJECT_TYPE);
    if (!value && PyErr_Occurred()) {
        goto FINALLY;
    }


    java_util_List_set(env, obj->object, (jint) i, value);
    if (process_java_exception(env)) {
        goto FINALLY;
    }
    // have to return 0 on success even though it's not documented
    result = 0;
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}

/*
 * Method for setting slices with the [int:int] operator on pyjlist.  For
 * example, o[i1:i2] = v where v is a sequence.
 */
static int pyjlist_setslice(PyObject *o, Py_ssize_t i1, Py_ssize_t i2,
                            PyObject *v)
{
    Py_ssize_t oSize;
    Py_ssize_t vSize;
    Py_ssize_t diff;
    Py_ssize_t i, vi;

    if (!PySequence_Check(v)) {
        PyErr_Format(PyExc_TypeError,
                     "PyJList can only slice assign a sequence");
        return -1;
    }

    oSize = PySequence_Size(o);
    vSize = PySequence_Size(v);
    if (i1 < 0) {
        i1 = 0;
    }
    if (i2 > oSize) {
        i2 = oSize;
    }
    if (i1 >= i2) {
        PyErr_Format(PyExc_IndexError, "invalid slice indices: %i:%i",
                     (int) i1, (int) i2);
        return -1;
    }
    diff = i2 - i1;
    if (diff != vSize) {
        /*
         * TODO: Python lists support slice assignment of a different length,
         * but that gets complicated, so not planning on supporting it until
         * requested.  For inspiration look at python's listobject.c's
         * list_ass_slice().
         */
        PyErr_Format(PyExc_IndexError,
                     "PyJList only supports assigning a sequence of the same size as the slice, slice = [%i:%i], value size=%i",
                     (int) i1, (int) i2, (int) vSize);
        return -1;
    }

    vi = 0;
    for (i = i1; i < i2; i++) {
        PyObject *vVal = PySequence_GetItem(v, vi);
        if (pyjlist_setitem(o, i, vVal) == -1) {
            /*
             * TODO This is not transactional if it fails partially through.
             * Not sure how to make that safe short of making a copy of o
             * and then replacing o's underlying jobject on success.  That
             * would slow it down though....
             */
            Py_DECREF(vVal);
            return -1;
        }
        Py_DECREF(vVal);
        vi++;
    }

    return 0;
}


/*
 * Method for the += operator on pyjlist.  For example, o1 += o2, where
 * o1 is a pyjlist.
 */
static PyObject* pyjlist_inplace_add(PyObject *o1, PyObject *o2)
{
    jobject        value    = NULL;
    JNIEnv        *env      = pyembed_get_env();
    PyJObject     *self     = (PyJObject*) o1;
    PyObject      *result   = NULL;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return NULL;
    }

    /*
     * TODO: To match Python behavior of += operator, we should really be
     * using JITERABLE_TYPE and ensuring its an instance of Iterable, not
     * Collection.
     */
    value = PyObject_As_jobject(env, o2, JCOLLECTION_TYPE);
    if (!value && PyErr_Occurred()) {
        return NULL;
    }

    if (!value) {
        PyErr_Format(PyExc_TypeError,
                     "Expected java.util.Collection but received null.");
        return NULL;
    }

    /*
     * it's a Collection so we need to simulate a python + and combine the
     * two collections
     */
    java_util_List_addAll(env, self->object, value);
    if (process_java_exception(env)) {
        goto FINALLY;
    }

    result = o1;
    Py_INCREF(o1);
FINALLY:
    (*env)->PopLocalFrame(env, NULL);
    return result;
}

/*
 * Method for *= operator on pyjlist.  For example, o *= 5, where o is
 * a pyjlist.
 */
static PyObject* pyjlist_inplace_fill(PyObject *o, Py_ssize_t count)
{
    PyJObject      *self    = (PyJObject*) o;
    JNIEnv         *env     = pyembed_get_env();

    if (count < 1) {

        java_util_List_clear(env, self->object);
        if (process_java_exception(env)) {
            return NULL;
        }
    } else if (count > 1) {
        int               i     = 0;
        PyObject         *copy  = pyjlist_new_copy(o);
        if (copy == NULL) {
            // error indicators already set
            return NULL;
        }

        // TODO there's probably a better way to do this
        for (i = 1; i < count; i++) {
            PyObject    *result = pyjlist_inplace_add(o, copy);
            if (!result) {
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

static PyObject* pyjlist_subscript(PyObject *self, PyObject *item)
{
    if (PyLong_Check(item)) {
        long i = PyLong_AsLong(item);
        if (i == -1 && PyErr_Occurred()) {
            return NULL;
        }
        if (i < 0) {
            i += (long) PyObject_Size(self);
        }
        return pyjlist_getitem(self, (Py_ssize_t) i);
    } else if (PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slicelength;
        if (PySlice_GetIndicesEx(item, PyObject_Size(self), &start, &stop, &step,
                                 &slicelength) < 0) {
            // error will already be set
            return NULL;
        }

        if (slicelength <= 0) {
            return pyjlist_getslice(self, 0, 0);
        } else if (step != 1) {
            PyErr_SetString(PyExc_TypeError, "pyjlist slices must have step of 1");
            return NULL;
        } else {
            return pyjlist_getslice(self, start, stop);
        }
    } else {
        PyErr_SetString(PyExc_TypeError,
                        "list indices must be integers, longs, or slices");
        return NULL;
    }
}

static int pyjlist_set_subscript(PyObject* self, PyObject* item,
                                 PyObject* value)
{
    if (PyLong_Check(item)) {
        long i = PyLong_AsLong(item);
        if (i == -1 && PyErr_Occurred()) {
            return -1;
        }
        if (i < 0) {
            i += (long) PyObject_Size(self);
        }
        return pyjlist_setitem(self, (Py_ssize_t) i, value);
    } else if (PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slicelength;
        if (PySlice_GetIndicesEx(item, PyObject_Size(self), &start, &stop, &step,
                                 &slicelength) < 0) {
            // error will already be set
            return -1;
        }

        if (slicelength <= 0) {
            return 0;
        } else if (step != 1) {
            PyErr_SetString(PyExc_TypeError, "pyjlist slices must have step of 1");
            return -1;
        } else {
            return pyjlist_setslice(self, start, stop, value);
        }
    } else {
        PyErr_SetString(PyExc_TypeError,
                        "list indices must be integers, longs, or slices");
        return -1;
    }

}

static PyType_Slot slots[] = {
    // NOTE: Inherited `tp_iter` from PyJIterable
    {Py_tp_doc, "Jep java.util.List"},
    /*
     * **** sequence slots ****
     * NOTE: Inherited `sq_length` and `sq_contains` from PyJCollection
     */
    {Py_sq_concat, (void*) pyjlist_add},
    {Py_sq_repeat, (void*) pyjlist_fill},
    {Py_sq_item, (void*) pyjlist_getitem},
    {Py_sq_ass_item, (void*) pyjlist_setitem},
    {Py_sq_inplace_concat, (void*) pyjlist_inplace_add},
    {Py_sq_inplace_repeat, (void*) pyjlist_inplace_fill},
    // mapping methods
    {Py_mp_subscript, (void*) pyjlist_subscript},
    {Py_mp_ass_subscript, (void*) pyjlist_set_subscript},
    {0, NULL},
};
PyType_Spec PyJList_Spec = {
    .name = "java.util.List",
    .basicsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .slots = slots,
};
