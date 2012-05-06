/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/* 
   jep - Java Embedded Python

   Copyright (c) 2004 - 2011 Mike Johnson.

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


// The following includes were added to support compilation on RHEL 4, which
// ships with python2.3.  With python2.4 (and possibly beyond), the includes
// are not necessary but do not affect operation.
#if 0
#include <pyport.h>
#include <object.h>
#include <pystate.h>
#include <pythonrun.h>
#include <compile.h>
#endif
// End additional includes for python2.3

#include "pyembed.h"
#include "pyjobject.h"
#include "pyjarray.h"
#include "util.h"


#ifdef __APPLE__
#ifndef WITH_NEXT_FRAMEWORK
#include <crt_externs.h>
// workaround for
// http://bugs.python.org/issue1602133
char **environ = NULL;
#endif
#endif

static PyThreadState *mainThreadState = NULL;

static PyObject* pyembed_findclass(PyObject*, PyObject*);
static PyObject* pyembed_forname(PyObject*, PyObject*);
static PyObject* pyembed_jimport(PyObject*, PyObject*);
static PyObject* pyembed_set_print_stack(PyObject*, PyObject*);
static PyObject* pyembed_jproxy(PyObject*, PyObject*);

static int maybe_pyc_file(FILE*, const char*, const char*, int);
static void pyembed_run_pyc(JepThread *jepThread, FILE *);


// ClassLoader.loadClass
static jmethodID loadClassMethod = 0;

// jep.ClassList.get()
static jmethodID getClassListMethod = 0;

// jep.Proxy.newProxyInstance
static jmethodID newProxyMethod = 0;

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

    { "jproxy",
      pyembed_jproxy,
      METH_VARARGS,
      "Create a Proxy class for a Python object.\n"
      "Accepts two arguments: ([a class object], [list of java interfaces "
      "to implement, string names])" },

    { NULL, NULL }
};


static struct PyMethodDef noop_methods[] = {
    { NULL, NULL }
};


static PyObject* initjep(void) {
    PyObject *modjep;

    PyImport_AddModule("_jep");
    Py_InitModule((char *) "_jep", jep_methods);
    modjep = PyImport_ImportModule("_jep");
    if(modjep == NULL)
        printf("WARNING: couldn't import module _jep.\n");
    else {
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

        PyModule_AddIntConstant(modjep, "USE_MAPPED_EXCEPTIONS", USE_MAPPED_EXCEPTIONS);
    }

    return modjep;
}


void pyembed_startup(void) {
#ifdef __APPLE__
#ifndef WITH_NEXT_FRAMEWORK
// workaround for
// http://bugs.python.org/issue1602133
    environ = *_NSGetEnviron();
#endif
#endif

    if(mainThreadState != NULL)
        return;

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


intptr_t pyembed_thread_init(JNIEnv *env, jobject cl, jobject caller) {
    JepThread *jepThread;
    PyObject  *tdict, *mod_main, *globals;
    
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

    mod_main = PyImport_AddModule("__main__");                      /* borrowed */
    if(mod_main == NULL) {
        THROW_JEP(env, "Couldn't add module __main__.");
        PyEval_ReleaseLock();
        return 0;
    }
    
    globals = PyModule_GetDict(mod_main);
    Py_INCREF(globals);

    // init static module
    jepThread->modjep      = initjep();
    jepThread->globals     = globals;
    jepThread->env         = env;
    jepThread->classloader = (*env)->NewGlobalRef(env, cl);
    jepThread->caller      = (*env)->NewGlobalRef(env, caller);
    jepThread->printStack  = 0;

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
    PyThreadState *prevThread;
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

    if(jepThread->globals) {
        Py_DECREF(jepThread->globals);
    }
    if(jepThread->modjep) {
        Py_DECREF(jepThread->modjep);
    }
    if(jepThread->classloader) {
        (*env)->DeleteGlobalRef(env, jepThread->classloader);
    }
    if(jepThread->caller) {
        (*env)->DeleteGlobalRef(env, jepThread->caller);
    }
    
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


