/*
   jep - Java Embedded Python

   Copyright (c) 2017 JEP AUTHORS.

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


static int subtypes_initialized = 0;

/*
 * MSVC requires tp_base to be set at runtime instead of in the type
 * declaration. :/  Otherwise we could just set tp_base in the type declaration
 * and be done with it.  Since we are building an inheritance tree of types, we
 * need to ensure that all the tp_base are set for the subtypes before we
 * possibly use those subtypes.
 *
 * Furthermore, we need to ensure that the inheritance tree is built in the
 * correct order, i.e. from the top down.  For example, we need to set that
 * PyJIterable's tp_base extends PyJObject before we set that PyJCollection's
 * tp_base extends PyJIterable.
 *
 * See https://docs.python.org/2/extending/newtypes.html
 *     https://docs.python.org/3/extending/newtypes.html
 */
static void pyjobject_init_subtypes(void)
{
    // start at the top with object
    if (PyType_Ready(&PyJObject_Type) < 0) {
        return;
    }

    // next do number
    if (!PyJNumber_Type.tp_base) {
        PyJNumber_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJNumber_Type) < 0) {
        return;
    }

    // next do iterable
    if (!PyJIterable_Type.tp_base) {
        PyJIterable_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJIterable_Type) < 0) {
        return;
    }

    // next do iterator
    if (!PyJIterator_Type.tp_base) {
        PyJIterator_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJIterator_Type) < 0) {
        return;
    }

    // next do collection
    if (!PyJCollection_Type.tp_base) {
        PyJCollection_Type.tp_base = &PyJIterable_Type;
    }
    if (PyType_Ready(&PyJCollection_Type) < 0) {
        return;
    }

    // next do list
    if (!PyJList_Type.tp_base) {
        PyJList_Type.tp_base = &PyJCollection_Type;
    }
    if (PyType_Ready(&PyJList_Type) < 0) {
        return;
    }

    // next do map
    if (!PyJMap_Type.tp_base) {
        PyJMap_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJMap_Type) < 0) {
        return;
    }

    // last do autocloseable
    if (!PyJAutoCloseable_Type.tp_base) {
        PyJAutoCloseable_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJAutoCloseable_Type) < 0) {
        return;
    }

    subtypes_initialized = 1;
}

