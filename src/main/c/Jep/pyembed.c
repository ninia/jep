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

#ifdef __unix__
    #ifdef PYTHON_LDLIBRARY
        /* see comments near dlopen() in pyembed_startup */
        #include <dlfcn.h>
    #endif
#endif

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
static PyObject* mainThreadModulesLock = NULL;

static int pyembed_is_version_unsafe(void);
static void handle_startup_exception(JNIEnv*, const char*);

static PyObject* pyembed_findclass(PyObject*, PyObject*);
static PyObject* pyembed_forname(PyObject*, PyObject*);
static PyObject* pyembed_jproxy(PyObject*, PyObject*);

static int maybe_pyc_file(FILE*, const char*, const char*, int);
static void pyembed_run_pyc(JepThread*, FILE*);

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

static PyObject* initjep(JNIEnv *env, jboolean hasSharedModules)
{
    PyObject *modjep;

#if PY_MAJOR_VERSION >= 3
    PyObject *sysmodules;
    modjep = PyModule_Create(&jep_module_def);
    if (modjep == NULL) {
        handle_startup_exception(env, "Couldn't create module _jep");
        return NULL;
    }
    sysmodules = PyImport_GetModuleDict();
    if (PyDict_SetItemString(sysmodules, "_jep", modjep) == -1) {
        handle_startup_exception(env, "Couldn't set _jep on sys.modules");
        return NULL;
    }
#else
    modjep = PyImport_AddModule("_jep");
    if (modjep == NULL) {
        handle_startup_exception(env, "Couldn't add module _jep");
        return NULL;
    }
    modjep = Py_InitModule((char *) "_jep", jep_methods);
    if (modjep == NULL) {
        handle_startup_exception(env, "Couldn't init module _jep");
        return NULL;
    }
#endif
    modjep = PyImport_ImportModule("_jep");
    if (modjep == NULL) {
        handle_startup_exception(env, "Couldn't import module _jep");
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
        if (hasSharedModules) {
            Py_INCREF(mainThreadModules);
            PyModule_AddObject(modjep, "mainInterpreterModules", mainThreadModules);
            Py_INCREF(mainThreadModulesLock);
            PyModule_AddObject(modjep, "mainInterpreterModulesLock", mainThreadModulesLock);
        }
    }

    return modjep;
}

void pyembed_preinit(JNIEnv *env,
                     jint noSiteFlag,
                     jint noUserSiteDirectory,
                     jint ignoreEnvironmentFlag,
                     jint verboseFlag,
                     jint optimizeFlag,
                     jint dontWriteBytecodeFlag,
                     jint hashRandomizationFlag,
                     jstring pythonHome)
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
    if (pythonHome) {
        const char* homeAsUTF = (*env)->GetStringUTFChars(env, pythonHome, NULL);
#if PY_MAJOR_VERSION >= 3
#if PY_MAJOR_VERSION > 3 || PY_MINOR_VERSION >= 5
        wchar_t* homeForPython = Py_DecodeLocale(homeAsUTF, NULL);
#else
        int length = (*env)->GetStringUTFLength(env, pythonHome);
        wchar_t* homeForPython = malloc((length + 1) * sizeof(wchar_t));
        mbstowcs(homeForPython, homeAsUTF, length + 1);
#endif
#else
        int length = (*env)->GetStringUTFLength(env, pythonHome);
        char* homeForPython = malloc(length);
        strncpy(homeForPython, homeAsUTF, length);
#endif
        (*env)->ReleaseStringUTFChars(env, pythonHome, homeAsUTF);

        Py_SetPythonHome(homeForPython);
        // Python documentation says that the string should not be changed for
        // the duration of the program so it can never be freed.
    }
}

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
static int pyjtypes_ready(void)
{
    // start at the top with object
    if (PyType_Ready(&PyJObject_Type) < 0) {
        return -1;
    }
    if (!PyJClass_Type.tp_base) {
        PyJClass_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJClass_Type) < 0) {
        return -1;
    }

    // next do number
    if (!PyJNumber_Type.tp_base) {
        PyJNumber_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJNumber_Type) < 0) {
        return -1;
    }

    // next do iterable
    if (!PyJIterable_Type.tp_base) {
        PyJIterable_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJIterable_Type) < 0) {
        return -1;
    }

    // next do iterator
    if (!PyJIterator_Type.tp_base) {
        PyJIterator_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJIterator_Type) < 0) {
        return -1;
    }

    // next do collection
    if (!PyJCollection_Type.tp_base) {
        PyJCollection_Type.tp_base = &PyJIterable_Type;
    }
    if (PyType_Ready(&PyJCollection_Type) < 0) {
        return -1;
    }

    // next do list
    if (!PyJList_Type.tp_base) {
        PyJList_Type.tp_base = &PyJCollection_Type;
    }
    if (PyType_Ready(&PyJList_Type) < 0) {
        return -1;
    }

    // next do map
    if (!PyJMap_Type.tp_base) {
        PyJMap_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJMap_Type) < 0) {
        return -1;
    }

    if (!PyJBuffer_Type.tp_base) {
        PyJBuffer_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJBuffer_Type) < 0) {
        return -1;
    }

    // last do autocloseable
    if (!PyJAutoCloseable_Type.tp_base) {
        PyJAutoCloseable_Type.tp_base = &PyJObject_Type;
    }
    if (PyType_Ready(&PyJAutoCloseable_Type) < 0) {
        return -1;
    }

    return 0;
}

