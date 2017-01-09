/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/*
   jep - Java Embedded Python

   Copyright (c) 2016 JEP AUTHORS.

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

#if STDC_HEADERS
    #include <stdio.h>
#endif

#include "Jep.h"

/*
 * fixes compiler warnings about PyMarshal_ReadLongFromFile and
 * PyMarshal_ReadLastObjectFromFile
 */
#include "marshal.h"


#ifdef __APPLE__
    #ifndef WITH_NEXT_FRAMEWORK
        #include <crt_externs.h>
        // workaround for
        // http://bugs.python.org/issue1602133
        char **environ = NULL;
    #endif
#endif

static PyThreadState *mainThreadState = NULL;

/* Saved for cross thread access to shared modules. */
static PyObject* mainThreadModules = NULL;

int pyembed_version_unsafe(void);

static PyObject* pyembed_findclass(PyObject*, PyObject*);
static PyObject* pyembed_forname(PyObject*, PyObject*);
static PyObject* pyembed_set_print_stack(PyObject*, PyObject*);
static PyObject* pyembed_jproxy(PyObject*, PyObject*);

static int maybe_pyc_file(FILE*, const char*, const char*, int);
static void pyembed_run_pyc(JepThread *jepThread, FILE *);


// ClassLoader.loadClass
static jmethodID loadClassMethod = 0;

// jep.Proxy.newProxyInstance
static jmethodID newProxyMethod = 0;

static struct PyMethodDef jep_methods[] = {
    {
        "findClass",
        pyembed_findclass,
        METH_VARARGS,
        "Find and instantiate a system class, somewhat faster than forName."
    },

    {
        "forName",
        pyembed_forname,
        METH_VARARGS,
        "Find and return a jclass object using the supplied ClassLoader."
    },

