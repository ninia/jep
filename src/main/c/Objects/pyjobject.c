/*
   jep - Java Embedded Python

   Copyright (c) 2004-2025 JEP AUTHORS.

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

int PyJObject_Check(PyObject *pyobj)
{
    JepModuleState* modState = pyembed_get_module_state();
    if (modState) {
        return PyObject_TypeCheck(pyobj, modState->PyJObject_Type);
    }
    return 0;
}

PyObject* PyJObject_New(JNIEnv *env, PyTypeObject* type, jobject obj,
                        jclass class)
{
    PyJObject *pyjob = (PyJObject*) type->tp_alloc(type, 0);

    if (obj) {
        pyjob->object = (*env)->NewGlobalRef(env, obj);
    } else {
        /* This should only happen for pyjclass*/
        pyjob->object = NULL;
    }
    if (class) {
        pyjob->clazz = (*env)->NewGlobalRef(env, class);
    } else {
        class = (*env)->GetObjectClass(env, obj);
        pyjob->clazz = (*env)->NewGlobalRef(env, class);
        (*env)->DeleteLocalRef(env, class);
        class = NULL;
    }

    return (PyObject*) pyjob;
}

static void pyjobject_dealloc(PyJObject *self)
{
#if USE_DEALLOC
    PyObject_GC_UnTrack(self);
    PyTypeObject *tp = Py_TYPE(self);
    JNIEnv *env = pyembed_get_env();
    if (env) {
        if (self->object) {
            (*env)->DeleteGlobalRef(env, self->object);
        }
        if (self->clazz) {
            (*env)->DeleteGlobalRef(env, self->clazz);
        }
    }
    tp->tp_free((PyObject*) self);
    Py_DECREF(tp);
#endif
}

static int pyjobject_traverse(PyJObject *self, visitproc visit, void *arg)
{
    Py_VISIT(Py_TYPE(self));
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
        pyres = jobject_As_PyString(env, self->object);
    } else {
        pyres = jobject_As_PyString(env, self->clazz);
    }

    return pyres;
}

static PyObject* pyjobject_richcompare(PyJObject *self,
                                       PyObject *_other,
                                       int opid)
{
    JNIEnv *env;

    if (PyJObject_Check(_other)) {
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
            jthrowable exc;
            if (!(*env)->IsInstanceOf(env, self->object, JCOMPARABLE_TYPE)) {
                PyErr_Format(PyExc_TypeError, "Invalid comparison operation for Java type %s",
                             ((PyObject*) self)->ob_type->tp_name);
                return NULL;
            }

            result = java_lang_Comparable_compareTo(env, target, other_target);
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

static Py_hash_t pyjobject_hash(PyJObject *self)
{
    JNIEnv *env    = pyembed_get_env();
    Py_hash_t hash = -1;

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

static PyObject* pyjobject_getJavaClassName(PyJObject* pyjob)
{
    JNIEnv* env = pyembed_get_env();
    jstring className     = NULL;
    PyObject *pyClassName = NULL;

    className = java_lang_Class_getName(env, pyjob->clazz);
    if (process_java_exception(env) || !className) {
        return NULL;
    }
    pyClassName = jstring_As_PyString(env, className);
    (*env)->DeleteLocalRef(env, className);
    return pyClassName;
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

/*
 * Attach the attribute java_name to the PyJObject instance to assist
 * developers with understanding the type at runtime.
 */
static PyGetSetDef pyjobject_getset[] = {
    {"java_name", (getter) pyjobject_getJavaClassName, NULL},
    {NULL} /* Sentinel */
};

static PyType_Slot slots[] = {
    {Py_tp_doc, "Jep java.lang.Object"},
    {Py_tp_dealloc, pyjobject_dealloc},
    {Py_tp_traverse, pyjobject_traverse},
    {Py_tp_hash, pyjobject_hash},
    {Py_tp_str, pyjobject_str},
    {Py_tp_richcompare, pyjobject_richcompare},
    {Py_tp_methods, pyjobject_methods},
    {Py_tp_getset, pyjobject_getset},
    {0, NULL},
};

PyType_Spec PyJObject_Spec = {
    .name = "java.lang.Object",
    .basicsize = sizeof(PyJObject),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE,
    .slots = slots
};