static void handle_startup_exception(JNIEnv *env, const char* excMsg)
{
    jclass excClass = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
    if (excClass != NULL) {
        (*env)->ThrowNew(env, excClass, excMsg);
    }
}


void pyembed_startup(JNIEnv *env, jobjectArray sharedModulesArgv)
{
    PyObject* sysModule       = NULL;
    PyObject* threadingModule = NULL;
    PyObject* lockCreator     = NULL;

#ifdef _DLFCN_H
    /*
     * In some Linux distros, Python modules are compiled without a dependency
     * on libpython, instead they rely on the Python symbols to be available
     * globally. Unfortunatly JNI does not load the libraries globally so the
     * symbols are not available which causes those Python modules to fail to
     * load. To get around this dlopen is used to promote libpython into the
     * global namespace.
     *
     * This is most notably a problem on Ubuntu which statically links libpython
     * into the python executable which means that all modules rely on Python
     * symbols being available globally.
     *
     * An alternative mechanism on Linux to get the libpython symbols globally
     * is to use LD_PRELOAD to load libpython, this still might be necessary
     * if dlopen fails for some reason.
     */
    void* dlresult = dlopen(PYTHON_LDLIBRARY,
                            RTLD_LAZY | RTLD_NOLOAD | RTLD_GLOBAL);
    if (dlresult) {
        // The dynamic linker maintains reference counts so closing it is a no-op.
        dlclose(dlresult);
    } else {
        /*
         * Ignore errors and hope that the library is loaded globally or the
         * extensions are linked. If developers need to debug the cause they
         * should print the result of dlerror.
         */
        dlerror();
    }
#endif

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

    if (pyembed_is_version_unsafe()) {
        return;
    }

    Py_Initialize();
    PyEval_InitThreads();

    if (pyjtypes_ready()) {
        handle_startup_exception(env, "Failed to initialize PyJTypes");
        return;
    }

    /*
     * Save a global reference to the sys.modules form the main thread to
     * support shared modules
     */
    sysModule = PyImport_ImportModule("sys");
    if (sysModule == NULL) {
        handle_startup_exception(env, "Failed to import sys module");
        return;
    }

    mainThreadModules = PyObject_GetAttrString(sysModule, "modules");
    if (mainThreadModules == NULL) {
        handle_startup_exception(env, "Failed to get sys.modules");
        return;
    }
    Py_DECREF(sysModule);

    threadingModule = PyImport_ImportModule("threading");
    if (threadingModule == NULL) {
        handle_startup_exception(env, "Failed to import threading module");
        return;
    }
    lockCreator = PyObject_GetAttrString(threadingModule, "Lock");
    if (lockCreator == NULL) {
        handle_startup_exception(env, "Failed to get Lock attribute");
        return;
    }

    mainThreadModulesLock = PyObject_CallObject(lockCreator, NULL);
    if (mainThreadModulesLock == NULL) {
        handle_startup_exception(env, "Failed to get main thread modules lock");
        return;
    }
    Py_DECREF(threadingModule);
    Py_DECREF(lockCreator);

    // save a pointer to the main PyThreadState object
    mainThreadState = PyThreadState_Get();

    /*
     * Workaround for shared modules sys.argv.  Set sys.argv on the main thread.
     * See github issue #81.
     */
    if (sharedModulesArgv != NULL) {
#if PY_MAJOR_VERSION < 3
        char **argv = NULL;
        jsize count = 0;
        int i       = 0;

        count = (*env)->GetArrayLength(env, sharedModulesArgv);
        (*env)->PushLocalFrame(env, count * 2);
        argv = (char**) malloc(count * sizeof(char*));
        for (i = 0; i < count; i++) {
            char* arg = NULL;

            jstring jarg = (*env)->GetObjectArrayElement(env, sharedModulesArgv, i);
            if (jarg == NULL) {
                PyEval_ReleaseThread(mainThreadState);
                (*env)->PopLocalFrame(env, NULL);
                THROW_JEP(env, "Received null argv.");
                return;
            }
            arg = (char*) (*env)->GetStringUTFChars(env, jarg, NULL);
            argv[i] = arg;
        }

        PySys_SetArgvEx(count, argv, 0);

        // free memory
        for (i = 0; i < count; i++) {
            jstring jarg = (*env)->GetObjectArrayElement(env, sharedModulesArgv, i);
            (*env)->ReleaseStringUTFChars(env, jarg, argv[i]);
        }
        free(argv);
        (*env)->PopLocalFrame(env, NULL);

#else
        wchar_t **argv = NULL;
        jsize count = 0;
        int i       = 0;

        count = (*env)->GetArrayLength(env, sharedModulesArgv);
        (*env)->PushLocalFrame(env, count * 2);
        argv = (wchar_t**) malloc(count * sizeof(wchar_t*));
        for (i = 0; i < count; i++) {
            char* arg     = NULL;
            wchar_t* argt = NULL;

            jstring jarg = (*env)->GetObjectArrayElement(env, sharedModulesArgv, i);
            if (jarg == NULL) {
                PyEval_ReleaseThread(mainThreadState);
                (*env)->PopLocalFrame(env, NULL);
                THROW_JEP(env, "Received null argv.");
                return;
            }
            arg = (char*) (*env)->GetStringUTFChars(env, jarg, NULL);
            argt = malloc((strlen(arg) + 1) * sizeof(wchar_t));
            mbstowcs(argt, arg, strlen(arg) + 1);
            (*env)->ReleaseStringUTFChars(env, jarg, arg);
            argv[i] = argt;
        }

        PySys_SetArgvEx(count, argv, 0);

        // free memory
        for (i = 0; i < count; i++) {
            free(argv[i]);
        }
        free(argv);
        (*env)->PopLocalFrame(env, NULL);

#endif
    }

    PyEval_ReleaseThread(mainThreadState);
}


