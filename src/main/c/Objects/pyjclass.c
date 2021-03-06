/*
   jep - Java Embedded Python

   Copyright (c) 2004-2019 JEP AUTHORS.

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
#include "abstract.h"
#include "object.h"
#include "pyport.h"
#include "tupleobject.h"


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
            printf("Error adding inner class %s\n", charName);
        } else {
            PyObject *pyname = PyUnicode_FromString(charName);
            Py_DECREF(pyname);
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


static int pyjclass_init(JNIEnv *env, PyObject *pyjob)
{
    ((PyJClassObject*) pyjob)->constructor = NULL;

    /*
     * attempt to add public inner classes as attributes since lots of people
     * code with public enum.  Note this will not allow the inner class to be
     * imported separately, it must be accessed through the enclosing class.
     */
    if (!pyjclass_add_inner_classes(env, (PyJClassObject*) pyjob)) {
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
    PyObject* pyjob = PyJObject_New(env, PyJClass_Type, NULL, obj);
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
    #ifndef Py_LIMITED_API
        assert(PyJClass_Type->tp_base == PyJObject_Type);
    #endif
    destructor dealloc = PyType_GetSlot(PyJObject_Type, Py_tp_dealloc);
    if (dealloc != NULL) dealloc((PyObject*) self);
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
    Py_ssize_t originalSize = PyTuple_Size(args);
    assert(originalArgs >= 0);
    PyObject *actualArgs = PyTuple_New(1 + originalSize);
    if (actualArgs == NULL) return NULL; // OutOfMemoryError
    for (Py_ssize_t i = 0; i < originalSize; i++) {
        PyObject *arg = PyTuple_GetItem(args, i);
        assert(arg != NULL);
        Py_INCREF(arg);
        if (!PyTuple_SetItem(actualArgs, i + 1, arg)) Py_UNREACHABLE();
    }
    result = PyObject_Call(self->constructor, actualArgs, keywords);
    Py_DECREF(actualArgs);
    return result;
}

static PyTypeObject* pyjclass_GetPyType(PyJClassObject* self)
{
    JNIEnv* env = pyembed_get_env();
    return PyJType_Get(env, self->clazz);
}


static PyGetSetDef pyjclass_getset[] = {
    {"__pytype__", (getter) pyjclass_GetPyType, NULL},
    {NULL} /* Sentinel */
};

PyTypeObject *PyJClass_Type = NULL;
int jep_jclass_type_ready() {
    static PyType_Slot SLOTS[] = {
        {Py_tp_dealloc, (void*) pyjclass_dealloc},
        {Py_tp_call, (void*) pyjclass_call},
        {Py_tp_doc, "jclass"},
        {Py_tp_getset, pyjclass_getset},
        {0, NULL}
    };
    PyType_Spec spec = {
        .name = "jep.PyJClass",
        .basicsize = sizeof(PyJClassObject),
        .flags = Py_TPFLAGS_DEFAULT,
        .slots = SLOTS
    };
    PyJClass_Type = (PyTypeObject*) PyType_FromSpecWithBases(&spec, (PyObject*) PyJObject_Type);
    return PyType_Ready(PyJClass_Type);
}