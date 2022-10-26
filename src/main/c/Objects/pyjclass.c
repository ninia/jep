/*
   jep - Java Embedded Python

   Copyright (c) 2004-2022 JEP AUTHORS.

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
 * https://bugs.python.org/issue2897
 * structmember.h must be included to use PyMemberDef
 */
#include "structmember.h"

/*
 * Adds a single inner class as attributes to the pyjclass. This will check if
 * the inner class is public and will only add it if it is public. This is
 * intended to only be called from pyjclass_add_inner_classes().
 *
 * @param env the JNI environment
 * @param topClz the pyjobject of the top/outer Class
 * @param innerClz the jclass of the inner class
 *
 * @return NULL on errors, topClz if there are no errors
 */
static PyObject* pyjclass_add_inner_class(JNIEnv *env, PyJClassObject *topClz,
        jclass innerClz)
{
    jint      mods;
    jboolean  public;

    // setup to verify this inner class should be available
    mods = java_lang_Class_getModifiers(env, innerClz);
    if (process_java_exception(env)) {
        return NULL;
    }
    public = java_lang_reflect_Modifier_isPublic(env, mods);
    if (process_java_exception(env)) {
        return NULL;
    }

    if (public) {
        PyObject        *attrClz    = NULL;
        jstring          shortName  = NULL;
        const char      *charName   = NULL;

        attrClz = PyJClass_Wrap(env, innerClz);
        if (!attrClz) {
            return NULL;
        }
        shortName = java_lang_Class_getSimpleName(env, innerClz);
        if (process_java_exception(env) || !shortName) {
            return NULL;
        }
        charName = jstring2char(env, shortName);

        if (PyDict_SetItemString(topClz->attr, charName, attrClz) != 0) {
            return NULL;
        }
        Py_DECREF(attrClz); // parent class will hold the reference
        release_utf_char(env, shortName, charName);
    }
    return (PyObject*) topClz;
}


/*
 * Adds a Java class's public inner classes as attributes to the pyjclass.
 *
 * @param env the JNI environment
 * @param topClz the pyjobject of the top/outer Class
 *
 * @return topClz if successful, otherwise NULL
 */
static PyObject* pyjclass_add_inner_classes(JNIEnv *env,
        PyJClassObject *topClz)
{
    jobjectArray      innerArray    = NULL;
    jsize             innerSize     = 0;

    innerArray = java_lang_Class_getDeclaredClasses(env, topClz->clazz);
    if (process_java_exception(env) || !innerArray) {
        return NULL;
    }

    innerSize = (*env)->GetArrayLength(env, innerArray);
    if (innerSize > 0) {
        int i;

        if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
            (*env)->DeleteLocalRef(env, innerArray);
            process_java_exception(env);
            return NULL;
        }

        // check each inner class to see if it's public
        for (i = 0; i < innerSize; i++) {
            jclass innerClz = (*env)->GetObjectArrayElement(env, innerArray, i);
            if (process_java_exception(env) || !innerClz) {
                (*env)->PopLocalFrame(env, NULL);
                (*env)->DeleteLocalRef(env, innerArray);
                return NULL;
            }
            if (pyjclass_add_inner_class(env, topClz, innerClz) == NULL) {
                (*env)->PopLocalFrame(env, NULL);
                (*env)->DeleteLocalRef(env, innerArray);
                return NULL;
            }
            (*env)->DeleteLocalRef(env, innerClz);
        }
        (*env)->PopLocalFrame(env, NULL);
    }
    (*env)->DeleteLocalRef(env, innerArray);

    return (PyObject*) topClz;
}

/*
 * Populate an attrs dict with all the Java methods and fields for the given
 * class. This is currently the only way static methods and fields are made
 * available. There is no known use for non-static fields but for now they are
 * left in for backwards compatiblity, consider removing them in the future.
 *
 * Since the method and fields for a class are added to the type and the type
 * is cached, this can just copy the attributes from the type and avoid any
 * reflection.
 */