/*
 * Verify the Python major.minor at runtime matches the Python major.minor
 * that Jep was built against.  If they don't match, refuse to initialize Jep
 * and instead throw an exception because a mismatch could cause a JVM crash.
 */
int pyembed_is_version_unsafe(void)
{
    const char *pyversion = NULL;
    char       *version   = NULL;
    char       *major     = NULL;
    char       *minor     = NULL;
    int         i         = 0;

    pyversion = Py_GetVersion();
    version = malloc(sizeof(char) * (strlen(pyversion) + 1));
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
        process_py_exception(env);
    }
    (*env)->ReleaseStringUTFChars(env, module, moduleName);
    PyEval_ReleaseThread(mainThreadState);
}

#if PY_MAJOR_VERSION < 3
/*
 * In python 2.7 when a module is shared between threads it will be run in
 * restricted mode because the __builtins__ attribute of the module does not
 * match the running interpreter. This causes some functionality to break.
 * To get around this limitation the builtins module is shared between all the
 * interpreters that are using shared modules.
 *
 * This is not necessary in python 3 because restricted mode was removed.
 */

void assignBuiltins(PyObject* modules, PyObject* builtins)
{
    PyObject *key, *value;
    Py_ssize_t pos = 0;

    while (PyDict_Next(modules, &pos, &key, &value)) {
        PyObject* moddict = PyModule_GetDict(value);
        if (moddict) {
            PyDict_SetItemString(moddict, "__builtins__", builtins);
        }
    }
}