    {
        "jarray",
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

    {
        "printStack",
        pyembed_set_print_stack,
        METH_VARARGS,
        "Turn on printing of stack traces (True|False)"
    },

    {
        "jproxy",
        pyembed_jproxy,
        METH_VARARGS,
        "Create a Proxy class for a Python object.\n"
        "Accepts two arguments: ([a class object], [list of java interfaces "
        "to implement, string names])"
    },

    { NULL, NULL }
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef jep_module_def = {
    PyModuleDef_HEAD_INIT,
    "_jep",              /* m_name */
    "_jep",              /* m_doc */
    -1,                  /* m_size */
    jep_methods,         /* m_methods */
    NULL,                /* m_reload */
    NULL,                /* m_traverse */
    NULL,                /* m_clear */
    NULL,                /* m_free */
};
#endif

static PyObject* initjep(void)
{
    PyObject *modjep;

#if PY_MAJOR_VERSION >= 3
    PyObject *sysmodules;
    modjep = PyModule_Create(&jep_module_def);
    sysmodules = PyImport_GetModuleDict();
    PyDict_SetItemString(sysmodules, "_jep", modjep);
#else
    PyImport_AddModule("_jep");
    Py_InitModule((char *) "_jep", jep_methods);
#endif
    modjep = PyImport_ImportModule("_jep");
    if (modjep == NULL) {
        printf("WARNING: couldn't import module _jep.\n");
    } else {
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
        PyModule_AddIntConstant(modjep, "JEP_NUMPY_ENABLED", JEP_NUMPY_ENABLED);
        Py_INCREF(mainThreadModules);
        PyModule_AddObject(modjep, "topInterpreterModules", mainThreadModules);
    }

    return modjep;
}

void pyembed_preinit(jint noSiteFlag,
                     jint noUserSiteDirectory,
                     jint ignoreEnvironmentFlag,
                     jint verboseFlag,
                     jint optimizeFlag,
                     jint dontWriteBytecodeFlag,
                     jint hashRandomizationFlag)
{
    if (noSiteFlag >= 0) {
        Py_NoSiteFlag = noSiteFlag;
    }
    if (noUserSiteDirectory >= 0) {
        Py_NoUserSiteDirectory = noUserSiteDirectory;
    }
    if (ignoreEnvironmentFlag >= 0) {
        Py_IgnoreEnvironmentFlag = ignoreEnvironmentFlag;
    }
    if (verboseFlag >= 0) {
        Py_VerboseFlag = verboseFlag;
    }
    if (optimizeFlag >= 0) {
        Py_OptimizeFlag = optimizeFlag;
    }
    if (dontWriteBytecodeFlag >= 0) {
        Py_DontWriteBytecodeFlag = dontWriteBytecodeFlag;
    }
    if (hashRandomizationFlag >= 0) {
        Py_HashRandomizationFlag = hashRandomizationFlag;
    }

}

void pyembed_startup(void)
{
    PyObject* sysModule = NULL;
#ifdef __APPLE__
#ifndef WITH_NEXT_FRAMEWORK
// workaround for
// http://bugs.python.org/issue1602133
    environ = *_NSGetEnviron();
#endif
#endif

    if (mainThreadState != NULL) {
        // this shouldn't happen but to be safe, don't initialize twice
        return;
    }

    if (pyembed_version_unsafe()) {
        return;
    }

    Py_Initialize();
    PyEval_InitThreads();

    /*
     * Save a global reference to the sys.modules form the main thread to
     * support shared modules
     */
    sysModule = PyImport_ImportModule("sys");
    mainThreadModules = PyObject_GetAttrString(sysModule, "modules");
    Py_DECREF(sysModule);

    // save a pointer to the main PyThreadState object
    mainThreadState = PyThreadState_Get();
    PyEval_ReleaseThread(mainThreadState);
}


/*
 * Verify the Python major.minor at runtime matches the Python major.minor
 * that Jep was built against.  If they don't match, refuse to initialize Jep
 * and instead throw an exception because a mismatch could cause a JVM crash.
 */
int pyembed_version_unsafe(void)
{
    const char *pyversion = NULL;
    char       *version   = NULL;
    char       *major     = NULL;
    char       *minor     = NULL;
    int         i         = 0;

    pyversion = Py_GetVersion();
    version = malloc(sizeof(char) * strlen(pyversion));
    strcpy(version, pyversion);
    major = version;

    while (version[i] != '\0') {
        if (!isdigit(version[i])) {
            version[i] = '\0';
            if (minor == NULL) {
                minor = version + i + 1;
            }
        }
        i += 1;
    }

    if (atoi(major) != PY_MAJOR_VERSION || atoi(minor) != PY_MINOR_VERSION) {
        char *msg;
        JNIEnv *env = pyembed_get_env();

        msg = malloc(sizeof(char) * 200);
        memset(msg, '\0', 200);
        sprintf(msg,
                "Jep will not initialize because it was compiled against Python %i.%i but is running against Python %s.%s",
                PY_MAJOR_VERSION, PY_MINOR_VERSION, major, minor);
        THROW_JEP(env, msg);
        free(version);
        free(msg);
        return 1;
    }

    free(version);
    return 0;
}


void pyembed_shutdown(JavaVM *vm)
{
    JNIEnv *env;

    // shut down python first
    PyEval_AcquireThread(mainThreadState);
    Py_Finalize();

    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        // failed to get a JNIEnv*, we can hope it's just shutting down fast
        return;
    } else {
        // delete global references
        unref_cache_primitive_classes(env);
        unref_cache_frequent_classes(env);
    }
}

/*
 * Import a module on the main thread. This must be called from the same thread
 * as pyembed_startup. On success the specified module will be available in the
 * mainThreadModules. On failure this function will raise a java exception.
 */
void pyembed_shared_import(JNIEnv *env, jstring module)
{
    const char *moduleName = NULL;
    PyObject   *pymodule   =  NULL;
    PyEval_AcquireThread(mainThreadState);

    moduleName = (*env)->GetStringUTFChars(env, module, 0);

    pymodule = PyImport_ImportModule(moduleName);
    if (pymodule) {
        Py_DECREF(pymodule);
    } else {
        PyObject *ptype, *pvalue, *pvaluestr, *ptrace;
        PyErr_Fetch(&ptype, &pvalue, &ptrace);
        Py_DECREF(ptype);
        Py_XDECREF(ptrace);
        pvaluestr = PyObject_Str(pvalue);
        Py_DECREF(pvalue);
        THROW_JEP(env, PyString_AsString(pvaluestr));
        Py_DECREF(pvaluestr);
    }

    (*env)->ReleaseStringUTFChars(env, module, moduleName);
    PyEval_ReleaseThread(mainThreadState);
}

intptr_t pyembed_thread_init(JNIEnv *env, jobject cl, jobject caller)
{
    JepThread *jepThread;
    PyObject  *tdict, *mod_main, *globals;
    if (cl == NULL) {
        THROW_JEP(env, "Invalid Classloader.");
        return 0;
    }

    PyEval_AcquireThread(mainThreadState);

    /*
     * Do not use PyMem_Malloc because PyGILState_Check() returns false since
     * the mainThreadState was created on a different thread. When python is
     * compiled with debug it checks the state and fails.
     */
    jepThread = malloc(sizeof(JepThread));
    if (!jepThread) {
        THROW_JEP(env, "Out of memory.");
        PyEval_ReleaseThread(mainThreadState);
        return 0;
    }

    jepThread->tstate = Py_NewInterpreter();
    /*
     * Py_NewInterpreter() seems to take the thread state, but we're going to
     * save/release and reacquire it since that doesn't seem documented
     */
    PyEval_SaveThread();
    PyEval_AcquireThread(jepThread->tstate);

    // store java.lang.Class objects for later use.
    // it's a noop if already done, but to synchronize, have the lock first
    if (!cache_frequent_classes(env)) {
        printf("WARNING: Failed to get and cache frequent class types!\n");
    }
    if (!cache_primitive_classes(env)) {
        printf("WARNING: Failed to get and cache primitive class types!\n");
    }


    mod_main = PyImport_AddModule("__main__");                      /* borrowed */
    if (mod_main == NULL) {
        THROW_JEP(env, "Couldn't add module __main__.");
        PyEval_ReleaseThread(jepThread->tstate);
        return 0;
    }
    globals = PyModule_GetDict(mod_main);
    Py_INCREF(globals);

    // init static module
    jepThread->modjep          = initjep();
    jepThread->globals         = globals;
    jepThread->env             = env;
    jepThread->classloader     = (*env)->NewGlobalRef(env, cl);
    jepThread->caller          = (*env)->NewGlobalRef(env, caller);
    jepThread->printStack      = 0;
    jepThread->fqnToPyJmethods = NULL;

    if ((tdict = PyThreadState_GetDict()) != NULL) {
        PyObject *key, *t;

#if PY_MAJOR_VERSION >= 3
        t   = PyCapsule_New((void *) jepThread, NULL, NULL);
#else
        t   = (PyObject *) PyCObject_FromVoidPtr((void *) jepThread, NULL);
#endif
        key = PyString_FromString(DICT_KEY);

        PyDict_SetItem(tdict, key, t);   /* takes ownership */

        Py_DECREF(key);
        Py_DECREF(t);
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return (intptr_t) jepThread;
}


void pyembed_thread_close(JNIEnv *env, intptr_t _jepThread)
{
    JepThread     *jepThread;
    PyObject      *tdict, *key;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        printf("WARNING: thread_close, invalid JepThread pointer.\n");
        return;
    }

    PyEval_AcquireThread(jepThread->tstate);

    key = PyString_FromString(DICT_KEY);
    if ((tdict = PyThreadState_GetDict()) != NULL && key != NULL) {
        PyDict_DelItem(tdict, key);
    }
    Py_DECREF(key);

    Py_CLEAR(jepThread->globals);
    Py_CLEAR(jepThread->fqnToPyJmethods);
    Py_CLEAR(jepThread->modjep);

    if (jepThread->classloader) {
        (*env)->DeleteGlobalRef(env, jepThread->classloader);
    }
    if (jepThread->caller) {
        (*env)->DeleteGlobalRef(env, jepThread->caller);
    }

    Py_EndInterpreter(jepThread->tstate);

    free(jepThread);
    PyEval_ReleaseLock();
}


JNIEnv* pyembed_get_env(void)
{
    JavaVM *jvm;
    JNIEnv *env;

    JNI_GetCreatedJavaVMs(&jvm, 1, NULL);
    (*jvm)->AttachCurrentThread(jvm, (void**) &env, NULL);

    return env;
}


// get thread struct when called from internals.
// NULL if not found.
// hold the lock before calling.
JepThread* pyembed_get_jepthread(void)
{
    PyObject  *tdict, *t, *key;
    JepThread *ret = NULL;

    key = PyString_FromString(DICT_KEY);
    if ((tdict = PyThreadState_GetDict()) != NULL && key != NULL) {
        t = PyDict_GetItem(tdict, key); /* borrowed */
        if (t != NULL && !PyErr_Occurred()) {
#if PY_MAJOR_VERSION >= 3
            ret = (JepThread*) PyCapsule_GetPointer(t, NULL);
#else
            ret = (JepThread*) PyCObject_AsVoidPtr(t);
#endif
        }
    }

    Py_DECREF(key);
    return ret;
}


// used by _forname
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


static PyObject* pyembed_jproxy(PyObject *self, PyObject *args)
{
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

    if (!PyArg_ParseTuple(args, "OO!:jproxy",
                          &pytarget,
                          &PyList_Type,
                          &interfaces)) {
        return NULL;
    }

    jepThread = pyembed_get_jepthread();
    if (!jepThread) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(PyExc_RuntimeError, "Invalid JepThread pointer.");
        }
        return NULL;
    }

    env = jepThread->env;
    cl  = jepThread->classloader;

    Py_UNBLOCK_THREADS;
    clazz = (*env)->FindClass(env, "jep/Proxy");
    Py_BLOCK_THREADS;
    if (process_java_exception(env) || !clazz) {
        return NULL;
    }

    if (newProxyMethod == 0) {
        newProxyMethod =
            (*env)->GetStaticMethodID(
                env,
                clazz,
                "newProxyInstance",
                "(JJLjep/Jep;Ljava/lang/ClassLoader;[Ljava/lang/String;)Ljava/lang/Object;");

        if (process_java_exception(env) || !newProxyMethod) {
            return NULL;
        }
    }

    inum = (int) PyList_GET_SIZE(interfaces);
    if (inum < 1) {
        return PyErr_Format(PyExc_ValueError, "Empty interface list.");
    }

    // now convert string list to java array

    classes = (*env)->NewObjectArray(env, (jsize) inum, JSTRING_TYPE, NULL);
    if (process_java_exception(env) || !classes) {
        return NULL;
    }

    for (i = 0; i < inum; i++) {
        char     *str;
        jstring   jstr;
        PyObject *item;

        item = PyList_GET_ITEM(interfaces, i);
        if (!PyString_Check(item)) {
            return PyErr_Format(PyExc_ValueError, "Item %zd not a string.", i);
        }

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
    if (process_java_exception(env) || !proxy) {
        return NULL;
    }

    // make sure target doesn't get garbage collected
    Py_INCREF(pytarget);

    return pyjobject_new(env, proxy);
}


