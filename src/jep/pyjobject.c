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
#include <jni.h>

// shut up the compiler
#ifdef _POSIX_C_SOURCE
#  undef _POSIX_C_SOURCE
#endif
#ifdef _FILE_OFFSET_BITS
# undef _FILE_OFFSET_BITS
#endif
#include "Python.h"

#include "pyembed.h"
#include "pyjobject.h"
#include "pyjmethod.h"
#include "pyjfield.h"
#include "pyjclass.h"
#include "util.h"
#include "pyjarray.h"
#include "pyjmethodwrapper.h"
#include "pyjiterable.h"
#include "pyjiterator.h"
#include "pyjcollection.h"
#include "pyjlist.h"
#include "pyjmap.h"

static int pyjobject_init(JNIEnv *env, PyJobject_Object*);
static void pyjobject_addmethod(PyJobject_Object*, PyObject*);
static void pyjobject_init_subtypes(void);
static int  subtypes_initialized = 0;

static jmethodID objectGetClass  = 0;
static jmethodID objectEquals    = 0;
static jmethodID objectHashCode  = 0;
static jmethodID classGetMethods = 0;
static jmethodID classGetFields  = 0;
static jmethodID classGetName    = 0;

/*
 * MSVC requires tp_base to be set at runtime instead of in
 * the type declaration. :/  Since we are building an
 * inheritance tree of types, we need to ensure that all the
 * tp_base are set for the subtypes before we possibly use
 * those subtypes.
 *
 * See https://docs.python.org/2/extending/newtypes.html
 *     https://docs.python.org/3/extending/newtypes.html
 */
static void pyjobject_init_subtypes(void) {
    // start at the top with object
    if(PyType_Ready(&PyJobject_Type) < 0)
        return;

    // next do iterable
    if(!PyJiterable_Type.tp_base) {
        PyJiterable_Type.tp_base = &PyJobject_Type;
    }
    if(PyType_Ready(&PyJiterable_Type) < 0)
        return;

    // next do collection
    if(!PyJcollection_Type.tp_base) {
        PyJcollection_Type.tp_base = &PyJiterable_Type;
    }
    if(PyType_Ready(&PyJcollection_Type) < 0)
        return;

    // next do list
    if(!PyJlist_Type.tp_base) {
        PyJlist_Type.tp_base = &PyJcollection_Type;
    }
    if(PyType_Ready(&PyJlist_Type) < 0)
        return;

    // last do map
    if(!PyJmap_Type.tp_base) {
        PyJmap_Type.tp_base = &PyJobject_Type;
    }
    if(PyType_Ready(&PyJmap_Type) < 0)
        return;
    
    subtypes_initialized = 1;
}


// called internally to make new PyJobject_Object instances
PyObject* pyjobject_new(JNIEnv *env, jobject obj) {
    PyJobject_Object *pyjob;
    jclass            objClz;
    int               jtype;
    
    if(!subtypes_initialized) {
        pyjobject_init_subtypes();
    }
    if(!obj) {
        PyErr_Format(PyExc_RuntimeError, "Invalid object.");
        return NULL;
    }

    objClz = (*env)->GetObjectClass(env, obj);

    /*
     * There exist situations where a Java method signature has a return
     * type of Object but actually returns a Class or array.  Also if you
     * call Jep.set(String, Object[]) it should be treated as an array, not
     * an object.  Hence this check here to build the optimal jep type in
     * the interpreter regardless of signature.
     */
    jtype = get_jtype(env, objClz);
    if(jtype == JARRAY_ID) {
        return pyjarray_new(env, obj);
    } else if(jtype == JCLASS_ID) {
        return pyjobject_new_class(env, obj);
    } else {
#if USE_NUMPY
        /*
         * check for jep/NDArray and autoconvert to numpy.ndarray instead of
         * pyjobject
         */
        if(jndarray_check(env, obj)) {
            return convert_jndarray_pyndarray(env, obj);
        }
        if(PyErr_Occurred()) {
            return NULL;
        }
#endif

        // check for some of our extensions to pyjobject
        if((*env)->IsInstanceOf(env, obj, JITERABLE_TYPE)) {
            if((*env)->IsInstanceOf(env, obj, JCOLLECTION_TYPE)) {
                if((*env)->IsInstanceOf(env, obj, JLIST_TYPE)) {
                    pyjob = (PyJobject_Object*) pyjlist_new();
                } else {
                    // a Collection we have less support for
                    pyjob = (PyJobject_Object*) pyjcollection_new();
                }
            } else {
                // an Iterable we have less support for
                pyjob = (PyJobject_Object*) pyjiterable_new();
            }
        } else if((*env)->IsInstanceOf(env, obj, JMAP_TYPE)) { 
            pyjob = (PyJobject_Object*) pyjmap_new();
        } else if((*env)->IsInstanceOf(env, obj, JITERATOR_TYPE)) {
            pyjob = (PyJobject_Object*) pyjiterator_new();
        } else {
            pyjob = PyObject_NEW(PyJobject_Object, &PyJobject_Type);
        }
    }


    pyjob->object      = (*env)->NewGlobalRef(env, obj);
    pyjob->clazz       = (*env)->NewGlobalRef(env, (*env)->GetObjectClass(env, obj));
    pyjob->attr        = PyList_New(0);
    pyjob->methods     = PyList_New(0);
    pyjob->fields      = PyList_New(0);
    pyjob->finishAttr  = 0;

    if(pyjobject_init(env, pyjob))
        return (PyObject *) pyjob;
    return NULL;
}


