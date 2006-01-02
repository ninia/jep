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
      "(size, jarray)" },
    
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


void pyembed_startup(void) {
    if(mainThreadState != NULL)
        return;

    Py_OptimizeFlag = 2;
    
    Py_Initialize();
    PyEval_InitThreads();

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


intptr_t pyembed_thread_init(JNIEnv *env, jobject cl) {
    JepThread *jepThread;
    PyObject  *tdict;
    
    if(cl == NULL) {
        THROW_JEP(env, "Invalid Classloader.");
        return 0;
    }
    
    PyEval_AcquireLock();
    Py_NewInterpreter();
    
    jepThread = PyMem_Malloc(sizeof(JepThread));
    if(!jepThread) {
        THROW_JEP(env, "Out of memory.");
        PyEval_ReleaseLock();
        return 0;
    }
    
    jepThread->tstate = PyEval_SaveThread();
    PyEval_RestoreThread(jepThread->tstate);

    // store primitive java.lang.Class objects for later use.
    // it's a noop if already done, but to synchronize, have the lock first
    if(!cache_primitive_classes(env))
        printf("WARNING: failed to get primitive class types.\n");

    // init static module
    jepThread->modjep      = initjep();
    jepThread->env         = env;
    jepThread->classloader = (*env)->NewGlobalRef(env, cl);
    
    if((tdict = PyThreadState_GetDict()) != NULL) {
        PyObject *key, *t;
        
        t   = (PyObject *) PyCObject_FromVoidPtr((void *) jepThread, NULL);
        key = PyString_FromString(DICT_KEY);
        
        PyDict_SetItem(tdict, key, t);   /* takes ownership */
        
        Py_DECREF(key);
        Py_DECREF(t);
    }
    
    PyEval_SaveThread();
    return (intptr_t) jepThread;
}


void pyembed_thread_close(intptr_t _jepThread) {
    PyThreadState *prevThread, *thread;
    JepThread     *jepThread;
    PyObject      *tdict, *key;
    JNIEnv        *env;

    jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        printf("WARNING: thread_close, invalid JepThread pointer.\n");
        return;
    }
    
    env = jepThread->env;
    if(!env) {
        printf("WARNING: thread_close, invalid env pointer.\n");
        return;
    }

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);

    key = PyString_FromString(DICT_KEY);
    if((tdict = PyThreadState_GetDict()) != NULL && key != NULL)
        PyDict_DelItem(tdict, key);
    Py_DECREF(key);
    
    if(jepThread->modjep)
        Py_DECREF(jepThread->modjep);
    if(jepThread->classloader)
        (*env)->DeleteGlobalRef(env, jepThread->classloader);
    
    Py_EndInterpreter(jepThread->tstate);
    
    PyMem_Free(jepThread);
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
}


JNIEnv* pyembed_get_env(void) {
    JavaVM *jvm;
    JNIEnv *env;

    JNI_GetCreatedJavaVMs(&jvm, 1, NULL);
    (*jvm)->AttachCurrentThread(jvm, (void**) &env, NULL);

    return env;
}


// get thread struct when called from internals.
// NULL if not found.
// hold the lock before calling.
JepThread* pyembed_get_jepthread(void) {
    PyObject  *tdict, *t, *key;
    JepThread *ret = NULL;
    
    key = PyString_FromString(DICT_KEY);
    if((tdict = PyThreadState_GetDict()) != NULL && key != NULL) {
        t = PyDict_GetItem(tdict, key); /* borrowed */
        if(t != NULL && !PyErr_Occurred())
            ret = (JepThread*) PyCObject_AsVoidPtr(t);
    }
    
    Py_DECREF(key);
    return ret;
}


static PyObject* pyembed_forname(PyObject *self, PyObject *args) {
    JNIEnv    *env       = NULL;
    char      *name;
    jobject    cl;
    jclass     objclazz;
    jstring    jstr;
    JepThread *jepThread;

    if(!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    jepThread = pyembed_get_jepthread();
    if(!jepThread) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Invalid JepThread pointer.");
        return NULL;
    }
    
    env = jepThread->env;
    cl  = jepThread->classloader;
    
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
    jclass     clazz;
    JepThread *jepThread;
    
    if(!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    jepThread = pyembed_get_jepthread();
    if(!jepThread) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Invalid JepThread pointer.");
        return NULL;
    }
    
    env = jepThread->env;
    
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


void pyembed_eval(JNIEnv *env,
                  intptr_t _jepThread,
                  char *str) {
    PyThreadState    *prevThread, *thread;
    PyObject         *modjep, *main, *dict, *result;
    JepThread        *jepThread;
    
    jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return;
    }

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);
    
    if(str == NULL)
        goto EXIT;

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
    
    // c programs inside some java environments may get buffered output
    fflush(stdout);
    fflush(stderr);
    
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
                           intptr_t _jepThread,
                           char *str) {
    PyThreadState  *prevThread;
    PyObject       *code;
    int             ret = -1;
    JepThread      *jepThread;
    
    jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return 0;
    }
    
    if(str == NULL)
        return 0;

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);
    
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


void pyembed_setloader(JNIEnv *env, intptr_t _jepThread, jobject cl) {
    jobject    oldLoader = NULL;
    JepThread *jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return;
    }
    
    if(!cl)
        return;
    
    oldLoader = jepThread->classloader;
    if(oldLoader)
        (*env)->DeleteGlobalRef(env, oldLoader);
    
    jepThread->classloader = (*env)->NewGlobalRef(env, cl);
}