static PyObject* pyembed_set_print_stack(PyObject *self, PyObject *args)
{
    JepThread *jepThread;
    char      *print = 0;

    if (!PyArg_ParseTuple(args, "b:setPrintStack", &print)) {
        return NULL;
    }

    jepThread = pyembed_get_jepthread();
    if (!jepThread) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(PyExc_RuntimeError, "Invalid JepThread pointer.");
        }
        return NULL;
    }

    if (print == 0) {
        jepThread->printStack = 0;
    } else {
        jepThread->printStack = 1;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject* pyembed_forname(PyObject *self, PyObject *args)
{
    JNIEnv    *env       = NULL;
    char      *name;
    jobject    cl;
    jclass     objclazz;
    jstring    jstr;
    JepThread *jepThread;
    PyObject  *result;

    if (!PyArg_ParseTuple(args, "s", &name)) {
        return NULL;
    }

    jepThread = pyembed_get_jepthread();
    if (!jepThread) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(PyExc_RuntimeError, "Invalid JepThread pointer.");
        }
        return NULL;
    }

    env = jepThread->env;
    cl  = jepThread->classloader;

    LOAD_CLASS_METHOD(env, cl);

    jstr = (*env)->NewStringUTF(env, (const char *) name);
    if (process_java_exception(env) || !jstr) {
        return NULL;
    }

    objclazz = (jclass) (*env)->CallObjectMethod(env,
               cl,
               loadClassMethod,
               jstr);
    (*env)->DeleteLocalRef(env, jstr);
    if (process_java_exception(env) || !objclazz) {
        return NULL;
    }
    result = (PyObject *) pyjobject_new_class(env, objclazz);
    (*env)->DeleteLocalRef(env, objclazz);
    return result;
}