void shareBuiltins(JepThread *jepThread)
{
    PyInterpreterState* interp = jepThread->tstate->interp;

    PyObject* sharedBuiltinModule = PyDict_GetItemString(mainThreadModules,
                                    "__builtin__");
    PyObject* sharedBuiltinDict   = PyModule_GetDict(sharedBuiltinModule);

    PyObject* originalBuiltinModule = PyDict_GetItemString(interp->modules,
                                      "__builtin__");
    PyObject* originalBuiltinDict   = interp->builtins;

    Py_INCREF(sharedBuiltinDict);
    interp->builtins = sharedBuiltinDict;
    Py_DECREF(originalBuiltinDict);

    // Incref to make sure this isn't collected when it is replaced, it
    // will be restored when the itnterpreter is closed.
    Py_INCREF(originalBuiltinModule);

    PyDict_SetItemString(interp->modules, "__builtin__", sharedBuiltinModule);
    assignBuiltins(interp->modules, sharedBuiltinDict);

    jepThread->originalBuiltins = originalBuiltinModule;
}

void unshareBuiltins(JepThread *jepThread)
{
    PyInterpreterState* interp = jepThread->tstate->interp;

    PyObject* originalBuiltinModule = jepThread->originalBuiltins;
    PyObject* originalBuiltinDict   = PyModule_GetDict(originalBuiltinModule);

    PyObject* sharedBuiltinDict   = interp->builtins;

    Py_INCREF(originalBuiltinDict);
    interp->builtins = originalBuiltinDict;
    Py_DECREF(sharedBuiltinDict);

    PyDict_SetItemString(interp->modules, "__builtin__", originalBuiltinModule);

    assignBuiltins(interp->modules, originalBuiltinDict);

    jepThread->originalBuiltins = NULL;
    Py_DECREF(originalBuiltinModule);
}

#endif

intptr_t pyembed_thread_init(JNIEnv *env, jobject cl, jobject caller,
                             jboolean hasSharedModules, jboolean usesubinterpreter)
{
    JepThread *jepThread;
    PyObject  *tdict, *globals;
    if (cl == NULL) {
        THROW_JEP(env, "Invalid Classloader.");
        return 0;
    }


    /*
     * Do not use PyMem_Malloc because PyGILState_Check() returns false since
     * the mainThreadState was created on a different thread. When python is
     * compiled with debug it checks the state and fails.
     */
    jepThread = malloc(sizeof(JepThread));
    if (!jepThread) {
        THROW_JEP(env, "Out of memory.");
        return 0;
    }

    if (usesubinterpreter) {
        PyEval_AcquireThread(mainThreadState);

        jepThread->tstate = Py_NewInterpreter();
#if PY_MAJOR_VERSION < 3
        if (hasSharedModules) {
            shareBuiltins(jepThread);
        } else {
            jepThread->originalBuiltins = NULL;
        }
#endif

        /*
         * Py_NewInterpreter() seems to take the thread state, but we're going to
         * save/release and reacquire it since that doesn't seem documented
         */
        PyEval_SaveThread();
    } else {
        jepThread->tstate = PyThreadState_New(mainThreadState->interp);
#if PY_MAJOR_VERSION < 3
        jepThread->originalBuiltins = NULL;
#endif
    }
    PyEval_AcquireThread(jepThread->tstate);

    // store java.lang.Class objects for later use.
    // it's a noop if already done, but to synchronize, have the lock first
    if (!cache_frequent_classes(env)) {
        printf("WARNING: Failed to get and cache frequent class types!\n");
    }
    if (!cache_primitive_classes(env)) {
        printf("WARNING: Failed to get and cache primitive class types!\n");
    }

    if (usesubinterpreter) {
        PyObject *mod_main = PyImport_AddModule("__main__");    /* borrowed */
        if (mod_main == NULL) {
            THROW_JEP(env, "Couldn't add module __main__.");
            PyEval_ReleaseThread(jepThread->tstate);
            return 0;
        }
        globals = PyModule_GetDict(mod_main);
        Py_INCREF(globals);
    } else {
        globals = PyDict_New();
        PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
    }

    // init static module
    jepThread->modjep          = initjep(env, hasSharedModules);
    jepThread->globals         = globals;
    jepThread->env             = env;
    jepThread->classloader     = (*env)->NewGlobalRef(env, cl);
    jepThread->caller          = (*env)->NewGlobalRef(env, caller);
    jepThread->fqnToPyJAttrs = NULL;

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
#if PY_MAJOR_VERSION < 3
    if (jepThread->originalBuiltins) {
        unshareBuiltins(jepThread);
    }
#endif
    key = PyString_FromString(DICT_KEY);
    if ((tdict = PyThreadState_GetDict()) != NULL && key != NULL) {
        PyDict_DelItem(tdict, key);
    }
    Py_DECREF(key);

    Py_CLEAR(jepThread->globals);
    Py_CLEAR(jepThread->fqnToPyJAttrs);
    Py_CLEAR(jepThread->modjep);

    if (jepThread->classloader) {
        (*env)->DeleteGlobalRef(env, jepThread->classloader);
    }
    if (jepThread->caller) {
        (*env)->DeleteGlobalRef(env, jepThread->caller);
    }
    if (jepThread->tstate->interp == mainThreadState->interp) {
        PyThreadState_Clear(jepThread->tstate);
        PyEval_ReleaseThread(jepThread->tstate);
        PyThreadState_Delete(jepThread->tstate);
    } else {
        Py_EndInterpreter(jepThread->tstate);
	PyThreadState_Swap(mainThreadState);
        PyEval_ReleaseThread(mainThreadState);
    }
    free(jepThread);
}


