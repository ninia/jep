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

#if STDC_HEADERS
# include <stdio.h>
#endif

#include "pyembed.h"
#include "pyjobject.h"
#include "pyjarray.h"
#include "util.h"


static PyThreadState *mainThreadState = NULL;
static PyObject      *modDict         = NULL;

static PyObject* pyembed_findclass(PyObject*, PyObject*);
static PyObject* pyembed_forname(PyObject*, PyObject*);

// ClassLoader.loadClass
static jmethodID loadClassMethod = 0;

static struct PyMethodDef jep_methods[] = {
    { "findClass",
      pyembed_findclass,
      METH_VARARGS,
      "Find and instantiate a system class, somewhat faster than forName." },
    
    { "forName",
      pyembed_forname,
      METH_VARARGS,
      "Find and return a jclass object using the supplied ClassLoader." },

    { "jarray",
      pyjarray_new_v,
      METH_VARARGS,
      "Create a new primitive array in Java.\n"
      "Accepts:\n"
      "(size, type _ID, [0]) || "
      "(size, JCHAR_ID, [string value] || "
      "(size, jobject) || "
      "(size, str) || "
      "(size, jarray)"
    },
    
    { NULL, NULL }
};


static PyObject* initjep(void) {
    PyObject *modjep;
    
    PyImport_AddModule("jep");
    Py_InitModule((char *) "jep", jep_methods);
    modjep = PyImport_ImportModule("jep");
    if(modjep == NULL)
        printf("WARNING: couldn't import module jep.\n");
    else {
#ifdef VERSION
        PyModule_AddStringConstant(modjep, "VERSION", VERSION);
#endif
        
        // stuff for making new pyjarray objects
        PyModule_AddIntConstant(modjep, "JBOOLEAN_ID", JBOOLEAN_ID);
        PyModule_AddIntConstant(modjep, "JINT_ID", JINT_ID);
        PyModule_AddIntConstant(modjep, "JLONG_ID", JLONG_ID);
        PyModule_AddIntConstant(modjep, "JSTRING_ID", JSTRING_ID);
        PyModule_AddIntConstant(modjep, "JDOUBLE_ID", JDOUBLE_ID);
        PyModule_AddIntConstant(modjep, "JSHORT_ID", JSHORT_ID);
        PyModule_AddIntConstant(modjep, "JFLOAT_ID", JFLOAT_ID);
        PyModule_AddIntConstant(modjep, "JCHAR_ID", JCHAR_ID);
        PyModule_AddIntConstant(modjep, "JBYTE_ID", JBYTE_ID);
    }

    return modjep;
}


// you should hold the global interpreter lock in python before
// calling this function.
// return new reference to modjep
PyObject* get_modjep(jint tstate) {
    PyObject *pyhash, *modjep = NULL;
    
    pyhash = PyLong_FromLong(tstate);
    modjep = PyDict_GetItem(modDict, pyhash);

    Py_INCREF(modjep);
    Py_DECREF(pyhash);
    return modjep;
}


void pyembed_startup(void) {
    if(mainThreadState != NULL)
        return;

    printf("Setting up Python interpreter...\n");
    
    Py_OptimizeFlag = 2;
    
    Py_Initialize();
    PyEval_InitThreads();

    modDict = PyDict_New();

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


jint pyembed_thread_init(JNIEnv *env) {
    PyThreadState *tstate;
    PyObject      *modjep;

    PyEval_AcquireLock();
    Py_NewInterpreter();
    
    tstate = PyEval_SaveThread();
    PyEval_RestoreThread(tstate);

    // store primitive java.lang.Class objects for later use.
    // it's a noop if already done, but to synchronize, have the lock first
    if(!cache_primitive_classes(env))
        printf("WARNING: failed to get primitive class types.\n");

    // init static module
    modjep = initjep();
    
    // keep modjep in dictionary, too
    if(modjep) {
        PyObject *pyhash = PyLong_FromLong((long) tstate);
        PyDict_SetItem(modDict, pyhash, modjep);
        Py_DECREF(pyhash);
    }
    
    PyEval_SaveThread();
    return (jint) tstate;
}


void pyembed_thread_close(jint tstate) {
    PyThreadState *prevThread, *thread;

    thread = (PyThreadState *) tstate;
    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);
    
    if(!thread->frame)
        Py_EndInterpreter(thread);
    else
        printf("WARNING: didn't EndInterpreter, thread still has frame.\n");
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
}