PyObject* pyjobject_new_class(JNIEnv *env, jclass clazz) {
    PyJobject_Object *pyjob;
    PyJclass_Object  *pyjclass;  // same object as pyjob, just casted
    
    if(!clazz) {
        PyErr_Format(PyExc_RuntimeError, "Invalid class object.");
        return NULL;
    }

   if(!PyJclass_Type.tp_base) {
       PyJclass_Type.tp_base = &PyJobject_Type;
   }
   if(PyType_Ready(&PyJclass_Type) < 0)
        return NULL;

    pyjclass           = PyObject_NEW(PyJclass_Object, &PyJclass_Type);
    pyjob              = (PyJobject_Object*) pyjclass;
    pyjob->object      = NULL;
    pyjob->clazz       = (*env)->NewGlobalRef(env, clazz);
    pyjob->attr        = PyList_New(0);
    pyjob->methods     = PyList_New(0);
    pyjob->fields      = PyList_New(0);
    pyjob->finishAttr  = 0;

    if(pyjclass_init(env, (PyObject *) pyjob)) {
        if(pyjobject_init(env, pyjob))
            return (PyObject *) pyjob;
    }
    return NULL;
}


static int pyjobject_init(JNIEnv *env, PyJobject_Object *pyjob) {
    jobjectArray      methodArray = NULL;
    jobjectArray      fieldArray  = NULL;
    int               i, len = 0;
    jobject           langClass   = NULL;

    jstring           className   = NULL;
    const char       *cClassName  = NULL;
    PyObject         *pyClassName = NULL;
    PyObject         *pyAttrName  = NULL;

    JepThread   *jepThread;
    PyObject    *cachedMethodList = NULL;

    (*env)->PushLocalFrame(env, 20);
    // ------------------------------ call Class.getMethods()

    // well, first call getClass()
    if(objectGetClass == 0) {
        objectGetClass = (*env)->GetMethodID(env,
                                             pyjob->clazz,
                                             "getClass",
                                             "()Ljava/lang/Class;");
        if(process_java_exception(env) || !objectGetClass)
            goto EXIT_ERROR;
    }

    langClass = (*env)->CallObjectMethod(env, pyjob->clazz, objectGetClass);
    if(process_java_exception(env) || !langClass)
        goto EXIT_ERROR;

    /*
     * attach attribute java_name to the pyjobject instance to assist with
     * understanding the type at runtime
     */
    if(classGetName == 0) {
        classGetName = (*env)->GetMethodID(env, langClass, "getName",
                "()Ljava/lang/String;");
    }
    className = (*env)->CallObjectMethod(env, pyjob->clazz, classGetName);
    cClassName = jstring2char(env, className);
    pyClassName = PyString_FromString(cClassName);
    release_utf_char(env, className, cClassName);
    pyAttrName = PyString_FromString("java_name");
    if(PyObject_SetAttr((PyObject *) pyjob, pyAttrName, pyClassName) == -1) {
        PyErr_Format(PyExc_RuntimeError,
                "Couldn't add java_name as attribute.");
    } else {
        pyjobject_addfield(pyjob, pyAttrName);
    }
    pyjob->javaClassName = pyClassName;
    Py_DECREF(pyAttrName);
    (*env)->DeleteLocalRef(env, className);

    // then, get methodid for getMethods()
    if(classGetMethods == 0) {
        classGetMethods = (*env)->GetMethodID(env,
                                              langClass,
                                              "getMethods",
                                              "()[Ljava/lang/reflect/Method;");
        if(process_java_exception(env) || !classGetMethods)
            goto EXIT_ERROR;
    }

    /*
     * Performance improvement.  The code below is very similar to previous
     * versions except methods are now cached in memory.
     *
     * Previously every time you instantiate a pyjobject, JEP would get the
     * complete list of methods, turn them into pyjmethods, and add them as
     * attributes to the pyjobject.
     *
     * Now JEP retains a python dictionary in memory with a key of the fully
     * qualified Java classname to a list of pyjmethods. Since the
     * Java methods will never change at runtime for a particular Class, this
     * is safe and drastically speeds up pyjobject instantiation by reducing
     * reflection calls. We continue to set and reuse the pyjmethods as
     * attributes on the pyjobject instance, but if pyjobject_getattr sees a
     * pyjmethod, it will put it inside a pyjmethodwrapper and return that,
     * enabling the reuse of the pyjmethod for this particular object instance.
     * For more info, see pyjmethodwrapper.
     *
     * We have the GIL at this point, so we can safely assume we're
     * synchronized and multiple threads will not alter the dictionary at the
     * same time.
     */
    jepThread = pyembed_get_jepthread();
    if(jepThread == NULL) {
        goto EXIT_ERROR;
    }
    if(jepThread->fqnToPyJmethods == NULL) {
        PyObject *methodCache = PyDict_New();
        jepThread->fqnToPyJmethods = methodCache;
    }

    cachedMethodList = PyDict_GetItem(jepThread->fqnToPyJmethods, pyClassName);
    if(cachedMethodList == NULL) {
        PyObject *pyjMethodList = NULL;
        pyjMethodList = PyList_New(0);

        // - GetMethodID fails when you pass the clazz object, it expects
        //   a java.lang.Class jobject.
        // - if you CallObjectMethod with the langClass jclass object,
        //   it'll return an array of methods, but they're methods of the
        //   java.lang.reflect.Method class -- not ->object.
        //
        // so what i did here was find the methodid using langClass,
        // but then i call the method using clazz. methodIds for java
        // classes are shared....

        methodArray = (jobjectArray) (*env)->CallObjectMethod(env, pyjob->clazz,
                classGetMethods);
        if(process_java_exception(env) || !methodArray)
            goto EXIT_ERROR;

        // for each method, create a new pyjmethod object
        // and add to the internal methods list.
        len = (*env)->GetArrayLength(env, methodArray);
        for (i = 0; i < len; i++) {
            PyJmethod_Object *pymethod = NULL;
            jobject rmethod = NULL;

            rmethod = (*env)->GetObjectArrayElement(env, methodArray, i);

            // make new PyJmethod_Object, linked to pyjob
            if(pyjob->object)
                pymethod = pyjmethod_new(env, rmethod, pyjob);
            else
                pymethod = pyjmethod_new_static(env, rmethod, pyjob);

            if(!pymethod)
                continue;

            if(pymethod->pyMethodName && PyString_Check(pymethod->pyMethodName)) {
                if(PyList_Append(pyjMethodList, (PyObject*) pymethod) != 0)
                    printf("WARNING: couldn't add method");
            }

            Py_DECREF(pymethod);
            (*env)->DeleteLocalRef(env, rmethod);
        } // end of looping over available methods
        PyDict_SetItem(jepThread->fqnToPyJmethods, pyClassName, pyjMethodList);
        cachedMethodList = pyjMethodList;
        Py_DECREF(pyjMethodList); // fqnToPyJmethods will hold the reference
        (*env)->DeleteLocalRef(env, methodArray);
    } // end of setting up cache for this Java Class

    len = (int) PyList_Size(cachedMethodList);
    for (i = 0; i < len; i++) {
        PyJmethod_Object* pymethod = (PyJmethod_Object*) PyList_GetItem(
                cachedMethodList, i);
        if(PyObject_SetAttr((PyObject *) pyjob, pymethod->pyMethodName,
                (PyObject*) pymethod) != 0) {
            PyErr_Format(PyExc_RuntimeError,
                    "Couldn't add method as attribute.");
        } else {
            pyjobject_addmethod(pyjob, pymethod->pyMethodName);
        }
    } // end of cached method optimizations
    

    // ------------------------------ process fields
    
    if(classGetFields == 0) {
        classGetFields = (*env)->GetMethodID(env,
                                             langClass,
                                             "getFields",
                                             "()[Ljava/lang/reflect/Field;");
        if(process_java_exception(env) || !classGetFields)
            goto EXIT_ERROR;
    }
    
    fieldArray = (jobjectArray) (*env)->CallObjectMethod(env,
                                                         pyjob->clazz,
                                                         classGetFields);
    if(process_java_exception(env) || !fieldArray)
        goto EXIT_ERROR;
    
    // for each field, create a pyjfield object and
    // add to the internal members list.
    len = (*env)->GetArrayLength(env, fieldArray);
    for(i = 0; i < len; i++) {
        jobject          rfield   = NULL;
        PyJfield_Object *pyjfield = NULL;
        
        rfield = (*env)->GetObjectArrayElement(env,
                                               fieldArray,
                                               i);
        
        pyjfield = pyjfield_new(env, rfield, pyjob);
        
        if(!pyjfield)
            continue;
        
        if(pyjfield->pyFieldName && PyString_Check(pyjfield->pyFieldName)) {
            if(PyObject_SetAttr((PyObject *) pyjob,
                                pyjfield->pyFieldName,
                                (PyObject *) pyjfield) != 0) {
                printf("WARNING: couldn't add field.\n");
            }
            else {
                pyjobject_addfield(pyjob, pyjfield->pyFieldName);
            }
        }
        
        Py_DECREF(pyjfield);
        (*env)->DeleteLocalRef(env, rfield);
    }
    (*env)->DeleteLocalRef(env, fieldArray);
    
    // we've finished the object.
    pyjob->finishAttr = 1;
    (*env)->PopLocalFrame(env, NULL);
    return 1;
    
    
EXIT_ERROR:
    (*env)->PopLocalFrame(env, NULL);
    
    if(PyErr_Occurred()) { // java exceptions translated by this time
        if(pyjob)
            pyjobject_dealloc(pyjob);
    }

    return 0;
}