/* Set the object attributes from the cache */
static int pyjobject_init(JNIEnv *env, PyJObject *pyjob)
{

    jstring className     = NULL;
    PyObject *pyClassName = NULL;
    JepThread *jepThread  = NULL;
    PyObject *cachedAttrs = NULL;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return 0;
    }

    /*
     * Attach the attribute java_name to the PyJObject instance to assist
     * developers with understanding the type at runtime.
     */
    className = java_lang_Class_getName(env, pyjob->clazz);
    if (process_java_exception(env) || !className) {
        goto EXIT_ERROR;
    }
    pyClassName = jstring_To_PyObject(env, className);
    pyjob->javaClassName = pyClassName;

    /*
     * Get methods for the PyJObject, optimized for performance.  The code
     * below is very similar to previous versions except methods are now
     * cached in memory.
     *
     * Previously every time you instantiate a PyJObject, Jep would get the
     * complete list of methods through reflection, turn them into PyJMethods,
     * and add them as attributes to the PyJObject.
     *
     * Now Jep retains a Python dictionary in memory with a key of the fully
     * qualified Java classname to a list of PyJMethods and PyJMultiMethods.
     * Since the Java methods will never change at runtime for a particular
     * Class, this is safe and drastically speeds up PyJObject instantiation
     * by reducing reflection calls. We continue to set and reuse the
     * PyJMethods and PyJMultiMethods attributes on the PyJObject instance,
     * but if pyjobject_getattro sees a PyJMethod or PyJMultiMethod, it will
     * put it inside a PyMethod and return that, enabling the reuse of the
     * PyJMethod or PyJMultiMethod for this particular object instance.
     *
     * We have the GIL at this point, so we can safely assume we're
     * synchronized and multiple threads will not alter the dictionary at the
     * same time.
     */
    jepThread = pyembed_get_jepthread();
    if (jepThread == NULL) {
        goto EXIT_ERROR;
    }
    if (jepThread->fqnToPyJAttrs == NULL) {
        jepThread->fqnToPyJAttrs = PyDict_New();
    }

    cachedAttrs = PyDict_GetItem(jepThread->fqnToPyJAttrs, pyClassName);
    if (cachedAttrs == NULL) {
        int i, len = 0;
        jobjectArray methodArray = NULL;
        jobjectArray fieldArray  = NULL;

        cachedAttrs = PyDict_New();

        // - GetMethodID fails when you pass the clazz object, it expects
        //   a java.lang.Class jobject.
        // - if you CallObjectMethod with the langClass jclass object,
        //   it'll return an array of methods, but they're methods of the
        //   java.lang.reflect.Method class -- not ->object.
        //
        // so what i did here was find the methodid using langClass,
        // but then i call the method using clazz. methodIds for java
        // classes are shared....
        methodArray = java_lang_Class_getMethods(env, pyjob->clazz);
        if (process_java_exception(env) || !methodArray) {
            goto EXIT_ERROR;
        }

        /*
         * For each method, create a new PyJMethod object and either add it to
         * the cached methods list or a PyJMultiMethod.
         */
        len = (*env)->GetArrayLength(env, methodArray);
        for (i = 0; i < len; i++) {
            PyJMethodObject *pymethod = NULL;
            jobject rmethod = NULL;

            rmethod = (*env)->GetObjectArrayElement(env, methodArray, i);
            pymethod = PyJMethod_New(env, rmethod);

            if (!pymethod) {
                continue;
            }

            /*
             * For every method of this name, check to see if a PyJMethod or
             * PyJMultiMethod is already in the cache with the same name. If
             * so, turn it into a PyJMultiMethod or add it to the existing
             * PyJMultiMethod.
             */
            if (pymethod->pyMethodName && PyString_Check(pymethod->pyMethodName)) {
                PyObject* cached = PyDict_GetItem(cachedAttrs, pymethod->pyMethodName);
                if (cached == NULL) {
                    if (PyDict_SetItem(cachedAttrs, pymethod->pyMethodName,
                                       (PyObject*) pymethod) != 0) {
                        goto EXIT_ERROR;
                    }
                } else if (PyJMethod_Check(cached)) {
                    PyObject* multimethod = PyJMultiMethod_New((PyObject*) pymethod, cached);
                    PyDict_SetItem(cachedAttrs, pymethod->pyMethodName, multimethod);
                    Py_DECREF(multimethod);
                } else if (PyJMultiMethod_Check(cached)) {
                    PyJMultiMethod_Append(cached, (PyObject*) pymethod);
                }
            }

            Py_DECREF(pymethod);
            (*env)->DeleteLocalRef(env, rmethod);
        } // end of looping over available methods

        //  process fields
        fieldArray = java_lang_Class_getFields(env, pyjob->clazz);
        if (process_java_exception(env) || !fieldArray) {
            goto EXIT_ERROR;
        }

        // for each field, create a pyjfield object and
        // add to the internal members list.
        len = (*env)->GetArrayLength(env, fieldArray);
        for (i = 0; i < len; i++) {
            jobject          rfield   = NULL;
            PyJFieldObject *pyjfield = NULL;

            rfield = (*env)->GetObjectArrayElement(env, fieldArray, i);

            pyjfield = PyJField_New(env, rfield);

            if (!pyjfield) {
                continue;
            }

            if (pyjfield->pyFieldName && PyString_Check(pyjfield->pyFieldName)) {
                if (PyDict_SetItem(cachedAttrs, pyjfield->pyFieldName,
                                   (PyObject*) pyjfield) != 0) {
                    goto EXIT_ERROR;
                }
            }

            Py_DECREF(pyjfield);
            (*env)->DeleteLocalRef(env, rfield);
        }
        (*env)->DeleteLocalRef(env, fieldArray);

        PyDict_SetItem(jepThread->fqnToPyJAttrs, pyClassName, cachedAttrs);
        Py_DECREF(cachedAttrs); // fqnToPyJAttrs will hold the reference
    } // end of setting up cache for this Java Class

    if (pyjob->object) {
        Py_INCREF(cachedAttrs);
        pyjob->attr = cachedAttrs;
    } else {
        /* PyJClass may add additional attributes so use a copy */
        pyjob->attr = PyDict_Copy(cachedAttrs);
    }

    (*env)->PopLocalFrame(env, NULL);
    return 1;