// get object from thread's dictionary. NULL and exception on error.
// returns borrowed reference.
PyObject* pyembed_getthread_object(int pos) {
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
    return PyList_GetItem(tlist, pos);            /* borrowed */
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
    JNIEnv    *env       = NULL;
    char      *name;
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


#define SET_TDICT(env, classLoader)                                             \
{                                                                               \
    PyObject *tdict;                                                            \
    if((tdict = PyThreadState_GetDict()) != NULL) {                             \
        PyObject *key, *tlist, *pyenv, *pycl;                                   \
        jobject   cl = NULL;                                                    \
                                                                                \
        if(classLoader)                                                         \
            cl = (*env)->NewGlobalRef(env, classLoader);                        \
        if(!cl)                                                                 \
            PyErr_Warn(PyExc_Warning, "No classloader.");                       \
                                                                                \
        pyenv = (PyObject *) PyCObject_FromVoidPtr((void *) env, NULL);         \
        pycl  = (PyObject *) PyCObject_FromVoidPtr(cl, NULL);                   \
        key   = PyString_FromString(DICT_KEY);                                  \
        tlist = PyList_New(0);                                                  \
                                                                                \
        PyList_Append(tlist, modjep);        /* takes ownership */              \
        PyList_Append(tlist, pyenv);                                            \
        PyList_Append(tlist, pycl);                                             \
                                                                                \
        PyDict_SetItem(tdict, key, tlist);   /* takes key, tlist ownership */   \
                                                                                \
        Py_DECREF(key);                                                         \
        Py_DECREF(pyenv);                                                       \
        Py_DECREF(tlist);                                                       \
        Py_DECREF(pycl);                                                        \
    }                                                                           \
}


void pyembed_eval(JNIEnv *env,
                  jint tstate,
                  char *str,
                  jobject classLoader) {
    PyThreadState *prevThread, *thread;
    PyObject      *modjep, *main, *dict, *result;
    
    thread = (PyThreadState *) tstate;

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);
    
    modjep = get_modjep(tstate);
    if(modjep == NULL) {
        PyErr_Clear();
        THROW_JEP(env, "Couldn't find modjep object.");
        goto EXIT;
    }
    if(str == NULL)
        goto EXIT;

    // store thread information in thread's dictionary as a list.
    SET_TDICT(env, classLoader);
    
    if(process_py_exception(env, 1))
        goto EXIT;
    
    main = PyImport_AddModule("__main__");                      /* borrowed */
    if(main == NULL) {
        THROW_JEP(env, "Couldn't add module __main__.");
        goto EXIT;
    }
    
    dict = PyModule_GetDict(main);
    Py_INCREF(dict);
    
    result = PyRun_String(str, Py_single_input, dict, dict);    /* new ref */
    
    process_py_exception(env, 1);
    Py_DECREF(dict);
    
    if(result != NULL)
        Py_DECREF(result);

EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
}


// returns 1 if finished, 0 if not, throws exception otherwise
int pyembed_compile_string(JNIEnv *env,
                           jint tstate,
                           char *str) {
    PyThreadState *prevThread, *thread;
    PyObject      *code;
    int            ret = -1;
    
    if(str == NULL)
        return 0;

    thread = (PyThreadState *) tstate;

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);
    
    code = Py_CompileString(str, "<stdin>", Py_single_input);
    
    if(code != NULL) {
        Py_DECREF(code);
        ret = 1;
    }
    else if(PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        ret = 0;
    }
    else
        process_py_exception(env, 0);

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return ret;
}


jobject pyembed_getvalue(JNIEnv *env,
                         jint tstate,
                         char *str) {
    PyThreadState *prevThread, *thread;
    PyObject      *main, *dict, *result;
    jobject        ret = NULL;
    
    thread = (PyThreadState *) tstate;
    if(str == NULL)
        return NULL;
    
    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);
    
    if(process_py_exception(env, 1))
        return NULL;
    
    main = PyImport_AddModule("__main__");                      /* borrowed */
    if(main == NULL) {
        THROW_JEP(env, "Couldn't add module __main__.");
        return NULL;
    }
    
    dict = PyModule_GetDict(main);
    Py_INCREF(dict);
    
    result = PyRun_String(str, Py_eval_input, dict, dict);      /* new ref */
    
    process_py_exception(env, 1);
    Py_DECREF(dict);
    
    if(result == NULL)
        return NULL;
    if(result == Py_None) {
        Py_DECREF(Py_None);
        return NULL;
    }
    
    // convert result to jobject
    if(pyjobject_check(result))
        ret = ((PyJobject_Object *) result)->object;
    else {
        char *tt;
        // i'm lazy, just convert everything else to strings.
        // otherwise we'd have to box primitives...
        PyObject *t = PyObject_Str(result);
        tt = PyString_AsString(t);
        ret = (jobject) (*env)->NewStringUTF(env, (const char *) tt);
        Py_DECREF(t);
    }
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    
    Py_DECREF(result);
    return ret;
}