static PyObject* pyembed_findclass(PyObject *self, PyObject *args)
{
    JNIEnv    *env       = NULL;
    PyObject  *result    = NULL;
    char      *name, *p;
    jclass     clazz;
    JepThread *jepThread;

    if (!PyArg_ParseTuple(args, "s", &name)) {
        return NULL;
    }

    jepThread = pyembed_get_jepthread();
    if (!jepThread) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(PyExc_RuntimeError, "Invalid JepThread pointer.");
        }
        return NULL;
    }

    env = jepThread->env;

    // replace '.' with '/'
    // i'm told this is okay to do with unicode.
    for (p = name; *p != '\0'; p++) {
        if (*p == '.') {
            *p = '/';
        }
    }

    clazz = (*env)->FindClass(env, name);
    if (process_java_exception(env)) {
        return NULL;
    }

    result = (PyObject *) pyjobject_new_class(env, clazz);
    (*env)->DeleteLocalRef(env, clazz);
    return result;
}


jobject pyembed_invoke_method(JNIEnv *env,
                              intptr_t _jepThread,
                              const char *cname,
                              jobjectArray args,
                              jintArray types)
{
    PyObject         *callable;
    JepThread        *jepThread;
    jobject           ret;

    ret = NULL;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return ret;
    }

    PyEval_AcquireThread(jepThread->tstate);

    callable = PyDict_GetItemString(jepThread->globals, (char *) cname);
    if (!callable) {
        THROW_JEP(env, "Object was not found in the global dictionary.");
        goto EXIT;
    }
    if (process_py_exception(env, 0)) {
        goto EXIT;
    }

    ret = pyembed_invoke(env, callable, args, types);

EXIT:
    PyEval_ReleaseThread(jepThread->tstate);

    return ret;
}