EXIT_ERROR:
    (*env)->PopLocalFrame(env, NULL);

    if (PyErr_Occurred()) { // java exceptions translated by this time
        if (pyjob) {
            pyjobject_dealloc(pyjob);
        }
    }

    return 0;
}


// called internally to make new PyJObject instances
PyObject* PyJObject_New(JNIEnv *env, jobject obj)
{
    PyJObject    *pyjob;
    jclass        objClz;
    int           jtype;

    if (!subtypes_initialized) {
        pyjobject_init_subtypes();
    }
    if (!obj) {
        PyErr_Format(PyExc_RuntimeError, "Invalid object.");
        return NULL;
    }

    objClz = (*env)->GetObjectClass(env, obj);

    /*
     * There exist situations where a Java method signature has a return
     * type of Object but actually returns a Class or array.  Also if you
     * call Jep.set(String, Object[]) it should be treated as an array, not
     * an object.  Hence this check here to build the optimal Jep type in
     * the interpreter regardless of signature.
     */
    jtype = get_jtype(env, objClz);
    if (jtype == JARRAY_ID) {
        return pyjarray_new(env, obj);
    } else if (jtype == JCLASS_ID) {
        return PyJObject_NewClass(env, obj);
    } else {
        /*
         * Check the Java type against our extensions to PyJObject. If we have
         * an extension to PyJObject that matches, then Python code using this
         * PyJObject can appear more pythonic: e.g. java.lang.Iterable can
         * become a PyJIterable and be used as a PyJObject but also as a Python
         * iterable such as "for i in obj:".
         */
        if ((*env)->IsInstanceOf(env, obj, JITERABLE_TYPE)) {
            if ((*env)->IsInstanceOf(env, obj, JCOLLECTION_TYPE)) {
                if ((*env)->IsInstanceOf(env, obj, JLIST_TYPE)) {
                    pyjob = PyJList_New();
                } else {
                    pyjob = PyJCollection_New();
                }
            } else {
                pyjob = PyJIterable_New();
            }
        } else if ((*env)->IsInstanceOf(env, obj, JMAP_TYPE)) {
            pyjob = PyJMap_New();
        } else if ((*env)->IsInstanceOf(env, obj, JITERATOR_TYPE)) {
            pyjob = PyJIterator_New();
        } else if ((*env)->IsInstanceOf(env, obj, JAUTOCLOSEABLE_TYPE)) {
            pyjob = PyJAutoCloseable_New();
        } else if ((*env)->IsInstanceOf(env, obj, JNUMBER_TYPE)) {
            pyjob = PyJNumber_New();
        } else {
            pyjob = PyObject_NEW(PyJObject, &PyJObject_Type);
        }
    }


    pyjob->object      = (*env)->NewGlobalRef(env, obj);
    pyjob->clazz       = (*env)->NewGlobalRef(env, objClz);

    (*env)->DeleteLocalRef(env, objClz);

    if (pyjobject_init(env, pyjob)) {
        return (PyObject *) pyjob;
    }
    return NULL;
}


PyObject* PyJObject_NewClass(JNIEnv *env, jclass clazz)
{
    PyJObject       *pyjob;
    PyJClassObject  *pyjclass;  // same object as pyjob, just casted

    if (!clazz) {
        PyErr_Format(PyExc_RuntimeError, "Invalid class object.");
        return NULL;
    }

    if (!PyJClass_Type.tp_base) {
        PyJClass_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJClass_Type) < 0) {
        return NULL;
    }

    pyjclass           = PyObject_NEW(PyJClassObject, &PyJClass_Type);
    pyjob              = (PyJObject*) pyjclass;
    pyjob->object      = NULL;
    pyjob->clazz       = (*env)->NewGlobalRef(env, clazz);

    if (pyjobject_init(env, pyjob)) {
        if (pyjclass_init(env, (PyObject *) pyjob)) {
            return (PyObject *) pyjob;
        }
    }
    return NULL;
}


void pyjobject_dealloc(PyJObject *self)
{
#if USE_DEALLOC
    JNIEnv *env = pyembed_get_env();
    if (env) {
        if (self->object) {
            (*env)->DeleteGlobalRef(env, self->object);
        }
        if (self->clazz) {
            (*env)->DeleteGlobalRef(env, self->clazz);
        }
    }

    Py_CLEAR(self->attr);
    Py_CLEAR(self->javaClassName);

    PyObject_Del(self);
#endif
}


