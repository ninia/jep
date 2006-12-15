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
static PyObject* pyembed_jimport(PyObject*, PyObject*);
static PyObject* pyembed_set_print_stack(PyObject*, PyObject*);


// ClassLoader.loadClass
static jmethodID loadClassMethod = 0;

// jep.ClassList.get()
static jmethodID getClassListMethod = 0;

// Integer(int)
static jmethodID integerIConstructor = 0;

// Long(long)
static jmethodID longJConstructor = 0;

// Float(float)
static jmethodID floatFConstructor = 0;

// Boolean(boolean)
static jmethodID booleanBConstructor = 0;

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

    { "jimport",
      pyembed_jimport,
      METH_VARARGS,
      "Same definition as the standard __import__." },

    { "printStack",
      pyembed_set_print_stack,
      METH_VARARGS,
      "Turn on printing of stack traces (True|False)" },

    { NULL, NULL }
};


static struct PyMethodDef noop_methods[] = {
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
    jepThread->printStack  = 0;

    // now, add custom import function to builtin module

    // i did have a whole crap load of code to do this from C but it
    // didn't work. i found a PEP that said it wasn't possible, then
    // Guido said they were wrong. *shrug*. this is my work-around.

    PyRun_SimpleString("import jep\n");
    PyRun_SimpleString("__builtins__.__import__ = jep.jimport\n");

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


// used by _jimport and _forname
#define LOAD_CLASS_METHOD(env, cl)                                          \
{                                                                           \
    if(loadClassMethod == 0) {                                              \
        jobject clazz;                                                      \
                                                                            \
        clazz = (*env)->GetObjectClass(env, cl);                            \
        if(process_java_exception(env) || !clazz)                           \
            return NULL;                                                    \
                                                                            \
        loadClassMethod =                                                   \
            (*env)->GetMethodID(env,                                        \
                                clazz,                                      \
                                "loadClass",                                \
                                "(Ljava/lang/String;)Ljava/lang/Class;");   \
                                                                            \
        if(process_java_exception(env) || !loadClassMethod) {               \
            (*env)->DeleteLocalRef(env, clazz);                             \
            return NULL;                                                    \
        }                                                                   \
                                                                            \
        (*env)->DeleteLocalRef(env, clazz);                                 \
    }                                                                       \
}


static PyObject* pyembed_set_print_stack(PyObject *self, PyObject *args) {
    JepThread *jepThread;
    JNIEnv    *env   = NULL;
    char      *print = 0;

	if(!PyArg_ParseTuple(args, "b:setPrintStack", &print))
        return NULL;

    jepThread = pyembed_get_jepthread();
    if(!jepThread) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Invalid JepThread pointer.");
        return NULL;
    }

    if(print == 0)
        jepThread->printStack = 0;
    else
        jepThread->printStack = 1;

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject* pyembed_jimport(PyObject *self, PyObject *args) {
    JNIEnv       *env        = NULL;
    jstring       jname;
    jclass        clazz;
    jobject       cl;
    JepThread    *jepThread;
    int           len, i;
    jobjectArray  jar;

	char         *name;
    PyObject     *module     = NULL;
	PyObject     *globals    = NULL;
	PyObject     *locals     = NULL;
	PyObject     *fromlist   = NULL;
    PyObject     *ret        = NULL;

	if(!PyArg_ParseTuple(args,
                         "s|OOO:__import__",
                         &name, &globals, &locals, &fromlist))
		return NULL;

    // try to let python handle first
    ret = PyImport_ImportModuleEx(name, globals, locals, fromlist);
    if(ret != NULL)
        return ret;
    PyErr_Clear();

    jepThread = pyembed_get_jepthread();
    if(!jepThread) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Invalid JepThread pointer.");
        return NULL;
    }
    
    env = jepThread->env;
    cl  = jepThread->classloader;

    LOAD_CLASS_METHOD(env, cl);

    clazz = (*env)->FindClass(env, "jep/ClassList");
    if(process_java_exception(env) || !clazz)
        return NULL;

