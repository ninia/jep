/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/* 
   jep - Java Embedded Python

   Copyright (C) 2004 Mike Johnson

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  


   *****************************************************************************
   This file handles two main things:
   - startup, shutdown of interpreters.
      (those are the pyembed_* functions)
   - setting of parameters
      (the pyembed_set*)

   The really interesting stuff is not here. :-) This file simply makes calls
   to the type definitions for pyjobject, etc.
   *****************************************************************************
*/ 	

#if HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
#endif

#if HAVE_STDLIB_H
# include <stdlib.h>
#else
int putenv(char *string) {
    return -1;
}
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
#include "Python.h"

#include "pyjobject.h"
#include "util.h"


static PyThreadState *mainThreadState = NULL;

static PyObject      *threadDict      = NULL;
static PyObject      *modDict         = NULL;

static const char *DICT_KEY = "jep";
// positions in thread dictionary, list value
static const LIST_MOD_JEP = 0;
static const LIST_ENV     = 1;
static const LIST_CL      = 2;

static PyObject* pyembed_findclass(PyObject*, PyObject*);
static PyObject* pyembed_forname(PyObject*, PyObject*);

// ClassLoader.loadClass
static jmethodID loadClassMethod = 0;

static struct PyMethodDef jep_methods[] = {
    { "findClass",
      pyembed_findclass,
      METH_VARARGS,
      "Find and instantiate a system class, faster than forName." },
    
    { "forName",
      pyembed_forname,
      METH_VARARGS,
      "Find and return a jclass object using the supplied ClassLoader." },
    
    { NULL, NULL }
};


PyThreadState* get_threadstate(const char *hash) {
    PyObject      *pyhash, *pylong, *str;
    PyThreadState *tstate;
    
    if(!hash)
        return NULL;
    
    // need to syncronize this method
    PyEval_AcquireLock();
    
    pyhash = PyString_FromString(hash);
    pylong = PyDict_GetItem(threadDict, pyhash);
    
    if(!pylong || pylong == Py_None)
        return NULL;
    
    Py_INCREF(pylong);
    
    if(PyLong_Check(pylong))
        tstate = (PyThreadState *) PyLong_AsLong(pylong);
    
    Py_DECREF(pylong);
    Py_DECREF(pyhash);
    PyEval_ReleaseLock();
    return tstate;
}


// you should *not* hold the global interpreter lock in python before
// calling this function.
// return new reference to modjep
PyObject* get_modjep(const char *hash) {
    PyObject *pyhash, *modjep = NULL;
    
    PyEval_AcquireLock();

    pyhash = PyString_FromString(hash);
    modjep = PyDict_GetItem(modDict, pyhash);

    Py_INCREF(modjep);
    Py_DECREF(pyhash);
    PyEval_ReleaseLock();
    return modjep;
}


void pyembed_startup(void) {
    if(mainThreadState != NULL)
        return;

    printf("Setting up Python interpreter...\n");
    
    putenv("PYTHONOPTIMIZE=1");
    
    Py_Initialize();
    PyEval_InitThreads();

    threadDict = PyDict_New();
    modDict    = PyDict_New();

    // save a pointer to the main PyThreadState object
    mainThreadState = PyThreadState_Get();

    PyEval_ReleaseLock();
}


void pyembed_shutdown(void) {
    printf("Shutting down Python...\n");
    PyEval_AcquireLock();
    PyThreadState_Swap(mainThreadState);
    Py_Finalize();
}


void pyembed_thread_init(JNIEnv *env, const char *hash) {
    PyThreadState *tstate;
    PyObject      *pyhash, *pytstate, *modjep;

    PyEval_AcquireLock();
    Py_NewInterpreter();
    
    tstate = PyEval_SaveThread();
    PyEval_RestoreThread(tstate);

    // store primitive java.lang.Class objects for later use.
    // it's a noop if already done, but to synchronize, have the lock first
    if(!cache_primitive_classes(env))
        printf("WARNING: failed to get primitive class types.\n");

    // store to dictionary with thread's hashcode
    pytstate = PyLong_FromLong((long) tstate);
    pyhash   = PyString_FromString(hash);
    PyDict_SetItem(threadDict, pyhash, pytstate);

    // keep modjep in dictionary, too
    PyImport_AddModule("jep");
    Py_InitModule((char *) "jep", jep_methods);
    modjep = PyImport_ImportModule("jep");
    if(modjep == NULL)
        printf("WARNING: couldn't import module jep.\n");
    else
        PyDict_SetItem(modDict, pyhash, modjep);
    
    Py_DECREF(pyhash);
    Py_DECREF(pytstate);
    PyEval_SaveThread();
}