int PyJObject_Check(PyObject *obj)
{
    if (PyObject_TypeCheck(obj, &PyJObject_Type)) {
        return 1;
    }
    return 0;
}

// call toString() on jobject. returns null on error.
// expected to return new reference.
static PyObject* pyjobject_str(PyJObject *self)
{
    PyObject   *pyres     = NULL;
    JNIEnv     *env;

    env   = pyembed_get_env();
    if (self->object) {
        pyres = jobject_topystring(env, self->object);
    } else {
        pyres = jobject_topystring(env, self->clazz);
    }

    if (process_java_exception(env)) {
        return NULL;
    }

    // python doesn't like Py_None here...
    if (pyres == NULL) {
        return Py_BuildValue("s", "");
    }

    return pyres;
}


static PyObject* pyjobject_richcompare(PyJObject *self,
                                       PyObject *_other,
                                       int opid)
{
    JNIEnv *env;

    if (PyType_IsSubtype(Py_TYPE(_other), &PyJObject_Type)) {
        PyJObject *other = (PyJObject *) _other;
        jboolean eq;

        jobject target, other_target;

        target = self->object;
        other_target = other->object;

        // lack of object indicates it's a pyjclass
        if (!target) {
            target = self->clazz;
        }
        if (!other_target) {
            other_target = other->clazz;
        }

        if (opid == Py_EQ && (self == other || target == other_target)) {
            Py_RETURN_TRUE;
        }

        env = pyembed_get_env();
        eq = JNI_FALSE;
        // skip calling Object.equals() if op is > or <
        if (opid != Py_GT && opid != Py_LT) {
            eq = java_lang_Object_equals(env, target, other_target);
        }

        if (process_java_exception(env)) {
            return NULL;
        }

        if (((eq == JNI_TRUE) && (opid == Py_EQ || opid == Py_LE || opid == Py_GE)) ||
                (eq == JNI_FALSE && opid == Py_NE)) {
            Py_RETURN_TRUE;
        } else if (opid == Py_EQ || opid == Py_NE) {
            Py_RETURN_FALSE;
        } else {
            /*
             * All Java objects have equals, but we must rely on Comparable for
             * the more advanced operators.  Java generics cannot actually
             * enforce the type of other in self.compareTo(other) at runtime,
             * but for simplicity let's assume if they got it to compile, the
             * two types can be compared. If the types aren't comparable to
             * one another, a ClassCastException will be thrown.
             *
             * In Python 2 we will allow the ClassCastException to halt the
             * comparison, because it will most likely return
             * NotImplemented in both directions and Python 2 will devolve to
             * comparing the pointer address.
             *
             * In Python 3 we will catch the ClassCastException and return
             * NotImplemented, because there's a chance the reverse comparison
             * of other.compareTo(self) will work.  If both directions return
             * NotImplemented (due to ClassCastException), Python 3 will
             * raise a TypeError.
             */
            jint result;
#if PY_MAJOR_VERSION >= 3
            jthrowable exc;
#endif

            if (!(*env)->IsInstanceOf(env, self->object, JCOMPARABLE_TYPE)) {
                char* jname = PyString_AsString(self->javaClassName);
                PyErr_Format(PyExc_TypeError, "Invalid comparison operation for Java type %s",
                             jname);
                return NULL;
            }

            result = java_lang_Comparable_compareTo(env, target, other_target);
#if PY_MAJOR_VERSION >= 3
            exc = (*env)->ExceptionOccurred(env);
            if (exc != NULL) {
                if ((*env)->IsInstanceOf(env, exc, CLASSCAST_EXC_TYPE)) {
                    /*
                     * To properly meet the richcompare docs we detect
                     * ClassException and return NotImplemented, enabling
                     * Python to try the reverse operation of
                     * other.compareTo(self).  Unfortunately this only safely
                     * works in Python 3.
                     */
                    (*env)->ExceptionClear(env);
                    Py_INCREF(Py_NotImplemented);
                    return Py_NotImplemented;
                }
            }
#endif
            if (process_java_exception(env)) {
                return NULL;
            }

            if ((result == -1 && opid == Py_LT) || (result == -1 && opid == Py_LE) ||
                    (result == 1 && opid == Py_GT) || (result == 1 && opid == Py_GE)) {
                Py_RETURN_TRUE;
            } else {
                Py_RETURN_FALSE;
            }
        }
    }

    /*
     * Reaching this point means we are comparing a Java object to a Python
     * object.  You might think that's not allowed, but the python doc on
     * richcompare indicates that when encountering NotImplemented, allow the
     * reverse comparison in the hopes that that's implemented.  This works
     * surprisingly well because it enables Python comparison operations on
     * things such as pyjobject != Py_None or
     * assertSequenceEqual(pyjlist, pylist) where each list has the same
     * contents.  This saves us from having to worry about if the Java object
     * is on the left side or the right side of the operator.
     *
     * In short, this is intentional to keep comparisons working well.
     */
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
}