    if(getClassListMethod == 0) {
        getClassListMethod =
            (*env)->GetStaticMethodID(env,
                                      clazz,
                                      "get",
                                      "(Ljava/lang/String;)[Ljava/lang/String;");
        
        if(process_java_exception(env) || !getClassListMethod)
            return NULL;
    }

    jname = (*env)->NewStringUTF(env, name);
    jar = (jobjectArray) (*env)->CallStaticObjectMethod(
        env,
        clazz,
        getClassListMethod,
        jname);

    if(process_java_exception(env) || !jar)
        return NULL;

    len = (*env)->GetArrayLength(env, jar);

    // it doesn't really matter what the module is named, python will
    // use it as it wants.
    if(len > 0)
            module = PyImport_AddModule(name);

    for(i = 0; i < len; i++) {
        jstring   member     = NULL;
        jclass    objclazz   = NULL;
        PyObject *pclass     = NULL;
        PyObject *memberList = NULL;

        member = (*env)->GetObjectArrayElement(env, jar, i);
        if(process_java_exception(env) || !member) {
            (*env)->DeleteLocalRef(env, member);
            continue;
        }

        // split member into list, member looks like "java.lang.System"
        {
            const char *cmember;
            PyObject   *pymember;
            
            cmember    = jstring2char(env, member);
            pymember   = PyString_FromString(cmember);
            memberList = PyObject_CallMethod(pymember, "split", "s", ".");

            Py_DECREF(pymember);
            (*env)->ReleaseStringUTFChars(env, member, cmember);

            if(!PyList_Check(memberList) || PyErr_Occurred()) {
                THROW_JEP(env, "Couldn't split member name");
                (*env)->DeleteLocalRef(env, member);
                continue;
            }
        }

        // make sure our class name is in the fromlist (a tuple, oddly)
        if(PyTuple_Check(fromlist) &&
           PyString_AsString(PyTuple_GET_ITEM(fromlist, 0))[0] != '*') {

            PyObject   *pymember;
            int         found, i, len;

            pymember = PyList_GET_ITEM(
                memberList,
                PyList_GET_SIZE(memberList) - 1); /* last one */

            found = 0;
            len   = PyTuple_GET_SIZE(fromlist);
            for(i = 0; i < len && found == 0; i++) {
                PyObject *el = PyTuple_GET_ITEM(fromlist, i);
                if(PyObject_Compare(pymember, el) == 0)
                    found = 1;
            }

            if(found == 0) {
                (*env)->DeleteLocalRef(env, member);
                Py_DECREF(memberList);
                continue;          /* don't process this class name */
            }
        }

        objclazz = (jclass) (*env)->CallObjectMethod(env,
                                                     cl,
                                                     loadClassMethod,
                                                     member);
        if((*env)->ExceptionOccurred(env) != NULL || !objclazz) {
            // this error we ignore
            // (*env)->ExceptionDescribe(env);
            Py_DECREF(memberList);
            (*env)->DeleteLocalRef(env, member);
            (*env)->ExceptionClear(env);
            continue;
        }

        // make a new class object
        pclass = (PyObject *) pyjobject_new_class(env, objclazz);
        if(pclass) {
            PyModule_AddObject(
                module,
                PyString_AsString(
                    PyList_GET_ITEM(
                        memberList,
                        PyList_GET_SIZE(memberList) - 1)), /* classname */
                pclass);                                   /* steals ref */
        }

        Py_DECREF(memberList);
        (*env)->DeleteLocalRef(env, member);
    }

    if(module == NULL)
        PyErr_Format(PyExc_ImportError, "No module %s found", name);
    else
        Py_INCREF(module);
    return module;
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
    
    LOAD_CLASS_METHOD(env, cl);
    
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


// returns new reference to globals dict
// hold lock before calling.
// null on error
PyObject* pyembed_get_globals(JNIEnv *env) {
    PyObject *main, *globals;

    main = PyImport_AddModule("__main__");    /* borrowed */
    if(main == NULL) {
        THROW_JEP(env, "Couldn't add module __main__.");
        return NULL;
    }

    globals = PyModule_GetDict(main);
    Py_INCREF(globals);

    return globals;
}


intptr_t pyembed_create_module(JNIEnv *env,
                               intptr_t _jepThread,
                               char *str) {
    PyThreadState  *prevThread;
    PyObject       *module;
    JepThread      *jepThread;
    intptr_t        ret;
    PyObject       *globals;

    jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return 0;
    }