JNIEnv* pyembed_get_env(void)
{
    JavaVM *jvm;
    JNIEnv *env;
    jsize nVMs;


    JNI_GetCreatedJavaVMs(&jvm, 1, &nVMs);
    /*
     * If the thread is already part of the JVM, the daemon status is not
     * changed. If this is a new thread, started by Python then this tells
     * Java to allow the process to exit even if the thread is still attached.
     * Since there are no hooks to detach the thread later daemon is the only
     * way to let the process exit normally.
     */
    (*jvm)->AttachCurrentThreadAsDaemon(jvm, (void**) &env, NULL);

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
    Py_XDECREF(key);
    if (!ret && !PyErr_Occurred()) {
        PyErr_SetString(PyExc_RuntimeError,
                        "No Jep instance available on current thread.");
    }
    return ret;
}

static PyObject* pyembed_jproxy(PyObject *self, PyObject *args)
{
    JepThread     *jepThread;
    JNIEnv        *env = NULL;
    PyObject      *pytarget;
    PyObject      *interfaces;
    PyObject      *result;
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
        const char *str;
        jstring     jstr;
        PyObject   *item;

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
    proxy = jep_Proxy_newProxyInstance(env,
                                       jepThread->caller,
                                       (jlong) (intptr_t) pytarget,
                                       classes);
    (*env)->DeleteLocalRef(env, classes);
    if (process_java_exception(env) || !proxy) {
        return NULL;
    }

    // make sure target doesn't get garbage collected
    Py_INCREF(pytarget);

    result = PyJObject_Wrap(env, proxy, NULL);
    (*env)->DeleteLocalRef(env, proxy);
    return result;
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

    jstr = (*env)->NewStringUTF(env, (const char *) name);
    if (process_java_exception(env) || !jstr) {
        return NULL;
    }

    objclazz = java_lang_ClassLoader_loadClass(env, cl, jstr);
    (*env)->DeleteLocalRef(env, jstr);
    if (process_java_exception(env) || !objclazz) {
        return NULL;
    }
    result = PyJClass_Wrap(env, objclazz);
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

    result = PyJClass_Wrap(env, clazz);
    (*env)->DeleteLocalRef(env, clazz);
    return result;
}


/*
 * Invoke callable object.  Hold the thread state lock before calling.
 */