// get attribute 'name' for object.
// uses obj->attr dict for storage.
// returns new reference.
static PyObject* pyjobject_getattro(PyObject *obj, PyObject *name)
{
    PyObject *ret = PyObject_GenericGetAttr(obj, name);
    if (ret == NULL) {
        return NULL;
    } else if (PyJMethod_Check(ret) || PyJMultiMethod_Check(ret)) {
        /*
         * TODO Should not bind non-static methods to pyjclass objects, but not
         * sure yet how to handle multimethods and static methods.
         */
#if PY_MAJOR_VERSION >= 3
        PyObject* wrapper = PyMethod_New(ret, (PyObject*) obj);
#else
        PyObject* wrapper = PyMethod_New(ret, (PyObject*) obj,
                                         (PyObject*) Py_TYPE(obj));
#endif
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
static int pyjobject_setattro(PyJObject *obj, PyObject *name, PyObject *v)
{
    PyObject *cur = NULL;
    if (v == NULL) {
        PyErr_Format(PyExc_TypeError,
                     "Deleting attributes from PyJObjects is not allowed.");
        return -1;
    }

    cur = PyDict_GetItem(obj->attr, name);
    if (PyErr_Occurred()) {
        return -1;
    }

    if (cur == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "No such field.");
        return -1;
    }

    if (!PyJField_Check(cur)) {
        PyErr_SetString(PyExc_TypeError, "Not a pyjfield object.");
        return -1;
    }

    return pyjfield_set((PyJFieldObject *) cur, obj, v);
}

static long pyjobject_hash(PyJObject *self)
{
    JNIEnv *env = pyembed_get_env();
    int   hash = -1;

    if (self->object) {
        hash = java_lang_Object_hashCode(env, self->object);
    } else {
        hash = java_lang_Object_hashCode(env, self->clazz);
    }
    if (process_java_exception(env)) {
        return -1;
    }

    /*
     * This seems odd but Python expects -1 for error occurred. Other Python
     * built-in types then return -2 if the actual hash is -1.
     */
    if (hash == -1) {
        hash = -2;
    }

    return hash;
}

/*
 * Creates a PyJMonitor that can emulate a Java synchronized(self) {...} block.
 */
static PyObject* pyjobject_synchronized(PyObject* self, PyObject* args)
{
    PyObject   *monitor = NULL;
    PyJObject  *thisObj = (PyJObject*) self;

    if (thisObj->object) {
        // PyJObject
        monitor = PyJMonitor_New(thisObj->object);
    } else {
        // PyJClass
        monitor = PyJMonitor_New(thisObj->clazz);
    }

    return monitor;
}

static PyMethodDef pyjobject_methods[] = {
    {
        "synchronized",
        pyjobject_synchronized,
        METH_NOARGS,
        "synchronized that emulates Java's synchronized { obj } and returns a Python ContextManager"
    },

    { NULL, NULL }
};


static PyMemberDef pyjobject_members[] = {
    {"__dict__", T_OBJECT, offsetof(PyJObject, attr), READONLY},
    {"java_name", T_OBJECT, offsetof(PyJObject, javaClassName), READONLY},
    {0}
};


PyTypeObject PyJObject_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJObject",                          /* tp_name */
    sizeof(PyJObject),                        /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor) pyjobject_dealloc,           /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    (hashfunc) pyjobject_hash,                /* tp_hash  */
    0,                                        /* tp_call */
    (reprfunc) pyjobject_str,                 /* tp_str */
    (getattrofunc) pyjobject_getattro,        /* tp_getattro */
    (setattrofunc) pyjobject_setattro,        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    "jobject",                                /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    (richcmpfunc) pyjobject_richcompare,      /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    pyjobject_methods,                        /* tp_methods */
    pyjobject_members,                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    offsetof(PyJObject, attr),                /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