    if(str == NULL)
        return 0;

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);

    globals = pyembed_get_globals(env);
    if(!globals)
        goto EXIT;

    if(PyImport_AddModule(str) == NULL || process_py_exception(env, 1))
        goto EXIT;

    Py_InitModule(str, noop_methods);
    module = PyImport_ImportModuleEx(str, globals, globals, NULL);

    PyDict_SetItem(globals,
                   PyString_FromString(str),
                   module);     /* steals */

    if(process_py_exception(env, 0) || module == NULL)
        ret = 0;
    else
        ret = (intptr_t) module;

EXIT:
    if(globals)
        Py_DECREF(globals);

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();

    return ret;
}


intptr_t pyembed_create_module_on(JNIEnv *env,
                                  intptr_t _jepThread,
                                  intptr_t _onModule,
                                  char *str) {
    PyThreadState  *prevThread;
    PyObject       *module, *onModule;
    JepThread      *jepThread;
    intptr_t        ret;
    PyObject       *globals;

    jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return 0;
    }

    if(str == NULL)
        return 0;

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);

    onModule = (PyObject *) _onModule;
    if(!PyModule_Check(onModule)) {
        THROW_JEP(env, "Invalid onModule.");
        goto EXIT;
    }

    globals = PyModule_GetDict(onModule);
    Py_INCREF(globals);

    if(PyImport_AddModule(str) == NULL || process_py_exception(env, 1))
        goto EXIT;

    Py_InitModule(str, noop_methods);
    module = PyImport_ImportModuleEx(str, globals, globals, NULL);

    PyDict_SetItem(globals,
                   PyString_FromString(str),
                   module);     /* steals */

    if(process_py_exception(env, 0) || module == NULL)
        ret = 0;
    else
        ret = (intptr_t) module;

EXIT:
    if(globals)
        Py_DECREF(globals);

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


// convert pyobject to boxed java value
jobject pyembed_box_py(JNIEnv *env, PyObject *result) {
    if(pyjclass_check(result))
        return ((PyJobject_Object *) result)->clazz;

    if(pyjobject_check(result))
        return ((PyJobject_Object *) result)->object;

    if(PyString_Check(result)) {
        char *s = PyString_AS_STRING(result);
        return (*env)->NewStringUTF(env, (const char *) s);
    }

    if(PyBool_Check(result)) {
        jclass clazz;
        jboolean b = JNI_FALSE;
        if(result == Py_True)
            b = JNI_TRUE;

        clazz = (*env)->FindClass(env, "java/lang/Boolean");

        if(booleanBConstructor == 0) {
            booleanBConstructor = (*env)->GetMethodID(env,
                                                      clazz,
                                                      "<init>",
                                                      "(Z)V");
        }

        if(!process_java_exception(env) && booleanBConstructor)
            return (*env)->NewObject(env, clazz, booleanBConstructor, b);
        else
            return NULL;
    }

    if(PyInt_Check(result)) {
        jclass clazz;
        jint i = PyInt_AS_LONG(result);

        clazz = (*env)->FindClass(env, "java/lang/Integer");

        if(integerIConstructor == 0) {
            integerIConstructor = (*env)->GetMethodID(env,
                                                      clazz,
                                                      "<init>",
                                                      "(I)V");
        }

        if(!process_java_exception(env) && integerIConstructor)
            return (*env)->NewObject(env, clazz, integerIConstructor, i);
        else
            return NULL;
    }

    if(PyLong_Check(result)) {
        jclass clazz;
        jeplong i = PyLong_AsLongLong(result);

        clazz = (*env)->FindClass(env, "java/lang/Long");

        if(longJConstructor == 0) {
            longJConstructor = (*env)->GetMethodID(env,
                                                   clazz,
                                                   "<init>",
                                                   "(J)V");
        }

        if(!process_java_exception(env) && longJConstructor)
            return (*env)->NewObject(env, clazz, longJConstructor, i);
        else
            return NULL;
    }

    if(PyFloat_Check(result)) {
        jclass clazz;

        // causes precision loss. python's float type sucks. *shrugs*
        jfloat f = PyFloat_AS_DOUBLE(result);

        clazz = (*env)->FindClass(env, "java/lang/Float");

        if(floatFConstructor == 0) {
            floatFConstructor = (*env)->GetMethodID(env,
                                                    clazz,
                                                    "<init>",
                                                    "(F)V");
        }

        if(!process_java_exception(env) && floatFConstructor)
            return (*env)->NewObject(env, clazz, floatFConstructor, f);
        else
            return NULL;
    }

    // convert everything else to string
    {
        jobject ret;
        char *tt;
        PyObject *t = PyObject_Str(result);
        tt = PyString_AsString(t);
        ret = (jobject) (*env)->NewStringUTF(env, (const char *) tt);
        Py_DECREF(t);

        return ret;
    }
}