jobject pyembed_invoke_as(JNIEnv *env,
                          PyObject *callable,
                          jobjectArray args,
                          jobject kwargs,
                          jclass expectedType)
{
    jobject        ret      = NULL;
    PyObject      *pyargs   = NULL;    /* a tuple */
    PyObject      *pykwargs = NULL;    /* a dictionary */
    PyObject      *pyret    = NULL;
    int            arglen   = 0;
    Py_ssize_t     i        = 0;

    if (!PyCallable_Check(callable)) {
        THROW_JEP(env, "pyembed:invoke Invalid callable.");
        return NULL;
    }

    if (args != NULL) {
        arglen = (*env)->GetArrayLength(env, args);
        pyargs = PyTuple_New(arglen);
    } else {
        // pyargs should be a Tuple of size 0 if no args
        pyargs = PyTuple_New(arglen);
    }

    // convert Java arguments to a Python tuple
    for (i = 0; i < arglen; i++) {
        jobject   val;
        PyObject *pyval;

        val = (*env)->GetObjectArrayElement(env, args, (jsize) i);
        if ((*env)->ExceptionCheck(env)) { /* careful, NULL is okay */
            goto EXIT;
        }

        pyval = jobject_As_PyObject(env, val);
        if (!pyval) {
            goto EXIT;
        }

        PyTuple_SET_ITEM(pyargs, i, pyval); /* steals */
        if (val) {
            (*env)->DeleteLocalRef(env, val);
        }
    }

    // convert Java arguments to a Python dictionary
    if (kwargs != NULL) {
        jobject entrySet;
        jobject itr;

        pykwargs = PyDict_New();
        entrySet = java_util_Map_entrySet(env, kwargs);
        if ((*env)->ExceptionCheck(env)) {
            goto EXIT;
        }
        itr = java_lang_Iterable_iterator(env, entrySet);
        if ((*env)->ExceptionCheck(env)) {
            goto EXIT;
        }

        while (java_util_Iterator_hasNext(env, itr)) {
            jobject  next;
            jobject  key;
            jobject  value;
            PyObject *pykey;
            PyObject *pyval;

            next = java_util_Iterator_next(env, itr);
            if (!next) {
                if (!(*env)->ExceptionCheck(env)) {
                    THROW_JEP(env, "Map.entrySet().iterator().next() returned null");
                }
                goto EXIT;
            }

            // convert Map.Entry's key to a PyObject*
            key = java_util_Map_Entry_getKey(env, next);
            if ((*env)->ExceptionCheck(env)) {
                goto EXIT;
            }
            pykey = jobject_As_PyObject(env, key);
            if (!pykey) {
                goto EXIT;
            }

            // convert Map.Entry's value to a PyObject*
            value = java_util_Map_Entry_getValue(env, next);
            if ((*env)->ExceptionCheck(env)) {
                Py_XDECREF(pykey);
                goto EXIT;
            }
            pyval = jobject_As_PyObject(env, value);
            if (!pyval) {
                Py_DECREF(pykey);
                goto EXIT;
            }

            if (PyDict_SetItem(pykwargs, pykey, pyval)) {
                process_py_exception(env);
                Py_DECREF(pykey);
                Py_DECREF(pyval);
                goto EXIT;
            }
            Py_DECREF(pykey);
            Py_DECREF(pyval);

            (*env)->DeleteLocalRef(env, next);
            if (key) {
                (*env)->DeleteLocalRef(env, key);
            }
            if (value) {
                (*env)->DeleteLocalRef(env, value);
            }
        }
    } // end of while loop

    // if hasNext() threw an exception
    if ((*env)->ExceptionCheck(env)) {
        goto EXIT;
    }

    pyret = PyObject_Call(callable, pyargs, pykwargs);
    if (process_py_exception(env) || !pyret) {
        goto EXIT;
    }

    // handles errors
    ret = PyObject_As_jobject(env, pyret, expectedType);
    if (!ret) {
        process_py_exception(env);
    }

EXIT:
    Py_CLEAR(pyargs);
    Py_CLEAR(pykwargs);
    Py_XDECREF(pyret);

    return ret;
}

jobject pyembed_invoke(JNIEnv *env,
                       PyObject *callable,
                       jobjectArray args,
                       jobject kwargs)
{
    return pyembed_invoke_as(env, callable, args, kwargs, JOBJECT_TYPE);
}