jobject pyembed_getvalue(JNIEnv *env, intptr_t _jepThread, char *str) {
    PyThreadState  *prevThread;
    PyObject       *main, *dict, *result;
    jobject         ret = NULL;
    JepThread      *jepThread;
    
    jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return NULL;
    }
    
    if(str == NULL)
        return NULL;
    
    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);
    
    if(process_py_exception(env, 1))
        goto EXIT;
    
    main = PyImport_AddModule("__main__");                      /* borrowed */
    if(main == NULL) {
        THROW_JEP(env, "Couldn't add module __main__.");
        goto EXIT;
    }
    
    dict = PyModule_GetDict(main);
    Py_INCREF(dict);
    
    result = PyRun_String(str, Py_eval_input, dict, dict);      /* new ref */
    
    process_py_exception(env, 1);
    Py_DECREF(dict);
    
    if(result == NULL)
        goto EXIT;              /* don't return, need to release GIL */
    if(result == Py_None) {
        Py_DECREF(Py_None);
        goto EXIT;
    }
    
    // convert result to jobject
    if(pyjobject_check(result))
        ret = ((PyJobject_Object *) result)->object;
    else {
        char *tt;
        // TODO i'm lazy, just convert everything else to strings.
        // TODO otherwise we'd have to box primitives...
        PyObject *t = PyObject_Str(result);
        tt = PyString_AsString(t);
        ret = (jobject) (*env)->NewStringUTF(env, (const char *) tt);
        Py_DECREF(t);
    }
    
EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    
    Py_DECREF(result);
    return ret;
}


void pyembed_run(JNIEnv *env,
                 intptr_t _jepThread,
                 char *file) {
    PyThreadState *prevThread;
    JepThread     *jepThread;
    
    jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return;
    }

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);
    
    if(file != NULL) {
        PyObject *main, *locals, *globals;
        
        FILE *script = fopen(file, "r");
        if(!script) {
            THROW_JEP(env, "Couldn't open script file.");
            goto EXIT;
        }
        
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
    JepThread *jepThread;                               \
                                                        \
    jepThread = (JepThread *) _jepThread;               \
    if(!jepThread) {                                    \
        THROW_JEP(env, "Couldn't get thread objects."); \
        return;                                         \
    }                                                   \
                                                        \
    if(name == NULL) {                                  \
        THROW_JEP(env, "name is invalid.");             \
        return;                                         \
    }                                                   \
                                                        \
    PyEval_AcquireLock();                               \
    prevThread = PyThreadState_Swap(jepThread->tstate);


void pyembed_setparameter_object(JNIEnv *env,
                                 intptr_t _jepThread,
                                 const char *name,
                                 jobject value) {
    PyObject      *pyjob;
    PyThreadState *prevThread;
    
    // does common things
    GET_COMMON;
    
    if(value == NULL) {
        Py_INCREF(Py_None);
        pyjob = Py_None;
    }
    else
        pyjob = pyjobject_new(env, value);
    
    if(pyjob) {
        PyModule_AddObject(jepThread->modjep,
                           (char *) name,
                           pyjob); // steals reference
    }

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_string(JNIEnv *env,
                                 intptr_t _jepThread,
                                 const char *name,
                                 const char *value) {
    PyObject      *pyvalue;
    PyThreadState *prevThread;
    
    // does common things
    GET_COMMON;
    
    if(value == NULL) {
        Py_INCREF(Py_None);
        pyvalue = Py_None;
    }
    else
        pyvalue = PyString_FromString(value);
    
    PyModule_AddObject(jepThread->modjep,
                       (char *) name,
                       pyvalue);  // steals reference

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_int(JNIEnv *env,
                              intptr_t _jepThread,
                              const char *name,
                              int value) {
    PyObject      *pyvalue;
    PyThreadState *prevThread;
    
    // does common things
    GET_COMMON;
    
    if((pyvalue = Py_BuildValue("i", value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    PyModule_AddObject(jepThread->modjep,
                       (char *) name,
                       pyvalue); // steals reference
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_long(JNIEnv *env,
                               intptr_t _jepThread,
                               const char *name,
                               jeplong value) {
    PyObject      *pyvalue;
    PyThreadState *prevThread;
    
    // does common things
    GET_COMMON;
    
    if((pyvalue = PyLong_FromLongLong(value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    PyModule_AddObject(jepThread->modjep,
                       (char *) name,
                       pyvalue); // steals reference
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_double(JNIEnv *env,
                                 intptr_t _jepThread,
                                 const char *name,
                                 double value) {
    PyObject      *pyvalue;
    PyThreadState *prevThread;
    
    // does common things
    GET_COMMON;
    
    if((pyvalue = PyFloat_FromDouble(value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    PyModule_AddObject(jepThread->modjep,
                       (char *) name,
                       pyvalue); // steals reference
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_float(JNIEnv *env,
                                intptr_t _jepThread,
                                const char *name,
                                float value) {
    PyObject      *pyvalue;
    PyThreadState *prevThread;
    
    // does common things
    GET_COMMON;
    
    if((pyvalue = PyFloat_FromDouble((double) value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    PyModule_AddObject(jepThread->modjep,
                       (char *) name,
                       pyvalue); // steals reference
    
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}