static PyObject* pyembed_jproxy(PyObject *self, PyObject *args) {
    PyThreadState *_save;
    JepThread     *jepThread;
    JNIEnv        *env = NULL;
    PyObject      *pytarget;
    PyObject      *interfaces;
    jclass         clazz;
    jobject        cl;
    jobject        classes;
    Py_ssize_t     inum, i;
    jobject        proxy;

	if(!PyArg_ParseTuple(args, "OO!:jproxy",
                         &pytarget, 
                         &PyList_Type,
                         &interfaces))
        return NULL;

    jepThread = pyembed_get_jepthread();
    if(!jepThread) {
        if(!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Invalid JepThread pointer.");
        return NULL;
    }

    env = jepThread->env;
    cl  = jepThread->classloader;

    Py_UNBLOCK_THREADS;
    clazz = (*env)->FindClass(env, "jep/Proxy");
    Py_BLOCK_THREADS;
    if(process_java_exception(env) || !clazz)
        return NULL;

    if(newProxyMethod == 0) {
        newProxyMethod =
            (*env)->GetStaticMethodID(
                env,
                clazz,
                "newProxyInstance",
                "(JJLjep/Jep;Ljava/lang/ClassLoader;[Ljava/lang/String;)Ljava/lang/Object;");

        if(process_java_exception(env) || !newProxyMethod)
            return NULL;
    }

    inum = PyList_GET_SIZE(interfaces);
    if(inum < 1)
        return PyErr_Format(PyExc_ValueError, "Empty interface list.");

    // now convert string list to java array

    classes = (*env)->NewObjectArray(env, (jsize) inum, JSTRING_TYPE, NULL);
    if(process_java_exception(env) || !classes)
        return NULL;

    for(i = 0; i < inum; i++) {
        char     *str;
        jstring   jstr;
        PyObject *item;

        item = PyList_GET_ITEM(interfaces, i);
        if(!PyString_Check(item))
            return PyErr_Format(PyExc_ValueError, "Item %zd not a string.", i);

        str  = PyString_AsString(item);
        jstr = (*env)->NewStringUTF(env, (const char *) str);

        (*env)->SetObjectArrayElement(env, classes, (jsize) i, jstr);
        (*env)->DeleteLocalRef(env, jstr);
    }

    // do the deed
    proxy = (*env)->CallStaticObjectMethod(env,
                                           clazz,
                                           newProxyMethod,
                                           (jlong) (intptr_t) jepThread,
                                           (jlong) (intptr_t) pytarget,
                                           jepThread->caller,
                                           cl,
                                           classes);
    if(process_java_exception(env) || !proxy)
        return NULL;

    // make sure target doesn't get garbage collected
    Py_INCREF(pytarget);

    return pyjobject_new(env, proxy);
}


static PyObject* pyembed_set_print_stack(PyObject *self, PyObject *args) {
    JepThread *jepThread;
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
    PyThreadState *_save;
    JNIEnv        *env = NULL;
    jstring        jname;
    jclass         clazz;
    jobject        cl;
    JepThread     *jepThread;
    Py_ssize_t     len, i;
    jobjectArray   jar;

	char         *name;
    PyObject     *module     = NULL;
    PyObject     *addmod     = NULL; /* add object to, may be same as module */
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

    Py_UNBLOCK_THREADS;
    clazz = (*env)->FindClass(env, "jep/ClassList");
    Py_BLOCK_THREADS;
    if(process_import_exception(env) || !clazz)
        return NULL;

    if(getClassListMethod == 0) {
        getClassListMethod =
            (*env)->GetStaticMethodID(env,
                                      clazz,
                                      "get",
                                      "(Ljava/lang/String;)[Ljava/lang/String;");
        
        if(process_import_exception(env) || !getClassListMethod)
            return NULL;
    }

    jname = (*env)->NewStringUTF(env, name);
    Py_UNBLOCK_THREADS;
    jar = (jobjectArray) (*env)->CallStaticObjectMethod(
        env,
        clazz,
        getClassListMethod,
        jname);
    Py_BLOCK_THREADS;

    if(process_import_exception(env) || !jar)
        return NULL;

    // process module name. foo.bar must be split so 'module' is 'bar'
    {
        PyObject *pyname;
        PyObject *tname;
        PyObject *modlist;

        pyname  = PyString_FromString(name);
        modlist = PyObject_CallMethod(pyname, "split", "s", ".");
        Py_DECREF(pyname);

        if(!PyList_Check(modlist) || PyErr_Occurred()) {
            return PyErr_Format(PyExc_ImportError,
                                "Couldn't split package name %s ",
                                name);
        }

        // first module gets added a little differently
        tname  = PyList_GET_ITEM(modlist, 0); /* borrowed */
        addmod = module = PyImport_AddModule(PyString_AsString(tname));

        if(module == NULL || PyErr_Occurred()) {
            return PyErr_Format(PyExc_ImportError,
                                "Couldn't add package %s ",
                                name);
        }

        len = PyList_GET_SIZE(modlist);
        for(i = 1; i < len; i++) {
            char     *cname;
            PyObject *globals;  /* shadow parent scope */

            tname = PyList_GET_ITEM(modlist, i); /* borrowed */
            cname = PyString_AsString(tname);
            
            globals = PyModule_GetDict(addmod); /* borrowed */

            Py_InitModule(cname, noop_methods);
            addmod = PyImport_ImportModuleEx(cname, globals, globals, NULL); /* new ref */

            PyDict_SetItem(globals,
                           tname,
                           addmod);     /* ownership */
            Py_DECREF(addmod);
        }
    }

    len = (*env)->GetArrayLength(env, jar);
    for(i = 0; i < len; i++) {
        jstring   member     = NULL;
        jclass    objclazz   = NULL;
        PyObject *pclass     = NULL;
        PyObject *memberList = NULL;

        member = (*env)->GetObjectArrayElement(env, jar, (jsize) i);
        if(process_import_exception(env) || !member) {
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

        // make sure our class name is in the fromlist (a tuple)
        if(PyTuple_Check(fromlist) &&
           PyString_AsString(PyTuple_GET_ITEM(fromlist, 0))[0] != '*') {

            PyObject   *pymember;
            int         found;
            Py_ssize_t  i, len;

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

        Py_UNBLOCK_THREADS;
        objclazz = (jclass) (*env)->CallObjectMethod(env,
                                                     cl,
                                                     loadClassMethod,
                                                     member);
        Py_BLOCK_THREADS;

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
                addmod,
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
        return PyErr_Format(PyExc_ImportError, "No module %s found", name);

    if(PyTuple_Check(fromlist) && PyTuple_GET_SIZE(fromlist) > 0) {
        // user specified a list of objects.
        // python's __import__.__doc__ reads:

        /* When importing a module from a package, note that
         * __import__('A.B', ...) returns package A when fromlist is
         * empty, but its submodule B when fromlist is not empty.
         */

        if(addmod == NULL) {
            return PyErr_Format(
                PyExc_ImportError,
                "While importing %s addmod was NULL. I goofed.",
                name);
        }

        Py_INCREF(addmod);
        return addmod;
    }

    // otherwise, return parent module.
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


jobject pyembed_invoke_method(JNIEnv *env,
                              intptr_t _jepThread,
                              const char *cname,
                              jobjectArray args,
                              jintArray types) {
    PyThreadState    *prevThread;
    PyObject         *callable;
    JepThread        *jepThread;
    jobject           ret;
    
    ret = NULL;

    jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return ret;
    }

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);

    callable = PyDict_GetItemString(jepThread->globals, (char *) cname);
    if(!callable) {
        THROW_JEP(env, "Object was not found in the global dictionary.");
        goto EXIT;
    }
    if(process_py_exception(env, 0))
        goto EXIT;

    ret = pyembed_invoke(env, callable, args, types);

EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();

    return ret;
}


// invoke object callable
// **** hold lock before calling ****
jobject pyembed_invoke(JNIEnv *env,
                       PyObject *callable,
                       jobjectArray args,
                       jintArray _types) {
    jobject        ret;
    int            iarg, arglen;
    jint          *types;       /* pinned primitive array */
    jboolean       isCopy;
    PyObject      *pyargs;      /* a tuple */
    PyObject      *pyret;

    types    = NULL;
    ret      = NULL;
    pyret    = NULL;

    if(!PyCallable_Check(callable)) {
        THROW_JEP(env, "pyembed:invoke Invalid callable.");
        return NULL;
    }

    // pin primitive array so we can get to it
    types = (*env)->GetIntArrayElements(env, _types, &isCopy);

    // first thing to do, convert java arguments to a python tuple
    arglen = (*env)->GetArrayLength(env, args);
    pyargs = PyTuple_New(arglen);
    for(iarg = 0; iarg < arglen; iarg++) {
        jobject   val;
        int       typeid;
        PyObject *pyval;

        val = (*env)->GetObjectArrayElement(env, args, iarg);
        if((*env)->ExceptionCheck(env)) /* careful, NULL is okay */
            goto EXIT;

        typeid = (int) types[iarg];

        // now we know the type, convert and add to pyargs.  we know
        pyval = convert_jobject(env, val, typeid);
        if((*env)->ExceptionOccurred(env))
            goto EXIT;

        PyTuple_SET_ITEM(pyargs, iarg, pyval); /* steals */
        if(val)
            (*env)->DeleteLocalRef(env, val);
    } // for(iarg = 0; iarg < arglen; iarg++)

    pyret = PyObject_CallObject(callable, pyargs);
    if(process_py_exception(env, 0) || !pyret)
        goto EXIT;

    // handles errors
    ret = pyembed_box_py(env, pyret);

EXIT:
    if(pyargs) {
        Py_DECREF(pyargs);
    }
    if(pyret) {
        Py_DECREF(pyret);
    }

    if(types) {
        (*env)->ReleaseIntArrayElements(env,
                                        _types,
                                        types,
                                        JNI_ABORT);

        (*env)->DeleteLocalRef(env, _types);
    }

    return ret;
}


void pyembed_eval(JNIEnv *env,
                  intptr_t _jepThread,
                  char *str) {
    PyThreadState    *prevThread;
    PyObject         *result;
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
    
    result = PyRun_String(str,  /* new ref */
                          Py_single_input,
                          jepThread->globals,
                          jepThread->globals);
    
    // c programs inside some java environments may get buffered output
    fflush(stdout);
    fflush(stderr);
    
    process_py_exception(env, 1);
    
    if(result != NULL) {
        Py_DECREF(result);
    }

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


intptr_t pyembed_create_module(JNIEnv *env,
                               intptr_t _jepThread,
                               char *str) {
    PyThreadState  *prevThread;
    PyObject       *module;
    JepThread      *jepThread;
    intptr_t        ret;
    PyObject       *key;

    jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return 0;
    }

    if(str == NULL)
        return 0;

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);

    if(PyImport_AddModule(str) == NULL || process_py_exception(env, 1))
        goto EXIT;

    Py_InitModule(str, noop_methods);
    module = PyImport_ImportModuleEx(str,
                                     jepThread->globals,
                                     jepThread->globals,
                                     NULL); /* new ref */

    key = PyString_FromString(str);
    PyDict_SetItem(jepThread->globals,
                   key,
                   module);     /* takes ownership */

    Py_DECREF(key);
    Py_DECREF(module);

    if(process_py_exception(env, 0) || module == NULL)
        ret = 0;
    else
        ret = (intptr_t) module;

EXIT:
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
    PyObject       *key;

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
    module = PyImport_ImportModuleEx(str, globals, globals, NULL); /* new ref */

    key = PyString_FromString(str);
    PyDict_SetItem(globals,
                   key,
                   module);     /* ownership */
    Py_DECREF(key);
    Py_DECREF(module);

    if(process_py_exception(env, 0) || module == NULL)
        ret = 0;
    else
        ret = (intptr_t) module;

EXIT:
    if(globals) {
        Py_DECREF(globals);
    }

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

    // class and object need to return a new local ref so the object
    // isn't garbage collected.

    if(pyjclass_check(result))
        return (*env)->NewLocalRef(env, ((PyJobject_Object *) result)->clazz);

    if(pyjobject_check(result))
        return (*env)->NewLocalRef(env, ((PyJobject_Object *) result)->object);

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
        jint i = (jint) PyInt_AS_LONG(result);

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
        jfloat f = (jfloat) PyFloat_AS_DOUBLE(result);

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

    if(pyjarray_check(result)) {
        PyJarray_Object *t = (PyJarray_Object *) result;
        pyjarray_release_pinned(t, JNI_COMMIT);

        return t->object;
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

    if(result != NULL) {
        Py_DECREF(result);
    }
    return ret;
}


jobject pyembed_getvalue(JNIEnv *env, intptr_t _jepThread, char *str) {
    PyThreadState  *prevThread;
    PyObject       *result;
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
    
    result = PyRun_String(str,  /* new ref */
                          Py_eval_input,
                          jepThread->globals,
                          jepThread->globals);
    
    process_py_exception(env, 1);
    
    if(result == NULL || result == Py_None)
        goto EXIT;              /* don't return, need to release GIL */
    
    // convert results to jobject
    ret = pyembed_box_py(env, result);
    
EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();

    if(result != NULL) {
        Py_DECREF(result);
    }
    return ret;
}



jobject pyembed_getvalue_array(JNIEnv *env, intptr_t _jepThread, char *str, int typeId) {
    PyThreadState  *prevThread;
    PyObject       *result;
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
    
    result = PyRun_String(str,  /* new ref */
                          Py_eval_input,
                          jepThread->globals,
                          jepThread->globals);
    
    process_py_exception(env, 1);
    
    if(result == NULL || result == Py_None)
        goto EXIT;              /* don't return, need to release GIL */
    
    if(PyString_Check(result)) {
        void *s = (void*) PyString_AS_STRING(result);
        Py_ssize_t n = PyString_Size(result);

        switch (typeId) {
        case JFLOAT_ID:
            if(n % SIZEOF_FLOAT != 0) {
                THROW_JEP(env, "The Python string is the wrong length.\n");
                goto EXIT;
            }

            ret = (*env)->NewFloatArray(env, (jsize) n / SIZEOF_FLOAT);
            (*env)->SetFloatArrayRegion(env, ret, 0, (jsize) (n / SIZEOF_FLOAT), (jfloat *) s);
            break;

        case JBYTE_ID:
            ret = (*env)->NewByteArray(env, (jsize) n);
            (*env)->SetByteArrayRegion(env, ret, 0, (jsize) n, (jbyte *) s);
            break;

        default:
            THROW_JEP(env, "Internal error: array type not handled.");
            ret = NULL;
            goto EXIT;

        } // switch

    }
    else{
        THROW_JEP(env, "Value is not a string.");
        goto EXIT;
    }
    
    
EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();

    if(result != NULL) {
        Py_DECREF(result);
    }
    return ret;
}






void pyembed_run(JNIEnv *env,
                 intptr_t _jepThread,
                 char *file) {
    PyThreadState *prevThread;
    JepThread     *jepThread;
    const char    *ext;
    
    jepThread = (JepThread *) _jepThread;
    if(!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return;
    }

    PyEval_AcquireLock();
    prevThread = PyThreadState_Swap(jepThread->tstate);
    
    if(file != NULL) {
        FILE *script = fopen(file, "r");
        if(!script) {
            THROW_JEP(env, "Couldn't open script file.");
            goto EXIT;
        }

        // check if it's a pyc/pyo file
        ext = file + strlen(file) - 4;
        if (maybe_pyc_file(script, file, ext, 0)) {
            /* Try to run a pyc file. First, re-open in binary */
			fclose(script);
            if((script = fopen(file, "rb")) == NULL) {
                THROW_JEP(env, "pyembed_run: Can't reopen .pyc file");
                goto EXIT;
            }

            /* Turn on optimization if a .pyo file is given */
            if(strcmp(ext, ".pyo") == 0)
                Py_OptimizeFlag = 1;
            else
                Py_OptimizeFlag = 0;

            pyembed_run_pyc(jepThread, script);
        }
        else {
            PyRun_File(script,
                       file,
                       Py_file_input,
                       jepThread->globals,
                       jepThread->globals);
        }

        // c programs inside some java environments may get buffered output
        fflush(stdout);
        fflush(stderr);
        
        fclose(script);
        process_py_exception(env, 1);
    }

EXIT:
    PyThreadState_Swap(prevThread);
    PyEval_ReleaseLock();
}


// gratuitously copyied from pythonrun.c::run_pyc_file
static void pyembed_run_pyc(JepThread *jepThread,
                            FILE *fp) {
	PyCodeObject    *co;
	PyObject        *v;
	long             magic;

	long PyImport_GetMagicNumber(void);

	magic = PyMarshal_ReadLongFromFile(fp);
	if(magic != PyImport_GetMagicNumber()) {
		PyErr_SetString(PyExc_RuntimeError,
                        "Bad magic number in .pyc file");
		return;
	}
	(void) PyMarshal_ReadLongFromFile(fp);
	v = (PyObject *) (intptr_t) PyMarshal_ReadLastObjectFromFile(fp);
	if(v == NULL || !PyCode_Check(v)) {
		Py_XDECREF(v);
		PyErr_SetString(PyExc_RuntimeError,
                        "Bad code object in .pyc file");
		return;
	}
	co = (PyCodeObject *) v;
	v = PyEval_EvalCode(co, jepThread->globals, jepThread->globals);
	Py_DECREF(co);
    Py_XDECREF(v);
}


/* Check whether a file maybe a pyc file: Look at the extension,
   the file type, and, if we may close it, at the first few bytes. */
// gratuitously copyied from pythonrun.c::run_pyc_file
static int maybe_pyc_file(FILE *fp,
                          const char* filename,
                          const char* ext,
                          int closeit) {
	if(strcmp(ext, ".pyc") == 0 || strcmp(ext, ".pyo") == 0)
		return 1;

	/* Only look into the file if we are allowed to close it, since
	   it then should also be seekable. */
	if(closeit) {
		/* Read only two bytes of the magic. If the file was opened in
		   text mode, the bytes 3 and 4 of the magic (\r\n) might not
		   be read as they are on disk. */
		unsigned int halfmagic = (unsigned int) PyImport_GetMagicNumber() & 0xFFFF;
		unsigned char buf[2];
		/* Mess:  In case of -x, the stream is NOT at its start now,
		   and ungetc() was used to push back the first newline,
		   which makes the current stream position formally undefined,
		   and a x-platform nightmare.
		   Unfortunately, we have no direct way to know whether -x
		   was specified.  So we use a terrible hack:  if the current
		   stream position is not 0, we assume -x was specified, and
		   give up.  Bug 132850 on SourceForge spells out the
		   hopelessness of trying anything else (fseek and ftell
		   don't work predictably x-platform for text-mode files).
		*/
		int ispyc = 0;
		if(ftell(fp) == 0) {
			if(fread(buf, 1, 2, fp) == 2 &&
               ((unsigned int)buf[1]<<8 | buf[0]) == halfmagic)
				ispyc = 1;
			rewind(fp);
		}

		return ispyc;
	}
	return 0;
}


// -------------------------------------------------- set() things

#define GET_COMMON                                                  \
    JepThread *jepThread;                                           \
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
            PyObject *key = PyString_FromString(name);
            PyDict_SetItem(jepThread->globals,
                           key,
                           pyjob); /* ownership */
            Py_DECREF(key);
            Py_DECREF(pyjob);
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


void pyembed_setparameter_array(JNIEnv *env,
                                intptr_t _jepThread,
                                intptr_t module,
                                const char *name,
                                jobjectArray obj) {
    PyObject      *pyjob;
    PyThreadState *prevThread;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;

    if(obj == NULL) {
        Py_INCREF(Py_None);
        pyjob = Py_None;
    }
    else
        pyjob = pyjarray_new(env, obj);
    
    if(pyjob) {
        if(pymodule == NULL) {
            PyObject *key = PyString_FromString(name);
            PyDict_SetItem(jepThread->globals,
                           key,
                           pyjob); /* ownership */
            Py_DECREF(key);
            Py_DECREF(pyjob);
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
            PyObject *key = PyString_FromString(name);
            PyDict_SetItem(jepThread->globals,
                           key,
                           pyjob); /* ownership */
            Py_DECREF(key);
            Py_DECREF(pyjob);
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
        PyObject *key = PyString_FromString(name);
        PyDict_SetItem(jepThread->globals,
                       key,
                       pyvalue); /* ownership */
        Py_DECREF(key);
        Py_DECREF(pyvalue);
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
        PyObject *key = PyString_FromString(name);
        PyDict_SetItem(jepThread->globals,
                       key,
                       pyvalue); /* ownership */
        Py_DECREF(key);
        Py_DECREF(pyvalue);
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
        PyObject *key = PyString_FromString(name);
        PyDict_SetItem(jepThread->globals,
                       key,
                       pyvalue); /* ownership */
        Py_DECREF(key);
        Py_DECREF(pyvalue);
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
        PyObject *key = PyString_FromString(name);
        PyDict_SetItem(jepThread->globals,
                       key,
                       pyvalue); /* ownership */
        Py_DECREF(key);
        Py_DECREF(pyvalue);
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
        PyObject *key = PyString_FromString(name);
        PyDict_SetItem(jepThread->globals,
                       key,
                       pyvalue); /* ownership */
        Py_DECREF(key);
        Py_DECREF(pyvalue);
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