void pyembed_thread_close(const char *hash) {
    PyThreadState *prevThread, *thread;
    PyObject      *pyhash;

    thread = get_threadstate(hash);
    if(thread == NULL)
        return;

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);
    
    if(!thread->frame)
        Py_EndInterpreter(thread);
    else
        printf("WARNING: didn't EndInterpreter, thread still has frame.\n");
    
    pyhash = PyString_FromString(hash);
    PyDict_DelItem(threadDict, pyhash);
    PyDict_DelItem(modDict, pyhash);
    Py_DECREF(pyhash);

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
}


// get object from thread's dictionary. NULL and exception on error.
// returns borrowed reference.
static PyObject* pyembed_getthread_object(int pos) {
    PyObject  *tdict, *tlist, *key;
    tdict = tlist = key = NULL;
    
    key = PyString_FromString(DICT_KEY);
    if((tdict = PyThreadState_GetDict()) == NULL) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Couldn't get thread dictionary.");
        return NULL;
    }
    
    tlist = PyDict_GetItem(tdict, key);           /* borrowed */
    if(!tlist || !PyList_Check(tlist)) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Couldn't get thread information.");
        return NULL;
    }
    
    Py_DECREF(key);
    return PyList_GetItem(tlist, pos);           /* borrowed */
}


// true of module has attribute
int pyembed_modjep_has(PyObject *name) {
    PyObject *modjep;
    
    modjep = pyembed_getthread_object(LIST_MOD_JEP);
    if(PyErr_Occurred()) {
        PyErr_Print();
        return 0;
    }
    if(!modjep)
        return 0;
    
    if(PyObject_HasAttr(modjep, name))
        return 1;
    return 0;
}


// add an object to modjep from caller.
// steals reference.
// may set an exception, returns 0 on error.
int pyembed_modjep_add(char *name, PyObject *obj) {
    PyObject *modjep;
    
    modjep = pyembed_getthread_object(LIST_MOD_JEP);
    if(PyErr_Occurred()) {
        PyErr_Print();
        return 0;
    }
    if(!modjep)
        return 0;
    
    PyModule_AddObject(modjep, name, obj);   /* steals ref */
    if(PyErr_Occurred()) {
        PyErr_Print();
        return 0;
    }
    return 1;
}


// get name from modjep.
// returns new reference.
PyObject* pyembed_modjep_get(PyObject *name) {
    PyObject *modjep;
    
    modjep = pyembed_getthread_object(LIST_MOD_JEP);
    if(PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
    if(!modjep)
        return NULL;
    
    return PyObject_GetAttr(modjep, name);
}


static PyObject* pyembed_forname(PyObject *self, PyObject *args) {
    PyObject  *ret       = NULL;
    JNIEnv    *env       = NULL;
    char      *name, *p;
    PyObject  *_env      = NULL;
    PyObject  *_cl       = NULL;
    jobject    cl;
    jclass     objclazz;
    jstring    jstr;

    if(!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    _env = pyembed_getthread_object(LIST_ENV);
    if(!_env || !PyCObject_Check(_env)) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_ValueError, "Invalid env pointer.");
        return NULL;
    }
    
    env = (JNIEnv *) PyCObject_AsVoidPtr(_env);
    if(!env) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_ValueError,
                            "Invalid env pointer, AsVoidPtr returned NULL.");
        return NULL;
    }
    
    _cl = pyembed_getthread_object(LIST_CL);
    if(!_cl || !PyCObject_Check(_cl)) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_ValueError, "Invalid classloader.");
        return NULL;
    }
    
    cl = (jobject) PyCObject_AsVoidPtr(_cl);
    if(!cl) {
        if(!PyErr_Occurred()) {
            PyErr_SetString(PyExc_ValueError,
                            "Invalid classloader, AsVoidPtr returned NULL.");
        }
        return NULL;
    }
    // return NULL;

    if(loadClassMethod == 0) {
        jobject clazz;
        
        clazz = (*env)->GetObjectClass(env, cl);
        if(process_java_exception(env) || !clazz)
            return NULL;

        loadClassMethod =
            (*env)->GetMethodID(env,
                                clazz,
                                "loadClass",
                                "(Ljava/lang/String;)Ljava/lang/Class;");
        
        if(process_java_exception(env) || !loadClassMethod) {
            (*env)->DeleteLocalRef(env, clazz);
            return NULL;
        }

        (*env)->DeleteLocalRef(env, clazz);
    }
    
    jstr = (*env)->NewStringUTF(env, (const char *) name);
    if(process_java_exception(env) || !jstr)
        return NULL;
    
    objclazz = (jclass) (*env)->CallObjectMethod(env,
                                                 cl,
                                                 loadClassMethod,
                                                 jstr);
    if(process_java_exception(env) || !objclazz)
        return NULL;
    
    return (PyObject *) pyjobject_new_class(env, objclazz);
}