void pyembed_run(JNIEnv *env,
                 jint tstate,
                 char *file,
                 jobject classLoader) {
    PyThreadState *prevThread, *thread;
    PyObject      *modjep;

    thread = (PyThreadState *) tstate;

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(thread);

    modjep = get_modjep(tstate);
    if(modjep == NULL) {
        PyErr_Clear();
        THROW_JEP(env, "Couldn't find modjep object.");
        goto EXIT;
    }
    
    // store thread information in thread's dictionary as a list.
    SET_TDICT(env, classLoader);

    if(file == NULL)
        goto EXIT;

    if(access(file, R_OK | F_OK) != 0)
        goto EXIT;
    else {
        PyObject *main, *locals, *globals;
        FILE *script = fopen(file, "r");
        
        // some inspiration from pythonrun.c
        
        main = PyImport_AddModule("__main__");    /* borrowed */
        if(main == NULL) {
            THROW_JEP(env, "Couldn't add module __main__.");
            goto EXIT;
        }
        
        globals = PyModule_GetDict(main);
        locals  = PyModule_GetDict(main);
        Py_INCREF(locals);
        Py_INCREF(globals);

        PyRun_File(script, file, Py_file_input, globals, locals);
        
        // c programs inside some java environments may get buffered output
        fflush(stdout);
        fflush(stderr);
        
        fclose(script);
        process_py_exception(env, 1);
        Py_DECREF(locals);
        Py_DECREF(globals);
    }

EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
}


// -------------------------------------------------- set() things

#define GET_COMMON                                      \
    if(name == NULL) {                                  \
        THROW_JEP(env, "name is invalid.");             \
        return;                                         \
    }                                                   \
    thread = (PyThreadState *) tstate;                  \
                                                        \
    PyEval_AcquireLock();                               \
    prevThread = PyThreadState_Swap(thread);            \
                                                        \
    modjep = get_modjep(tstate);                        \
    if(modjep == NULL) {                                \
        PyErr_Clear();                                  \
                                                        \
        THROW_JEP(env, "Couldn't find modjep object."); \
        return;                                         \
    }


void pyembed_setparameter_object(JNIEnv *env,
                                 jint tstate,
                                 const char *name,
                                 jobject value) {
    PyObject      *pyjob, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    // sets thread, modjep
    GET_COMMON;
    
    if(value == NULL) {
        Py_INCREF(Py_None);
        pyjob = Py_None;
    }
    else
        pyjob = pyjobject_new(env, value);
    
    if(pyjob)
        PyModule_AddObject(modjep, (char *) name, pyjob); // steals reference

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_string(JNIEnv *env,
                                 jint tstate,
                                 const char *name,
                                 const char *value) {
    PyObject      *pyvalue, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    // sets thread, modjep
    GET_COMMON;
    
    if(value == NULL) {
        Py_INCREF(Py_None);
        pyvalue = Py_None;
    }
    else
        pyvalue = PyString_FromString(value);
    
    PyModule_AddObject(modjep, (char *) name, pyvalue);  // steals reference

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_int(JNIEnv *env,
                              jint tstate,
                              const char *name,
                              int value) {
    PyObject      *pyvalue, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    // sets thread, modjep
    GET_COMMON;
    
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
                               jint tstate,
                               const char *name,
                               jeplong value) {
    PyObject      *pyvalue, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    // sets thread, modjep
    GET_COMMON;
    
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
                                 jint tstate,
                                 const char *name,
                                 double value) {
    PyObject      *pyvalue, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    // sets thread, modjep
    GET_COMMON;
    
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
                                jint tstate,
                                const char *name,
                                float value) {
    PyObject      *pyvalue, *modjep = NULL;
    PyThreadState *prevThread, *thread;
    
    // sets thread, modjep
    GET_COMMON;
    
    if((pyvalue = PyFloat_FromDouble((double) value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    PyModule_AddObject(modjep, (char *) name, pyvalue); // steals reference
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}