// invoke object callable
// **** hold lock before calling ****
jobject pyembed_invoke(JNIEnv *env,
                       PyObject *callable,
                       jobjectArray args,
                       jintArray _types)
{
    jobject        ret;
    int            iarg, arglen;
    jint          *types;       /* pinned primitive array */
    jboolean       isCopy;
    PyObject      *pyargs;      /* a tuple */
    PyObject      *pyret;

    types    = NULL;
    ret      = NULL;
    pyret    = NULL;

    if (!PyCallable_Check(callable)) {
        THROW_JEP(env, "pyembed:invoke Invalid callable.");
        return NULL;
    }

    // pin primitive array so we can get to it
    types = (*env)->GetIntArrayElements(env, _types, &isCopy);

    // first thing to do, convert java arguments to a python tuple
    arglen = (*env)->GetArrayLength(env, args);
    pyargs = PyTuple_New(arglen);
    for (iarg = 0; iarg < arglen; iarg++) {
        jobject   val;
        int       typeid;
        PyObject *pyval;

        val = (*env)->GetObjectArrayElement(env, args, iarg);
        if ((*env)->ExceptionCheck(env)) { /* careful, NULL is okay */
            goto EXIT;
        }

        typeid = (int) types[iarg];

        // now we know the type, convert and add to pyargs.  we know
        pyval = convert_jobject(env, val, typeid);
        if ((*env)->ExceptionOccurred(env)) {
            goto EXIT;
        }

        PyTuple_SET_ITEM(pyargs, iarg, pyval); /* steals */
        if (val) {
            (*env)->DeleteLocalRef(env, val);
        }
    } // for(iarg = 0; iarg < arglen; iarg++)

    pyret = PyObject_CallObject(callable, pyargs);
    if (process_py_exception(env, 0) || !pyret) {
        goto EXIT;
    }

    // handles errors
    ret = PyObject_As_jobject(env, pyret, JOBJECT_TYPE);
    if (!ret) {
        process_py_exception(env, 0);
    }

EXIT:
    Py_XDECREF(pyargs);
    Py_XDECREF(pyret);

    if (types) {
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
                  char *str)
{
    PyObject         *result;
    JepThread        *jepThread;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return;
    }

    PyEval_AcquireThread(jepThread->tstate);

    if (str == NULL) {
        goto EXIT;
    }

    if (process_py_exception(env, 1)) {
        goto EXIT;
    }

    result = PyRun_String(str,  /* new ref */
                          Py_single_input,
                          jepThread->globals,
                          jepThread->globals);

    // c programs inside some java environments may get buffered output
    fflush(stdout);
    fflush(stderr);

    process_py_exception(env, 1);

    Py_XDECREF(result);

EXIT:
    PyEval_ReleaseThread(jepThread->tstate);
}


// returns 1 if finished, 0 if not, throws exception otherwise
int pyembed_compile_string(JNIEnv *env,
                           intptr_t _jepThread,
                           char *str)
{
    PyObject       *code;
    int             ret = -1;
    JepThread      *jepThread;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return 0;
    }

    if (str == NULL) {
        return 0;
    }

    PyEval_AcquireThread(jepThread->tstate);

    code = Py_CompileString(str, "<stdin>", Py_single_input);

    if (code != NULL) {
        Py_DECREF(code);
        ret = 1;
    } else if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        ret = 0;
    } else {
        process_py_exception(env, 0);
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return ret;
}


intptr_t pyembed_create_module(JNIEnv *env,
                               intptr_t _jepThread,
                               char *str)
{
    PyObject       *module;
    JepThread      *jepThread;
    intptr_t        ret;
    PyObject       *key;

    ret = 0;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return 0;
    }

    if (str == NULL) {
        return 0;
    }

    PyEval_AcquireThread(jepThread->tstate);

    if (PyImport_AddModule(str) == NULL || process_py_exception(env, 1)) {
        goto EXIT;
    }

    PyImport_AddModule(str);
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

    if (process_py_exception(env, 0) || module == NULL) {
        ret = 0;
    } else {
        ret = (intptr_t) module;
    }

EXIT:
    PyEval_ReleaseThread(jepThread->tstate);

    return ret;
}


intptr_t pyembed_create_module_on(JNIEnv *env,
                                  intptr_t _jepThread,
                                  intptr_t _onModule,
                                  char *str)
{
    PyObject       *module, *onModule;
    JepThread      *jepThread;
    intptr_t        ret;
    PyObject       *globals;
    PyObject       *key;

    ret = 0;
    globals = 0;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return 0;
    }

    if (str == NULL) {
        return 0;
    }

    PyEval_AcquireThread(jepThread->tstate);

    onModule = (PyObject *) _onModule;
    if (!PyModule_Check(onModule)) {
        THROW_JEP(env, "Invalid onModule.");
        goto EXIT;
    }

    globals = PyModule_GetDict(onModule);
    Py_INCREF(globals);

    if (PyImport_AddModule(str) == NULL || process_py_exception(env, 1)) {
        goto EXIT;
    }

    PyImport_AddModule(str);
    module = PyImport_ImportModuleEx(str, globals, globals, NULL); /* new ref */

    key = PyString_FromString(str);
    PyDict_SetItem(globals,
                   key,
                   module);     /* ownership */
    Py_DECREF(key);
    Py_DECREF(module);

    if (process_py_exception(env, 0) || module == NULL) {
        ret = 0;
    } else {
        ret = (intptr_t) module;
    }