void pyjobject_dealloc(PyJobject_Object *self) {
#if USE_DEALLOC
    JNIEnv *env = pyembed_get_env();
    if(env) {
        if(self->object)
            (*env)->DeleteGlobalRef(env, self->object);
        if(self->clazz)
            (*env)->DeleteGlobalRef(env, self->clazz);
    }

    Py_CLEAR(self->attr);
    Py_CLEAR(self->methods);
    Py_CLEAR(self->fields);
    Py_CLEAR(self->javaClassName);
    
    PyObject_Del(self);
#endif
}


int pyjobject_check(PyObject *obj) {
    if(PyObject_TypeCheck(obj, &PyJobject_Type))
        return 1;
    return 0;
}


// add a method name to obj->methods list
static void pyjobject_addmethod(PyJobject_Object *obj, PyObject *name) {
    if(!PyString_Check(name))
        return;
    if(!PyList_Check(obj->methods))
        return;

    PyList_Append(obj->methods, name);
}


void pyjobject_addfield(PyJobject_Object *obj, PyObject *name) {
    if(!PyString_Check(name))
        return;
    if(!PyList_Check(obj->fields))
        return;
    
    PyList_Append(obj->fields, name);
}


// find and call a method on this object that matches the python args.
// typically called by way of pyjmethod when python invokes __call__.
//
// steals reference to self, methodname and args.
PyObject* find_method(JNIEnv *env,
                      PyJobject_Object *self,
                      PyObject *methodName,
                      Py_ssize_t methodCount,
                      PyObject *attr,
                      PyObject *args) {
    // all possible method candidates
    PyJmethod_Object **cand = NULL;
    Py_ssize_t         pos, i, listSize, argsSize;
    
    pos = i = listSize = argsSize = 0;

    // not really likely if we were called from pyjmethod, but hey...
    if(methodCount < 1) {
        PyErr_Format(PyExc_RuntimeError, "I have no methods.");
        return NULL;
    }

    if(!attr || !PyList_CheckExact(attr)) {
        PyErr_Format(PyExc_RuntimeError, "Invalid attr list.");
        return NULL;
    }
    
    cand = (PyJmethod_Object **)
        PyMem_Malloc(sizeof(PyJmethod_Object*) * methodCount);
    
    // just for safety
    for(i = 0; i < methodCount; i++)
        cand[i] = NULL;
    
    listSize = PyList_GET_SIZE(attr);
    for(i = 0; i < listSize; i++) {
        PyObject *tuple = PyList_GetItem(attr, i);               /* borrowed */

        if(PyErr_Occurred())
            break;
        
        if(!tuple || tuple == Py_None || !PyTuple_CheckExact(tuple))
            continue;

        if(PyTuple_Size(tuple) == 2) {
            PyObject *key = PyTuple_GetItem(tuple, 0);           /* borrowed */
            
            if(PyErr_Occurred())
                break;
            
            if(!key || !PyString_Check(key))
                continue;
            
            if(PyObject_RichCompareBool(key, methodName, Py_EQ)) {
                PyObject *method = PyTuple_GetItem(tuple, 1);    /* borrowed */
                if(pyjmethod_check(method))
                    cand[pos++] = (PyJmethod_Object *) method;
            }
        }
    }
    
    if(PyErr_Occurred())
        goto EXIT_ERROR;
    
    // makes more sense to work with...
    pos--;
    
    if(pos < 0) {
        // didn't find a method by that name....
        // that shouldn't happen unless the search above is broken.
        PyErr_Format(PyExc_NameError, "No such method.");
        goto EXIT_ERROR;
    }
    if(pos == 0) {
        // we're done, call that one
        PyObject *ret = pyjmethod_call_internal(cand[0], self, args);
        PyMem_Free(cand);
        return ret;
    }

    // first, find out if there's only one method that
    // has the correct number of args
    argsSize = PyTuple_Size(args);
    {
        PyJmethod_Object *matching = NULL;
        int               count    = 0;
        
        for(i = 0; i <= pos && cand[i]; i++) {
            // make sure method is fully initialized
            if(!cand[i]->parameters) {
                if(!pyjmethod_init(env, cand[i])) {
                    // init failed, that's not good.
                    cand[i] = NULL;
                    PyErr_Warn(PyExc_Warning, "pyjmethod init failed.");
                    continue;
                }
            }

            if(cand[i]->lenParameters == argsSize) {
                matching = cand[i];
                count++;
            }
            else
                cand[i] = NULL; // eliminate non-matching
        }
        
        if(matching && count == 1) {
            PyMem_Free(cand);
            return pyjmethod_call_internal(matching, self, args);
        }
    } // local scope
    
    for(i = 0; i <= pos; i++) {
        int parmpos = 0;
        
        // already eliminated?
        if(!cand[i])
            continue;
        
        // check if argument types match
        (*env)->PushLocalFrame(env, 20);
        for(parmpos = 0; parmpos < cand[i]->lenParameters; parmpos++) {
            PyObject *param       = PyTuple_GetItem(args, parmpos);
            int       paramTypeId = -1;
            jclass    paramType =
                (jclass) (*env)->GetObjectArrayElement(env,
                                                       cand[i]->parameters,
                                                       parmpos);

            if(process_java_exception(env) || !paramType)
                break;
            
            paramTypeId = get_jtype(env, paramType);
            
            if(pyarg_matches_jtype(env, param, paramType, paramTypeId)) {
                if(PyErr_Occurred())
                    break;
                continue;
            }
            
            // args don't match
            break;
        }
        (*env)->PopLocalFrame(env, NULL);
        
        // this method matches?
        if(parmpos == cand[i]->lenParameters) {
            PyObject *ret = pyjmethod_call_internal(cand[i], self, args);
            PyMem_Free(cand);
            return ret;
        }
    }


EXIT_ERROR:
    PyMem_Free(cand);
    if(!PyErr_Occurred())
        PyErr_Format(PyExc_NameError,
                     "Matching overloaded method not found.");
    return NULL;
}