static PyObject* pyjclass_init_attr(JNIEnv *env, jclass clazz)
{
    PyObject *attr = PyDict_New();
    if (!attr) {
        return attr;
    }

    if (!(*env)->IsAssignableFrom(env, clazz, JOBJECT_TYPE)) {
        /*
         * Don't bother with primitive types, they don't work with PyJType and they
         * don't have any fields or methods.
         */
        return attr;
    }
    PyTypeObject* type = PyJType_Get(env, clazz);
    if (!type) {
        Py_DecRef(attr);
        return NULL;
    }
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(type->tp_dict, &pos, &key, &value)) {
        if (PyJMethod_Check(value) || PyJMultiMethod_Check(value)
                || PyJField_Check(value) ) {
            if (PyDict_SetItem(attr, key, value) != 0) {
                Py_DecRef((PyObject*) type);
                Py_DecRef(attr);
                return NULL;
            }
        }
    }
    Py_DecRef((PyObject*) type);
    return attr;
}

static int pyjclass_init(JNIEnv *env, PyObject *pyobj)
{
    PyJClassObject *pyjclass = (PyJClassObject*) pyobj;

    pyjclass->constructor = NULL;
    pyjclass->attr = pyjclass_init_attr(env, ((PyJObject*) pyobj)->clazz);
    if (pyjclass->attr == NULL) {
        return 0;
    }

    /*
     * attempt to add public inner classes as attributes since lots of people
     * code with public enum.  Note this will not allow the inner class to be
     * imported separately, it must be accessed through the enclosing class.
     */
    if (!pyjclass_add_inner_classes(env, pyjclass)) {
        /*
         * let's just print the error to stderr and continue on without
         * inner class support, it's not the end of the world
         */
        if (PyErr_Occurred()) {
            PyErr_PrintEx(0);
        }
    }

    return 1;
}

PyObject* PyJClass_Wrap(JNIEnv *env, jobject obj)
{
    PyObject* pyjob = PyJObject_New(env, &PyJClass_Type, NULL, obj);
    if (pyjob) {
        if (!pyjclass_init(env, pyjob)) {
            Py_DecRef(pyjob);
            pyjob = NULL;
        }
    }
    return pyjob;
}

static void pyjclass_dealloc(PyJClassObject *self)
{
#if USE_DEALLOC
    Py_CLEAR(self->constructor);
    Py_CLEAR(self->attr);
    PyJClass_Type.tp_base->tp_dealloc((PyObject*) self);
#endif
}

/*
 * Initialize the constructors field of a pyjclass.
 *
 * @return 1 on successful initialization, -1 on error.
 */
static int pyjclass_init_constructors(PyJClassObject *pyc)
{
    JNIEnv       *env         = NULL;
    jobjectArray  initArray   = NULL;
    int           initLen     = 0;
    PyObject     *pycallable  = NULL;
    int           i           = 0;

    env = pyembed_get_env();
    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return -1;
    }

    initArray = java_lang_Class_getConstructors(env, pyc->clazz);
    if (process_java_exception(env) || !initArray) {
        goto EXIT_ERROR;
    }

    initLen   = (*env)->GetArrayLength(env, initArray);

    for (i = 0; i < initLen; i++) {
        PyObject *pyjinit     = NULL;
        jobject   constructor = NULL;

        constructor = (*env)->GetObjectArrayElement(env, initArray, i);
        if (process_java_exception(env) || !constructor) {
            goto EXIT_ERROR;
        }
        pyjinit = PyJConstructor_New(env, constructor);
        if (pyjinit == NULL) {
            goto EXIT_ERROR;
        }
        (*env)->DeleteLocalRef(env, constructor);

        if (i == 0) {
            pycallable = pyjinit;
        } else if (i == 1) {
            PyObject* firstInit = pycallable;
            pycallable = PyJMultiMethod_New(firstInit, pyjinit);
            Py_DECREF(firstInit);
            Py_DECREF(pyjinit);
            if (pycallable == NULL) {
                goto EXIT_ERROR;
            }
        } else {
            if (PyJMultiMethod_Append(pycallable, pyjinit) == -1) {
                Py_DECREF(pyjinit);
                goto EXIT_ERROR;
            }
            Py_DECREF(pyjinit);
        }
    }
    (*env)->DeleteLocalRef(env, initArray);

    if (pycallable) {
        pyc->constructor = pycallable;
    }

    (*env)->PopLocalFrame(env, NULL);
    return 1;

EXIT_ERROR:
    (*env)->PopLocalFrame(env, NULL);
    Py_CLEAR(pycallable);
    return -1;
}