EXIT:
    Py_XDECREF(globals);
    PyEval_ReleaseThread(jepThread->tstate);
    return ret;
}


void pyembed_setloader(JNIEnv *env, intptr_t _jepThread, jobject cl)
{
    jobject    oldLoader = NULL;
    JepThread *jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return;
    }

    if (!cl) {
        return;
    }

    PyEval_AcquireThread(jepThread->tstate);
    Py_CLEAR(jepThread->fqnToPyJmethods);

    oldLoader = jepThread->classloader;
    if (oldLoader) {
        (*env)->DeleteGlobalRef(env, oldLoader);
    }

    jepThread->classloader = (*env)->NewGlobalRef(env, cl);
    PyEval_ReleaseThread(jepThread->tstate);
}
jobject pyembed_getvalue_on(JNIEnv *env,
                            intptr_t _jepThread,
                            intptr_t _onModule,
                            char *str)
{
    PyObject       *dict, *result, *onModule;
    jobject         ret = NULL;
    JepThread      *jepThread;

    result = 0;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return NULL;
    }

    if (str == NULL) {
        return NULL;
    }

    PyEval_AcquireThread(jepThread->tstate);

    if (process_py_exception(env, 1)) {
        goto EXIT;
    }

    onModule = (PyObject *) _onModule;
    if (!PyModule_Check(onModule)) {
        THROW_JEP(env, "pyembed_getvalue_on: Invalid onModule.");
        goto EXIT;
    }

    dict = PyModule_GetDict(onModule);
    Py_INCREF(dict);

    result = PyRun_String(str, Py_eval_input, dict, dict);      /* new ref */

    process_py_exception(env, 1);
    Py_DECREF(dict);

    if (result == NULL) {
        goto EXIT;    /* don't return, need to release GIL */
    }
    if (result == Py_None) {
        goto EXIT;
    }

    // convert results to jobject
    ret = PyObject_As_jobject(env, result, JOBJECT_TYPE);
    if (!ret) {
        process_py_exception(env, 1);
    }

EXIT:
    Py_XDECREF(result);
    PyEval_ReleaseThread(jepThread->tstate);
    return ret;
}


jobject pyembed_getvalue(JNIEnv *env, intptr_t _jepThread, char *str)
{
    PyObject       *result;
    jobject         ret = NULL;
    JepThread      *jepThread;

    result = NULL;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return NULL;
    }

    if (str == NULL) {
        return NULL;
    }

    PyEval_AcquireThread(jepThread->tstate);

    if (process_py_exception(env, 1)) {
        goto EXIT;
    }

    result = PyRun_String(str,  /* new ref */
                          Py_eval_input,
                          jepThread->globals,
                          jepThread->globals);

    process_py_exception(env, 1);

    if (result == NULL || result == Py_None) {
        goto EXIT;    /* don't return, need to release GIL */
    }

    // convert results to jobject
    ret = PyObject_As_jobject(env, result, JOBJECT_TYPE);
    if (!ret) {
        process_py_exception(env, 1);
    }

EXIT:
    Py_XDECREF(result);
    PyEval_ReleaseThread(jepThread->tstate);
    return ret;
}



jobject pyembed_getvalue_array(JNIEnv *env, intptr_t _jepThread, char *str)
{
    PyObject       *result;
    jobject         ret = NULL;
    JepThread      *jepThread;

    result = NULL;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return NULL;
    }

    if (str == NULL) {
        return NULL;
    }

    PyEval_AcquireThread(jepThread->tstate);

    if (process_py_exception(env, 1)) {
        goto EXIT;
    }

    result = PyRun_String(str,  /* new ref */
                          Py_eval_input,
                          jepThread->globals,
                          jepThread->globals);

    process_py_exception(env, 1);

    if (result == NULL || result == Py_None) {
        goto EXIT;    /* don't return, need to release GIL */
    }

#if PY_MAJOR_VERSION >= 3
    if (PyBytes_Check(result) == 0) {
        PyObject *temp = PyBytes_FromObject(result);
        if (process_py_exception(env, 1) || result == NULL) {
            goto EXIT;
        } else {
            Py_DECREF(result);
            result = temp;
        }
    }