// find and call a method on this object that matches the python args.
// typically called from pyjmethod when python invokes __call__.
//
// steals reference to self, methodname and args.
PyObject* pyjobject_find_method(PyJobject_Object *self,
                                PyObject *methodName,
                                PyObject *args) {
    // util method does this for us
    return find_method(pyembed_get_env(),
                       self,
                       methodName,
                       PyList_Size(self->methods),
                       self->attr,
                       args);
}


// call toString() on jobject. returns null on error.
// excpected to return new reference.
PyObject* pyjobject_str(PyJobject_Object *self) {
    PyObject   *pyres     = NULL;
    JNIEnv     *env;

    env   = pyembed_get_env();
    if(self->object)
        pyres = jobject_topystring(env, self->object, self->clazz);
    else
        pyres = jobject_topystring(env, self->clazz, self->clazz);

    if(process_java_exception(env))
        return NULL;

    // python doesn't like Py_None here...
    if(pyres == NULL)
        return Py_BuildValue("s", "");
    
    return pyres;
}


static PyObject* pyjobject_richcompare(PyJobject_Object *self,
                                       PyObject *_other,
                                       int opid) {
    JNIEnv *env;

    if(PyType_IsSubtype(Py_TYPE(_other), &PyJobject_Type)) {
        PyJobject_Object *other = (PyJobject_Object *) _other;
        jboolean eq;

        jobject target, other_target;

        target = self->object;
        other_target = other->object;

        // lack of object indicates it's a pyjclass
        if(!target) {
            target = self->clazz;
        }
        if(!other_target) {
            other_target = other->clazz;
        }

        if(opid == Py_EQ && (self == other || target == other_target)) {
            Py_RETURN_TRUE;
        }

        env = pyembed_get_env();
        eq = JNI_FALSE;
        // skip calling Object.equals() if op is > or <
        if(opid != Py_GT && opid != Py_LT) {
            // get the methodid for Object.equals()
            if(objectEquals == 0) {
                objectEquals = (*env)->GetMethodID(
                    env,
                    self->clazz,
                    "equals",
                    "(Ljava/lang/Object;)Z");
                if(process_java_exception(env) || !objectEquals)
                    return NULL;
            }

            eq = (*env)->CallBooleanMethod(
                env,
                target,
                objectEquals,
                other_target);
        }

        if(process_java_exception(env))
            return NULL;

        if(((eq == JNI_TRUE) && (opid == Py_EQ || opid == Py_LE || opid == Py_GE)) ||
            (eq == JNI_FALSE && opid == Py_NE)) {
            Py_RETURN_TRUE;
        } else if(opid == Py_EQ || opid == Py_NE) {
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
            jclass comparable;
            jmethodID compareTo;
            jint result;
#if PY_MAJOR_VERSION >= 3
            jthrowable exc;
#endif

            comparable = (*env)->FindClass(env, "java/lang/Comparable");
            if(!(*env)->IsInstanceOf(env, self->object, comparable)) {
                char* jname = PyString_AsString(self->javaClassName);
                PyErr_Format(PyExc_TypeError, "Invalid comparison operation for Java type %s", jname);
                return NULL;
            }

            compareTo = (*env)->GetMethodID(env, comparable, "compareTo", "(Ljava/lang/Object;)I");
            if(process_java_exception(env) || !compareTo) {
                return NULL;
            }

            result = (*env)->CallIntMethod(env, target, compareTo, other_target);
#if PY_MAJOR_VERSION >= 3
            exc = (*env)->ExceptionOccurred(env);
            if(exc != NULL) {
                jclass castExc = (*env)->FindClass(env, "java/lang/ClassCastException");
                if((*env)->IsInstanceOf(env, exc, castExc)) {
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
            if(process_java_exception(env)) {
                return NULL;
            }

            if((result == -1 && opid == Py_LT) || (result == -1 && opid == Py_LE) ||
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
     * suprisingly well because it enables python comparison operations on
     * things such as pyjobject != Py_None or
     * assertSequenceEqual(pyjlist, pylist) where each list has the same
     * contents.  This saves us from having to worry about if the java object
     * is on the left side or the right side of the operator.
     *
     * In short, this is intentional to keep comparisons working well.
     */
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
}


// get attribute 'name' for object.
// uses obj->attr list of tuples for storage.
// returns new reference.
PyObject* pyjobject_getattr(PyJobject_Object *obj,
                                   char *name) {
    PyObject *ret, *pyname, *methods, *members;
    ret = pyname = methods = members = NULL;
    
    if(!name) {
        Py_RETURN_NONE;
    }
    pyname  = PyString_FromString(name);
    methods = PyString_FromString("__methods__");
    members = PyString_FromString("__members__");
    
    if(PyObject_RichCompareBool(pyname, methods, Py_EQ)) {
        Py_DECREF(pyname);
        Py_DECREF(methods);
        Py_DECREF(members);

        Py_INCREF(obj->methods);
        return obj->methods;
    }
    Py_DECREF(methods);
    
    if(PyObject_RichCompareBool(pyname, members, Py_EQ)) {
        Py_DECREF(pyname);
        Py_DECREF(members);
        
        Py_INCREF(obj->fields);
        return obj->fields;
    }
    Py_DECREF(members);
    
    if(!PyList_Check(obj->attr)) {
        Py_DECREF(pyname);
        PyErr_Format(PyExc_RuntimeError, "Invalid attr list.");
        return NULL;
    }
    
    // util function fetches from attr list for us.
    ret = tuplelist_getitem(obj->attr, pyname);      /* new reference */
    
    Py_DECREF(pyname);
    
    // method optimizations
    if(pyjmethod_check(ret))
    {
        PyJmethodWrapper_Object *wrapper = pyjmethodwrapper_new(obj, (PyJmethod_Object*) ret);
        Py_DECREF(ret);
        ret = (PyObject *) wrapper;
    }

    if(PyErr_Occurred() || ret == Py_None) {
        if(ret == Py_None)
            Py_DECREF(Py_None);
        PyErr_Format(PyExc_AttributeError, "attr not found: %s", name);
        return NULL;
    }
    
    if(pyjfield_check(ret)) {
        PyObject *t = pyjfield_get((PyJfield_Object *) ret);
        Py_DECREF(ret);
        return t;
    }
    
    return ret;
}


// set attribute v for object.
// uses obj->attr dictionary for storage.
int pyjobject_setattr(PyJobject_Object *obj,
                             char *name,
                             PyObject *v) {
    PyObject *pyname, *tuple;

    if(v == NULL) {
        PyErr_Format(PyExc_TypeError, "Deleting attributes from PyJobjects is not allowed");
        return -1;
    }
    if(!name) {
        PyErr_Format(PyExc_RuntimeError, "Invalid name: NULL.");
        return -1;
    }
    
    if(!PyList_Check(obj->attr)) {
        PyErr_Format(PyExc_RuntimeError, "Invalid attr list.");
        return -1;
    }
    
    Py_INCREF(v);
    
    if(obj->finishAttr) {
        PyObject *cur, *pyname;
        int       ret;
        
        // finished setting internal objects.
        // don't allow python to add new, but do
        // allow python script to change values on pyjfields
        
        pyname = PyString_FromString(name);
        cur    = tuplelist_getitem(obj->attr, pyname);      /* new reference */
        Py_DECREF(pyname);

        if(PyErr_Occurred())
            return -1;
        
        if(cur == Py_None) {
            PyErr_SetString(PyExc_RuntimeError, "No such field.");
            return -1;
        }
        
        if(!pyjfield_check(cur)) {
            PyErr_SetString(PyExc_TypeError, "Not a pyjfield object.");
            return -1;
        }
        
        if(!PyList_Check(obj->attr)) {
            Py_DECREF(pyname);
            PyErr_SetString(PyExc_RuntimeError, "Invalid attr list.");
            return -1;
        }
        
        // now, just ask pyjfield to handle.
        ret = pyjfield_set((PyJfield_Object *) cur, v); /* borrows ref */
        
        Py_DECREF(cur);
        Py_DECREF(v);
        return ret;
    }
    
    pyname = PyString_FromString((const char *) name);
    tuple  = PyTuple_New(2);
    
    Py_INCREF(pyname);
    PyTuple_SetItem(tuple, 0, pyname);   /* steals ref */
    PyTuple_SetItem(tuple, 1, v);        /* steals ref */
    
    // the docs don't mention this, but the source INCREFs tuple
    // ...
    // after much printf'ing. uhm. must decref it somewhere.
    // ...
    // doh. the docs suck.
    
    // Py_INCREF(tuple);
    
    PyList_Append(obj->attr, tuple);
    
    Py_DECREF(tuple);
    Py_DECREF(pyname);
    return 0;  // success
}

static long pyjobject_hash(PyJobject_Object *self) {
    JNIEnv *env = pyembed_get_env();
    int   hash = -1;

    if(objectHashCode == 0) {
        objectHashCode = (*env)->GetMethodID(env,
                                             self->clazz,
                                             "hashCode",
                                             "()I");
        if(process_java_exception(env) || !objectHashCode) {
            return -1;
        }
    }

    if(self->object) {
        hash = (*env)->CallIntMethod(env, self->object, objectHashCode);
    } else {
        hash = (*env)->CallIntMethod(env, self->clazz, objectHashCode);
    }
    if(process_java_exception(env)) {
        return -1;
    }

    /*
     * this seems odd but python expects -1 for error occurred and other
     * built-in types then return -2 if the actual hash is -1
     */
    if(hash == -1) {
        hash = -2;
    }

    return hash;
}


/*
 * Implements PyObject_Dir(PyObject*) for pyjobjects. This is required for
 * Python 3.3+ for dir(pyjobject) to work correctly.
 */
static PyObject* pyjobject_dir(PyObject *o, PyObject* ignore) {
    PyObject* attrs;
    PyJobject_Object *self = (PyJobject_Object*) o;
    Py_ssize_t size, i, contains;

    attrs = PyList_New(0);
    size = PySequence_Size(self->methods);
    for(i = 0; i < size; i++) {
        PyObject *item = PySequence_GetItem(self->methods, i);
        contains = PySequence_Contains(attrs, item);
        if(contains < 0) {
           Py_DECREF(attrs);
           return NULL;
        } else if(contains == 0) {
            if(PyList_Append(attrs, item) < 0) {
               Py_DECREF(attrs);
               return NULL; 
            }
        }
    }

    // TODO copy/paste is bad, turn it into a method
    size = PySequence_Size(self->fields);
    for(i = 0; i < size; i++) {
        PyObject *item = PySequence_GetItem(self->fields, i);
        contains = PySequence_Contains(attrs, item);
        if(contains < 0) {
           Py_DECREF(attrs);
           return NULL;
        } else if(contains == 0) {
            if(PyList_Append(attrs, item) < 0) {
               Py_DECREF(attrs);
               return NULL;
            }
        }
    }
 
    if(PyList_Sort(attrs) < 0) {
       Py_DECREF(attrs);
       return NULL;
    }

    return attrs;
}


static struct PyMethodDef pyjobject_methods[] = {
    { "__dir__",
      (PyCFunction) pyjobject_dir,
      METH_NOARGS,
      "__dir__ for pyjobjects"
    },

    { NULL, NULL }
};


PyTypeObject PyJobject_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "PyJobject",                              /* tp_name */
    sizeof(PyJobject_Object),                 /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor) pyjobject_dealloc,           /* tp_dealloc */
    0,                                        /* tp_print */
    (getattrfunc) pyjobject_getattr,          /* tp_getattr */
    (setattrfunc) pyjobject_setattr,          /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    (hashfunc) pyjobject_hash,                /* tp_hash  */
    0,                                        /* tp_call */
    (reprfunc) pyjobject_str,                 /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
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
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