jobject pyembed_invoke_method_as(JNIEnv *env,
                                 intptr_t _jepThread,
                                 const char *cname,
                                 jobjectArray args,
                                 jobject kwargs,
                                 jclass expectedType)
{
    PyObject  *callable;
    JepThread *jepThread;
    jobject    ret = NULL;

    jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return ret;
    }

    PyEval_AcquireThread(jepThread->tstate);

    callable = PyDict_GetItemString(jepThread->globals, cname);
    if (!callable) {
        /* Not a global */
        char* dot = strchr(cname, '.');
        if (dot && (dot - cname) < 63) {
            /* dot notation indicates it is an attribute */
            char globalName[64];
            PyObject* obj;
            strncpy(globalName, cname, dot - cname);
            globalName[dot - cname] = '\0';
            obj = PyDict_GetItemString(jepThread->globals, globalName);
            if (obj) {
                callable = PyObject_GetAttrString(obj, dot + 1);
                if (callable) {
                    ret = pyembed_invoke_as(env, callable, args, kwargs, expectedType);
                    Py_DECREF(callable);
                } else {
                    process_py_exception(env);
                }
            } else {
                char errorBuf[128];
                snprintf(errorBuf, 128, "Unable to find object with name: %s", globalName);
                THROW_JEP(env, errorBuf);
            }
        } else {
            char errorBuf[128];
            snprintf(errorBuf, 128, "Unable to find object with name: %s", cname);
            THROW_JEP(env, errorBuf);
        }
    } else if (!process_py_exception(env)) {
        ret = pyembed_invoke_as(env, callable, args, kwargs, expectedType);
    }

    PyEval_ReleaseThread(jepThread->tstate);

    return ret;
}

jobject pyembed_invoke_method(JNIEnv *env,
                              intptr_t _jepThread,
                              const char *cname,
                              jobjectArray args,
                              jobject kwargs)
{
    return pyembed_invoke_method_as(env, _jepThread, cname, args, kwargs,
                                    JOBJECT_TYPE);
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

    if (process_py_exception(env)) {
        goto EXIT;
    }

    result = PyRun_String(str,  /* new ref */
                          Py_single_input,
                          jepThread->globals,
                          jepThread->globals);

    // c programs inside some java environments may get buffered output
    fflush(stdout);
    fflush(stderr);

    process_py_exception(env);

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
        process_py_exception(env);
    }

    PyEval_ReleaseThread(jepThread->tstate);
    return ret;
}

void pyembed_exec(JNIEnv *env, intptr_t _jepThread, char *str)
{
    PyObject  *result = NULL;
    JepThread *jepThread = (JepThread *) _jepThread;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return;
    }

    if (str == NULL) {
        return;
    }

    PyEval_AcquireThread(jepThread->tstate);

    result = PyRun_String(str, Py_file_input, jepThread->globals,
                          jepThread->globals);
    if (result) {
        // Result is expected to be Py_None.
        Py_DECREF(result);
    } else {
        process_py_exception(env);
    }

    PyEval_ReleaseThread(jepThread->tstate);
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
    Py_CLEAR(jepThread->fqnToPyJAttrs);

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

    if (process_py_exception(env)) {
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

    process_py_exception(env);
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
        process_py_exception(env);
    }

EXIT:
    Py_XDECREF(result);
    PyEval_ReleaseThread(jepThread->tstate);
    return ret;
}