jobject pyembed_getvalue_on(JNIEnv *env,
                            intptr_t _jepThread,
                            intptr_t _onModule,
                            char *str) {
    PyThreadState  *prevThread;
    PyObject       *dict, *result, *onModule;
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

    onModule = (PyObject *) _onModule;
    if(!PyModule_Check(onModule)) {
        THROW_JEP(env, "pyembed_getvalue_on: Invalid onModule.");
        goto EXIT;
    }
    
    dict = PyModule_GetDict(onModule);
    Py_INCREF(dict);
    
    result = PyRun_String(str, Py_eval_input, dict, dict);      /* new ref */
    
    process_py_exception(env, 1);
    Py_DECREF(dict);
    
    if(result == NULL)
        goto EXIT;              /* don't return, need to release GIL */
    if(result == Py_None)
        goto EXIT;
    
    // convert results to jobject
    ret = pyembed_box_py(env, result);
    
EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();

    if(result != NULL)
        Py_DECREF(result);
    return ret;
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
    if(result == Py_None)
        goto EXIT;
    
    // convert results to jobject
    ret = pyembed_box_py(env, result);
    
EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();

    if(result != NULL)
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
        PyObject *globals;
        
        FILE *script = fopen(file, "r");
        if(!script) {
            THROW_JEP(env, "Couldn't open script file.");
            goto EXIT;
        }
        
        globals = pyembed_get_globals(env);
        if(!globals)
            goto EXIT;

        PyRun_File(script, file, Py_file_input, globals, globals);
        
        // c programs inside some java environments may get buffered output
        fflush(stdout);
        fflush(stderr);
        
        fclose(script);
        process_py_exception(env, 1);
        Py_DECREF(globals);
    }

EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
}


// -------------------------------------------------- set() things

#define GET_COMMON                                                  \
    JepThread *jepThread;                                           \
    PyObject  *globals;                                             \
                                                                    \
    jepThread = (JepThread *) _jepThread;                           \
    if(!jepThread) {                                                \
        THROW_JEP(env, "Couldn't get thread objects.");             \
        return;                                                     \
    }                                                               \
                                                                    \
    if(name == NULL) {                                              \
        THROW_JEP(env, "name is invalid.");                         \
        return;                                                     \
    }                                                               \
                                                                    \
    PyEval_AcquireLock();                                           \
    prevThread = PyThreadState_Swap(jepThread->tstate);             \
                                                                    \
    globals = pyembed_get_globals(env);                             \
    if(!globals) {                                                  \
        THROW_JEP(env, "pyembed_set: Couldn't get globals dict.");  \
        return;                                                     \
    }                                                               \
                                                                    \
    pymodule = NULL;                                                \
    if(module != 0)                                                 \
        pymodule = (PyObject *) module;