static PyObject* pyembed_findclass(PyObject *self, PyObject *args) {
    PyObject  *ret       = NULL;
    JNIEnv    *env       = NULL;
    char      *name, *p;
    PyObject  *_env      = NULL;
    jclass     clazz;
    
    if(!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    _env = pyembed_getthread_object(LIST_ENV);
    if(!_env || !PyCObject_Check(_env)) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_ValueError, "Invalid env pointer.");
        return NULL;
    }
    
    env = (JNIEnv *) PyCObject_AsVoidPtr((PyObject *) _env);
    if(!env) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_ValueError,
                            "Invalid env pointer, AsVoidPtr returned NULL.");
        return NULL;
    }
    
    // replace '.' with '/'
    // i'm told this is okay to do with unicode.
    for(p = name; *p != '\0'; p++) {
        if(*p == '.')
            *p = '/';
    }
    
    clazz = (*env)->FindClass(env, name);
    if(process_java_exception(env))
        return NULL;
    
    return (PyObject *) pyjobject_new_class(env, clazz);
}


void pyembed_run(JNIEnv *env,
                 const char *hash,
                 char *file,
                 jobject classLoader) {
    int ret = 0;
    PyThreadState *prevThread, *thread;
    PyObject      *modjep, *tdict;

    thread = get_threadstate(hash);
    if(thread == NULL) {
        PyErr_Clear();

        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't get threadstate.");
        return;
    }
    modjep = get_modjep(hash);
    if(modjep == NULL) {
        PyErr_Clear();
        
        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't find modjep object.");
        return;
    }
    
    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);
    
    // store thread information in thread's dictionary as a list.
    if((tdict = PyThreadState_GetDict()) != NULL) {
        PyObject *key, *tlist, *pyenv, *pycl;
        jobject   cl = NULL;

        if(classLoader)
            cl = (*env)->NewGlobalRef(env, classLoader);
        if(!cl)
            PyErr_Warn(PyExc_Warning, "No classloader.");
        
        pyenv = (PyObject *) PyCObject_FromVoidPtr(env, NULL);
        pycl  = (PyObject *) PyCObject_FromVoidPtr(cl, NULL);
        key   = PyString_FromString(DICT_KEY);
        tlist = PyList_New(0);
        
        PyList_Append(tlist, modjep);        /* takes ownership */
        PyList_Append(tlist, pyenv);
        PyList_Append(tlist, pycl);
        
        PyDict_SetItem(tdict, key, tlist);   /* takes key, tlist ownership */
        
        Py_DECREF(key);
        Py_DECREF(pyenv);
        Py_DECREF(tlist);
        Py_DECREF(pycl);
    }
    
    if(file == NULL)
        return;

    if(access(file, R_OK | F_OK) != 0)
        return;
    else {
        PyObject *main, *dict;
        FILE *script = fopen(file, "r");
        
        // some inspiration/code from pythonrun.c
        
        main = PyImport_AddModule("__main__");    /* must be borrowed */
        if(main == NULL)
            return; // TODO throw exception
        
        dict = PyModule_GetDict(main);
        Py_INCREF(dict);
        if(PyDict_GetItemString(dict, "__file__") == NULL) {
            PyObject *f = PyString_FromString(file);
            
            if(f == NULL)
                return; // TODO
            if(PyDict_SetItemString(dict, "__file__", f) < 0) {
                Py_DECREF(f);
                return; // TODO
            }
            Py_DECREF(f);
        }

        PyRun_File(script, file, Py_file_input, dict, dict);
        
        process_py_exception(env, 1);
        Py_DECREF(dict);
    }

    // PyEval_SaveThread();
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
}