// call constructor as a method and return pyjobject.
static PyObject* pyjclass_call(PyJClassObject *self,
                               PyObject *args,
                               PyObject *keywords)
{
    PyObject *boundConstructor = NULL;
    PyObject *result           = NULL;
    if (self->constructor == NULL) {
        if (pyjclass_init_constructors(self) == -1) {
            return NULL;
        }
        if (self->constructor == NULL) {
            PyErr_Format(PyExc_TypeError, "No public constructor");
            return NULL;
        }
    }
    /*
     * Bind the constructor to the class so that the class will
     * be the first arg when constructor is called.
     */
    boundConstructor = PyMethod_New(self->constructor, (PyObject*) self);
    result = PyObject_Call(boundConstructor, args, keywords);
    Py_DECREF(boundConstructor);
    return result;
}

// get attribute 'name' for object.
// uses obj->attr dict for storage.
// returns new reference.
static PyObject* pyjclass_getattro(PyObject *obj, PyObject *name)
{
    PyObject *ret = PyObject_GenericGetAttr(obj, name);
    if (ret == NULL) {
        return NULL;
    } else if (PyJMethod_Check(ret) || PyJMultiMethod_Check(ret)) {
        /*
         * TODO Should not bind non-static methods to pyjclass objects, but not
         * sure yet how to handle multimethods and static methods.
         */
        PyObject* wrapper = PyMethod_New(ret, (PyObject*) obj);
        Py_DECREF(ret);
        return wrapper;
    } else if (PyJField_Check(ret)) {
        PyObject *resolved = pyjfield_get((PyJFieldObject *) ret, (PyJObject*) obj);
        Py_DECREF(ret);
        return resolved;
    }
    return ret;
}


// set attribute v for object.
// uses obj->attr dictionary for storage.
static int pyjclass_setattro(PyObject *obj, PyObject *name, PyObject *v)
{
    PyObject *cur = NULL;
    if (v == NULL) {
        PyErr_Format(PyExc_TypeError,
                     "Deleting attributes from PyJObjects is not allowed.");
        return -1;
    }
    PyJObject *pyjobj = (PyJObject*) obj;
    PyJClassObject *pyjclass = (PyJClassObject*) obj;
    cur = PyDict_GetItem(pyjclass->attr, name);
    if (PyErr_Occurred()) {
        return -1;
    }

    if (cur == NULL) {
        PyErr_Format(PyExc_AttributeError, "'%s' object has no attribute '%s'.",
                     PyUnicode_AsUTF8(pyjobj->javaClassName), PyUnicode_AsUTF8(name));
        return -1;
    }

    if (!PyJField_Check(cur)) {
        if (PyJMethod_Check(cur) || PyJMultiMethod_Check(cur)) {
            PyErr_Format(PyExc_AttributeError, "'%s' object cannot assign to method '%s'.",
                         PyUnicode_AsUTF8(pyjobj->javaClassName), PyUnicode_AsUTF8(name));
        } else {
            PyErr_Format(PyExc_AttributeError,
                         "'%s' object cannot assign to attribute '%s'.",
                         PyUnicode_AsUTF8(pyjobj->javaClassName), PyUnicode_AsUTF8(name));
        }
        return -1;
    }

    return pyjfield_set((PyJFieldObject *) cur, pyjobj, v);
}


static PyTypeObject* pyjclass_GetPyType(PyJClassObject* self)
{
    JNIEnv* env = pyembed_get_env();
    return PyJType_Get(env, self->clazz);
}

static PyMemberDef pyjclass_members[] = {
    {"__dict__", T_OBJECT, offsetof(PyJClassObject, attr), READONLY},
    {0}
};

static PyGetSetDef pyjclass_getset[] = {
    {"__pytype__", (getter) pyjclass_GetPyType, NULL},
    {NULL} /* Sentinel */
};

PyTypeObject PyJClass_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJClass",
    sizeof(PyJClassObject),
    0,
    (destructor) pyjclass_dealloc,            /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    (ternaryfunc) pyjclass_call,              /* tp_call */
    0,                                        /* tp_str */
    pyjclass_getattro,                        /* tp_getattro */
    pyjclass_setattro,                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    "jclass",                                 /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    0,                                        /* tp_methods */
    pyjclass_members,                         /* tp_members */
    pyjclass_getset,                          /* tp_getset */
    0, // &PyJObject_Type                     /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    offsetof(PyJClassObject, attr),           /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