void pyembed_setparameter_object(JNIEnv *env,
                                 intptr_t _jepThread,
                                 intptr_t module,
                                 const char *name,
                                 jobject value) {
    PyObject      *pyjob;
    PyThreadState *prevThread;
    PyObject      *pymodule;
    
    // does common things
    GET_COMMON;
    
    if(value == NULL) {
        Py_INCREF(Py_None);
        pyjob = Py_None;
    }
    else
        pyjob = pyjobject_new(env, value);
    
    if(pyjob) {
        if(pymodule == NULL) {
            PyDict_SetItem(globals,
                           PyString_FromString(name),
                           pyjob); // steals reference
        }
        else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyjob); // steals reference
        }
    }

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_class(JNIEnv *env,
                                intptr_t _jepThread,
                                intptr_t module,
                                const char *name,
                                jclass value) {
    PyObject      *pyjob;
    PyThreadState *prevThread;
    PyObject      *pymodule;
    
    // does common things
    GET_COMMON;
    
    if(value == NULL) {
        Py_INCREF(Py_None);
        pyjob = Py_None;
    }
    else
        pyjob = pyjobject_new_class(env, value);
    
    if(pyjob) {
        if(pymodule == NULL) {
            PyDict_SetItem(globals,
                           PyString_FromString(name),
                           pyjob); // steals reference
        }
        else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyjob); // steals reference
        }
    }

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_string(JNIEnv *env,
                                 intptr_t _jepThread,
                                 intptr_t module,
                                 const char *name,
                                 const char *value) {
    PyObject      *pyvalue;
    PyThreadState *prevThread;
    PyObject      *pymodule;
    
    // does common things
    GET_COMMON;

    if(value == NULL) {
        Py_INCREF(Py_None);
        pyvalue = Py_None;
    }
    else
        pyvalue = PyString_FromString(value);

    if(pymodule == NULL) {
        PyDict_SetItem(globals,
                       PyString_FromString(name),
                       pyvalue);  // steals reference
    }
    else {
        PyModule_AddObject(pymodule,
                           (char *) name,
                           pyvalue); // steals reference
    }

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_int(JNIEnv *env,
                              intptr_t _jepThread,
                              intptr_t module,
                              const char *name,
                              int value) {
    PyObject      *pyvalue;
    PyThreadState *prevThread;
    PyObject      *pymodule;
    
    // does common things
    GET_COMMON;
    
    if((pyvalue = Py_BuildValue("i", value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    if(pymodule == NULL) {
        PyDict_SetItem(globals,
                       PyString_FromString(name),
                       pyvalue); // steals reference
    }
    else {
        PyModule_AddObject(pymodule,
                           (char *) name,
                           pyvalue); // steals reference
    }

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_long(JNIEnv *env,
                               intptr_t _jepThread,
                               intptr_t module,
                               const char *name,
                               jeplong value) {
    PyObject      *pyvalue;
    PyThreadState *prevThread;
    PyObject      *pymodule;
    
    // does common things
    GET_COMMON;
    
    if((pyvalue = PyLong_FromLongLong(value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    if(pymodule == NULL) {
        PyDict_SetItem(globals,
                       PyString_FromString(name),
                       pyvalue); // steals reference
    }
    else {
        PyModule_AddObject(pymodule,
                           (char *) name,
                           pyvalue); // steals reference
    }

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_double(JNIEnv *env,
                                 intptr_t _jepThread,
                                 intptr_t module,
                                 const char *name,
                                 double value) {
    PyObject      *pyvalue;
    PyThreadState *prevThread;
    PyObject      *pymodule;
    
    // does common things
    GET_COMMON;
    
    if((pyvalue = PyFloat_FromDouble(value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    if(pymodule == NULL) {
        PyDict_SetItem(globals,
                       PyString_FromString(name),
                       pyvalue); // steals reference
    }
    else {
        PyModule_AddObject(pymodule,
                           (char *) name,
                           pyvalue); // steals reference
    }

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}


void pyembed_setparameter_float(JNIEnv *env,
                                intptr_t _jepThread,
                                intptr_t module,
                                const char *name,
                                float value) {
    PyObject      *pyvalue;
    PyThreadState *prevThread;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;
    
    if((pyvalue = PyFloat_FromDouble((double) value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }
    
    if(pymodule == NULL) {
        PyDict_SetItem(globals,
                       PyString_FromString(name),
                       pyvalue); // steals reference
    }
    else {
        PyModule_AddObject(pymodule,
                           (char *) name,
                           pyvalue); // steals reference
    }

    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
    return;
}