jobject pyembed_getvalue(JNIEnv *env, intptr_t _jepThread, char *str,
                         jclass clazz)
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

    if (process_py_exception(env)) {
        goto EXIT;
    }

    result = PyRun_String(str,  /* new ref */
                          Py_eval_input,
                          jepThread->globals,
                          jepThread->globals);

    process_py_exception(env);

    if (result == NULL || result == Py_None) {
        goto EXIT;    /* don't return, need to release GIL */
    }

    // convert results to jobject
    ret = PyObject_As_jobject(env, result, clazz);
    if (!ret) {
        process_py_exception(env);
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

    if (process_py_exception(env)) {
        goto EXIT;
    }

    result = PyRun_String(str,  /* new ref */
                          Py_eval_input,
                          jepThread->globals,
                          jepThread->globals);

    process_py_exception(env);

    if (result == NULL || result == Py_None) {
        goto EXIT;    /* don't return, need to release GIL */
    }

#if PY_MAJOR_VERSION >= 3
    if (PyBytes_Check(result) == 0) {
        PyObject *temp = PyBytes_FromObject(result);
        if (process_py_exception(env) || result == NULL) {
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
        process_py_exception(env);
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
#if PY_MAJOR_VERSION >= 3
    // Python 3.3 added an extra long containing the size of the source.
    // https://github.com/python/cpython/commit/5136ac0ca21a05691978df8d0650f902c8ca3463
    (void) PyMarshal_ReadLongFromFile(fp);
#if PY_MAJOR_VERSION > 3 || PY_MINOR_VERSION >= 7
    // PEP 552 added another long
    (void) PyMarshal_ReadLongFromFile(fp);
#endif
#endif
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
                    && ((unsigned int)buf[1] << 8 | buf[0]) == halfmagic) {
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

    pyjob = jobject_As_PyObject(env, value);

    if (pyjob) {
        if (pymodule == NULL) {
            PyDict_SetItemString(jepThread->globals,
                                 name,
                                 pyjob); /* ownership */
            Py_DECREF(pyjob);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyjob); // steals reference
        }
    }
    process_py_exception(env);

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
            PyDict_SetItemString(jepThread->globals,
                                 name,
                                 pyjob); /* ownership */
            Py_DECREF(pyjob);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyjob); // steals reference
        }
    }
    process_py_exception(env);

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
        pyjob = PyJClass_Wrap(env, value);
    }

    if (pyjob) {
        if (pymodule == NULL) {
            PyDict_SetItemString(jepThread->globals,
                                 name,
                                 pyjob); /* ownership */
            Py_DECREF(pyjob);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyjob); // steals reference
        }
    }
    process_py_exception(env);

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

    if (pyvalue) {
        if (pymodule == NULL) {
            PyDict_SetItemString(jepThread->globals,
                                 name,
                                 pyvalue); /* ownership */
            Py_DECREF(pyvalue);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyvalue); // steals reference
        }
    }
    process_py_exception(env);

    PyEval_ReleaseThread(jepThread->tstate);
    return;
}

void pyembed_setparameter_bool(JNIEnv *env,
                               intptr_t _jepThread,
                               intptr_t module,
                               const char *name,
                               jboolean value)
{
    PyObject      *pyvalue;
    PyObject      *pymodule;

    // does common things
    GET_COMMON;

    pyvalue = jboolean_As_PyObject(value);

    if (pyvalue) {
        if (pymodule == NULL) {
            PyDict_SetItemString(jepThread->globals,
                                 name,
                                 pyvalue); /* ownership */
            Py_DECREF(pyvalue);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyvalue); // steals reference
        }
    }
    process_py_exception(env);

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

    pyvalue = jint_As_PyObject(value);

    if (pyvalue) {
        if (pymodule == NULL) {
            PyDict_SetItemString(jepThread->globals,
                                 name,
                                 pyvalue); /* ownership */
            Py_DECREF(pyvalue);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyvalue); // steals reference
        }
    }
    process_py_exception(env);

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

    pyvalue = jlong_As_PyObject(value);

    if (pyvalue) {
        if (pymodule == NULL) {
            PyDict_SetItemString(jepThread->globals,
                                 name,
                                 pyvalue); /* ownership */
            Py_DECREF(pyvalue);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyvalue); // steals reference
        }
    }
    process_py_exception(env);

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

    pyvalue = jdouble_As_PyObject(value);

    if (pyvalue) {
        if (pymodule == NULL) {
            PyDict_SetItemString(jepThread->globals,
                                 name,
                                 pyvalue); /* ownership */
            Py_DECREF(pyvalue);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyvalue); // steals reference
        }
    }
    process_py_exception(env);

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

    pyvalue = jfloat_As_PyObject((double) value);

    if (pyvalue) {
        if (pymodule == NULL) {
            PyDict_SetItemString(jepThread->globals,
                                 name,
                                 pyvalue); /* ownership */
            Py_DECREF(pyvalue);
        } else {
            PyModule_AddObject(pymodule,
                               (char *) name,
                               pyvalue); // steals reference
        }
    }
    process_py_exception(env);

    PyEval_ReleaseThread(jepThread->tstate);
    return;
}