#endif

    if (PyBytes_Check(result)) {
        void *s = (void*) PyBytes_AS_STRING(result);
        Py_ssize_t n = PyBytes_Size(result);
        ret = (*env)->NewByteArray(env, (jsize) n);
        (*env)->SetByteArrayRegion(env, ret, 0, (jsize) n, (jbyte *) s);
    } else {
        THROW_JEP(env, "Value is not a string.");
        goto EXIT;
    }


EXIT:
    Py_XDECREF(result);
    PyEval_ReleaseThread(jepThread->tstate);
    return ret;
}



void pyembed_run(JNIEnv *env,
                 intptr_t _jepThread,
                 char *file)
{
    JepThread     *jepThread;
    const char    *ext;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return;
    }

    PyEval_AcquireThread(jepThread->tstate);

    if (file != NULL) {
        FILE *script = fopen(file, "r");
        if (!script) {
            THROW_JEP(env, "Couldn't open script file.");
            goto EXIT;
        }

        // check if it's a pyc/pyo file
        ext = file + strlen(file) - 4;
        if (maybe_pyc_file(script, file, ext, 0)) {
            /* Try to run a pyc file. First, re-open in binary */
            fclose(script);
            if ((script = fopen(file, "rb")) == NULL) {
                THROW_JEP(env, "pyembed_run: Can't reopen .pyc file");
                goto EXIT;
            }

            /* Turn on optimization if a .pyo file is given */
            if (strcmp(ext, ".pyo") == 0) {
                Py_OptimizeFlag = 2;
            } else {
                Py_OptimizeFlag = 0;
            }

            pyembed_run_pyc(jepThread, script);
        } else {
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
    PyEval_ReleaseThread(jepThread->tstate);
}


// gratuitously copyied from pythonrun.c::run_pyc_file
static void pyembed_run_pyc(JepThread *jepThread,
                            FILE *fp)
{
    PyObject *co;
    PyObject *v;
    long magic;

    long PyImport_GetMagicNumber(void);

    magic = PyMarshal_ReadLongFromFile(fp);
    if (magic != PyImport_GetMagicNumber()) {
        PyErr_SetString(PyExc_RuntimeError, "Bad magic number in .pyc file");
        return;
    }
    (void) PyMarshal_ReadLongFromFile(fp);
    v = (PyObject *) (intptr_t) PyMarshal_ReadLastObjectFromFile(fp);
    if (v == NULL || !PyCode_Check(v)) {
        Py_XDECREF(v);
        PyErr_SetString(PyExc_RuntimeError, "Bad code object in .pyc file");
        return;
    }
    co = v;
#if PY_MAJOR_VERSION >= 3
    v = PyEval_EvalCode(co, jepThread->globals, jepThread->globals);
#else
    v = PyEval_EvalCode((PyCodeObject *) co, jepThread->globals,
                        jepThread->globals);
#endif
    Py_DECREF(co);
    Py_XDECREF(v);
}

/* Check whether a file maybe a pyc file: Look at the extension,
 the file type, and, if we may close it, at the first few bytes. */
// gratuitously copyied from pythonrun.c::run_pyc_file
static int maybe_pyc_file(FILE *fp, const char* filename, const char* ext,
                          int closeit)
{
    if (strcmp(ext, ".pyc") == 0 || strcmp(ext, ".pyo") == 0) {
        return 1;
    }

    /* Only look into the file if we are allowed to close it, since
     it then should also be seekable. */
    if (closeit) {
        /* Read only two bytes of the magic. If the file was opened in
         text mode, the bytes 3 and 4 of the magic (\r\n) might not
         be read as they are on disk. */
        unsigned int halfmagic = (unsigned int) PyImport_GetMagicNumber()
                                 & 0xFFFF;
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
        if (ftell(fp) == 0) {
            if (fread(buf, 1, 2, fp) == 2
                    && (buf[1] << 8 | buf[0]) == halfmagic) {
                ispyc = 1;
            }
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
    PyEval_AcquireThread(jepThread->tstate);                        \
                                                                    \
    pymodule = NULL;                                                \
    if(module != 0)                                                 \
        pymodule = (PyObject *) module;



void pyembed_setparameter_object(JNIEnv *env,
                                 intptr_t _jepThread,
                                 intptr_t module,
                                 const char *name,
                                 jobject value)
{
    PyObject      *pyjob;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;

    if (value == NULL) {
        Py_INCREF(Py_None);
        pyjob = Py_None;
    } else {
        pyjob = pyjobject_new(env, value);
    }

    if (pyjob) {
        if (pymodule == NULL) {
            PyObject *key = PyString_FromString(name);
            PyDict_SetItem(jepThread->globals,
                           key,
                           pyjob); /* ownership */
            Py_DECREF(key);
            Py_DECREF(pyjob);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyjob); // steals reference
        }
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return;
}


void pyembed_setparameter_array(JNIEnv *env,
                                intptr_t _jepThread,
                                intptr_t module,
                                const char *name,
                                jobjectArray obj)
{
    PyObject      *pyjob;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;

    if (obj == NULL) {
        Py_INCREF(Py_None);
        pyjob = Py_None;
    } else {
        pyjob = pyjarray_new(env, obj);
    }

    if (pyjob) {
        if (pymodule == NULL) {
            PyObject *key = PyString_FromString(name);
            PyDict_SetItem(jepThread->globals,
                           key,
                           pyjob); /* ownership */
            Py_DECREF(key);
            Py_DECREF(pyjob);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyjob); // steals reference
        }
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return;
}


void pyembed_setparameter_class(JNIEnv *env,
                                intptr_t _jepThread,
                                intptr_t module,
                                const char *name,
                                jclass value)
{
    PyObject      *pyjob;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;

    if (value == NULL) {
        Py_INCREF(Py_None);
        pyjob = Py_None;
    } else {
        pyjob = pyjobject_new_class(env, value);
    }

    if (pyjob) {
        if (pymodule == NULL) {
            PyObject *key = PyString_FromString(name);
            PyDict_SetItem(jepThread->globals,
                           key,
                           pyjob); /* ownership */
            Py_DECREF(key);
            Py_DECREF(pyjob);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyjob); // steals reference
        }
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return;
}


void pyembed_setparameter_string(JNIEnv *env,
                                 intptr_t _jepThread,
                                 intptr_t module,
                                 const char *name,
                                 const char *value)
{
    PyObject      *pyvalue;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;

    if (value == NULL) {
        Py_INCREF(Py_None);
        pyvalue = Py_None;
    } else {
        pyvalue = PyString_FromString(value);
    }

    if (pymodule == NULL) {
        PyObject *key = PyString_FromString(name);
        PyDict_SetItem(jepThread->globals,
                       key,
                       pyvalue); /* ownership */
        Py_DECREF(key);
        Py_DECREF(pyvalue);
    } else {
        PyModule_AddObject(pymodule,
                           (char *) name,
                           pyvalue); // steals reference
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return;
}


void pyembed_setparameter_int(JNIEnv *env,
                              intptr_t _jepThread,
                              intptr_t module,
                              const char *name,
                              int value)
{
    PyObject      *pyvalue;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;

    if ((pyvalue = Py_BuildValue("i", value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }

    if (pymodule == NULL) {
        PyObject *key = PyString_FromString(name);
        PyDict_SetItem(jepThread->globals,
                       key,
                       pyvalue); /* ownership */
        Py_DECREF(key);
        Py_DECREF(pyvalue);
    } else {
        PyModule_AddObject(pymodule,
                           (char *) name,
                           pyvalue); // steals reference
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return;
}


void pyembed_setparameter_long(JNIEnv *env,
                               intptr_t _jepThread,
                               intptr_t module,
                               const char *name,
                               PY_LONG_LONG value)
{
    PyObject      *pyvalue;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;

    if ((pyvalue = PyLong_FromLongLong(value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }

    if (pymodule == NULL) {
        PyObject *key = PyString_FromString(name);
        PyDict_SetItem(jepThread->globals,
                       key,
                       pyvalue); /* ownership */
        Py_DECREF(key);
        Py_DECREF(pyvalue);
    } else {
        PyModule_AddObject(pymodule,
                           (char *) name,
                           pyvalue); // steals reference
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return;
}


void pyembed_setparameter_double(JNIEnv *env,
                                 intptr_t _jepThread,
                                 intptr_t module,
                                 const char *name,
                                 double value)
{
    PyObject      *pyvalue;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;

    if ((pyvalue = PyFloat_FromDouble(value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }

    if (pymodule == NULL) {
        PyObject *key = PyString_FromString(name);
        PyDict_SetItem(jepThread->globals,
                       key,
                       pyvalue); /* ownership */
        Py_DECREF(key);
        Py_DECREF(pyvalue);
    } else {
        PyModule_AddObject(pymodule,
                           (char *) name,
                           pyvalue); // steals reference
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return;
}


void pyembed_setparameter_float(JNIEnv *env,
                                intptr_t _jepThread,
                                intptr_t module,
                                const char *name,
                                float value)
{
    PyObject      *pyvalue;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;

    if ((pyvalue = PyFloat_FromDouble((double) value)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory.");
        return;
    }

    if (pymodule == NULL) {
        PyObject *key = PyString_FromString(name);
        PyDict_SetItem(jepThread->globals,
                       key,
                       pyvalue); /* ownership */
        Py_DECREF(key);
        Py_DECREF(pyvalue);
    } else {
        PyModule_AddObject(pymodule,
                           (char *) name,
                           pyvalue); // steals reference
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return;
}