// create a new object on the main interpreter so i can be
// shared between threads/calls/sub-interpreters.
// you should hold the interpreter lock before calling.
PyThreadState* pyembed_mainthread_swap(void) {
    return PyThreadState_Swap(mainThreadState);
}


PyThreadState* pyembed_swap_thread(PyThreadState *tstate) {
    return PyThreadState_Swap(tstate);
}


// -------------------------------------------------- set() things

void pyembed_setparameter_object(JNIEnv *env,
                                 const char *hash,
                                 const char *name,
                                 jobject value) {
    PyObject      *pyjob, *modjep = NULL;
    PyThreadState *prevThread, *thread;

    if(name == NULL || value == NULL) {
        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "name or value is null.");
        return;
        return;
    }

    thread = get_threadstate(hash);
    if(thread == NULL) {
        PyErr_Clear();

        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't get threadstate.");
        return;
    }

    modjep = get_modjep(hash);
    if(modjep == NULL) {
        PyErr_Clear();
        
        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't find modjep object.");
        return;
    }

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);

    pyjob   = pyjobject_new(env, value);
    if(pyjob)
        PyModule_AddObject(modjep, (char *) name, pyjob); // steals reference

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_string(JNIEnv *env,
                                 const char *hash,
                                 const char *name,
                                 const char *value) {
    PyObject      *pyvalue, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    if(name == NULL || value == NULL)
        return;
    
    thread = get_threadstate(hash);
    if(thread == NULL) {
        PyErr_Clear();

        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't get threadstate.");
        return;
    }

    modjep = get_modjep(hash);
    if(modjep == NULL) {
        PyErr_Clear();
        
        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't find modjep object.");
        return;
    }
    
    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);

    pyvalue = PyString_FromString(value);
    PyModule_AddObject(modjep, (char *) name, pyvalue);  // steals reference

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_int(JNIEnv *env,
                              const char *hash,
                              const char *name,
                              int value) {
    PyObject      *pyvalue, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    if(name == NULL)
        return;

    thread = get_threadstate(hash);
    if(thread == NULL) {
        PyErr_Clear();

        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't get threadstate.");
        return;
    }

    modjep = get_modjep(hash);
    if(modjep == NULL) {
        PyErr_Clear();
        
        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't find modjep object.");
        return;
    }
    
    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);

    if((pyvalue = Py_BuildValue("i", value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    PyModule_AddObject(modjep, (char *) name, pyvalue); // steals reference
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_long(JNIEnv *env,
                              const char *hash,
                              const char *name,
                              long long value) {
    PyObject      *pyvalue, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    if(name == NULL)
        return;

    thread = get_threadstate(hash);
    if(thread == NULL) {
        PyErr_Clear();

        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't get threadstate.");
        return;
    }

    modjep = get_modjep(hash);
    if(modjep == NULL) {
        PyErr_Clear();
        
        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't find modjep object.");
        return;
    }
    
    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);

    if((pyvalue = PyLong_FromLongLong(value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    PyModule_AddObject(modjep, (char *) name, pyvalue); // steals reference
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_double(JNIEnv *env,
                                 const char *hash,
                                 const char *name,
                                 double value) {
    PyObject      *pyvalue, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    if(name == NULL)
        return;

    thread = get_threadstate(hash);
    if(thread == NULL) {
        PyErr_Clear();

        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't get threadstate.");
        return;
    }

    modjep = get_modjep(hash);
    if(modjep == NULL) {
        PyErr_Clear();
        
        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't find modjep object.");
        return;
    }
    
    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);

    if((pyvalue = PyFloat_FromDouble(value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    PyModule_AddObject(modjep, (char *) name, pyvalue); // steals reference
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_float(JNIEnv *env,
                                const char *hash,
                                const char *name,
                                float value) {
    PyObject      *pyvalue, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    if(name == NULL)
        return;

    thread = get_threadstate(hash);
    if(thread == NULL) {
        PyErr_Clear();

        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't get threadstate.");
        return;
    }

    modjep = get_modjep(hash);
    if(modjep == NULL) {
        PyErr_Clear();
        
        jclass clazz = (*env)->FindClass(env,
                                         JEPEXCEPTION);
        if(clazz != NULL)
            (*env)->ThrowNew(env, clazz, "Couldn't find modjep object.");
        return;
    }
    
    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);

    if((pyvalue = PyFloat_FromDouble((double) value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    PyModule_AddObject(modjep, (char *) name, pyvalue); // steals reference
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}
