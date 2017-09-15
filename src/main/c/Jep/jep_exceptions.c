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

static PyObject* pyerrtype_from_throwable(JNIEnv*, jthrowable);

// exception handling
jmethodID jepExcInitStrLong   = 0;
jmethodID jepExcInitStrThrow  = 0;
jmethodID stackTraceElemInit  = 0;

/*
 * Converts a Python exception to a JepException.  Returns true if an
 * exception was processed.  This can also handle a Java Throwable embedded
 * in a Python exception if process_java_exception(...) was called.
 */
int process_py_exception(JNIEnv *env)
{
    PyObject *ptype, *pvalue, *ptrace, *pystack = NULL;
    PyObject *message = NULL;
    char *m = NULL;
    PyJObject *jexc = NULL;
    jobject jepException = NULL;
    jstring jmsg;

    if (!PyErr_Occurred()) {
        return 0;
    }

    // let's not turn this into a Java exception if the user exited
    if (PyErr_ExceptionMatches(PyExc_SystemExit)) {
        /*
         * If you look at the CPython source code, the following line will
         * trigger a clean exit from Python.
         */
        PyErr_PrintEx(1);
        return 0;
    }

    if ((*env)->ExceptionOccurred(env)) {
        /*
         * There's a bug in Jep somewhere, hopefully this printout will help
         * diagnose it.  Then clear the error so the code can attempt to
         * continue on.
         */
        printf("WARNING: Jep internal error. Java exception detected at start of process_py_exception():\n");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }

    /*
     * If there's a trace, we'll try and attach it as the cause on the
     * JepException so the application can determine if/how stack traces
     * should be logged.
     */

    PyErr_Fetch(&ptype, &pvalue, &ptrace);

    if (ptype) {
        message = PyObject_Str(ptype);

        if (pvalue) {
            PyObject *v = NULL;
            if (PyObject_TypeCheck(pvalue, (PyTypeObject*) PyExc_BaseException)) {
                /*
                 * If Python went through PyErr_NormalizeException(...), then
                 * it's possible a PyJObject pvalue was moved to
                 * pvalue.message. PyErr_NormalizeException is called whenever
                 * there is an except: block even if the error is not actually
                 * caught.
                 */
#if PY_MAJOR_VERSION < 3
                PyObject *message = PyObject_GetAttrString(pvalue, "message");
                if (message != NULL && PyJObject_Check(message)) {
                    Py_DECREF(pvalue);
                    pvalue = message;
                }
#else
                PyObject *args = PyObject_GetAttrString(pvalue, "args");
                if (args != NULL && PyTuple_Check(args) && PyTuple_Size(args) > 0) {
                    PyObject *message = PyTuple_GetItem(args, 0);
                    Py_INCREF(message);
                    Py_DECREF(pvalue);
                    Py_DECREF(args);
                    pvalue = message;
                }
#endif
            }


            if (PyJObject_Check(pvalue)) {
                // it's a Java exception that came from process_java_exception(...)
                jstring jmessage;
                jexc = (PyJObject*) pvalue;

                jmessage = java_lang_Throwable_getLocalizedMessage(env, jexc->object);
                if ((*env)->ExceptionCheck(env)) {
                    fprintf(stderr,
                            "Error while processing a Python exception, unexpected java exception.\n");
                    PyErr_Restore(ptype, pvalue, ptrace);
                    PyErr_Print();
                    return 1;
                }
                if (jmessage != NULL) {
                    v = jstring_To_PyObject(env, jmessage);
                }
            }

            if (v == NULL) {
                // unsure of what we got, treat it as a string
                v = PyObject_Str(pvalue);
            }

            if (v != NULL && PyString_Check(v)) {
                PyObject *t;
#if PY_MAJOR_VERSION >= 3
                t = PyUnicode_FromFormat("%U: %U", message, v);
#else
                t = PyString_FromFormat("%s: %s", PyString_AsString(message),
                                        PyString_AsString(v));
#endif
                Py_DECREF(v);
                Py_DECREF(message);
                message = t;
            }
            m = PyString_AsString(message);

            // make a JepException
            jmsg = (*env)->NewStringUTF(env, (const char *) m);
            if (jexc != NULL) {
                // constructor JepException(String, Throwable)
                if (jepExcInitStrThrow == 0) {
                    jepExcInitStrThrow = (*env)->GetMethodID(env, JEP_EXC_TYPE,
                                         "<init>",
                                         "(Ljava/lang/String;Ljava/lang/Throwable;)V");
                }
                jepException = (*env)->NewObject(env, JEP_EXC_TYPE,
                                                 jepExcInitStrThrow, jmsg, jexc->object);
            } else {
                // constructor JepException(String, long)
                if (jepExcInitStrLong == 0) {
                    jepExcInitStrLong = (*env)->GetMethodID(env, JEP_EXC_TYPE,
                                                            "<init>", "(Ljava/lang/String;J)V");
                }
                jepException = (*env)->NewObject(env, JEP_EXC_TYPE,
                                                 jepExcInitStrLong, jmsg, (jlong) ptype);
            }
            (*env)->DeleteLocalRef(env, jmsg);
            if ((*env)->ExceptionCheck(env) || !jepException) {
                PyErr_Format(PyExc_RuntimeError,
                             "creating jep.JepException failed.");
                return 1;
            }

            /*
             * Attempt to get the Python traceback.
             */
            if (ptrace) {
                PyObject *modTB, *extract = NULL;
                modTB = PyImport_ImportModule("traceback");
                if (modTB == NULL) {
                    printf("Error importing python traceback module\n");
                }
                extract = PyString_FromString("extract_tb");
                if (extract == NULL) {
                    printf("Error making PyString 'extract_tb'\n");
                }
                if (modTB != NULL && extract != NULL) {
                    pystack = PyObject_CallMethodObjArgs(modTB, extract, ptrace,
                                                         NULL);
                }
                if (PyErr_Occurred()) {
                    /*
                     * Well this isn't good, we got an error while we're trying
                     * to process errors, let's just print it out.
                     */
                    PyErr_Print();
                }
                Py_XDECREF(modTB);
                Py_XDECREF(extract);
            }

            /*
             * This could go in the above if statement but I got tired of
             * incrementing so far to the right.
             */
            if (pystack != NULL) {
                Py_ssize_t stackSize, i, count;
                jsize index, javaStackLength;
                jobjectArray stackArray, javaStack, reverse;
                jclass stackTraceElemClazz;

                stackTraceElemClazz = (*env)->FindClass(env,
                                                        "java/lang/StackTraceElement");
                if (stackTraceElemInit == 0) {
                    stackTraceElemInit =
                        (*env)->GetMethodID(env, stackTraceElemClazz,
                                            "<init>",
                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
                }

                // make new StackTraceElement[len(Python traceback)]
                stackSize = PyList_Size(pystack);
                stackArray = (*env)->NewObjectArray(env, (jsize) stackSize,
                                                    stackTraceElemClazz, NULL);
                if ((*env)->ExceptionCheck(env) || !stackArray) {
                    PyErr_Format(PyExc_RuntimeError,
                                 "creating java.lang.StackTraceElement[] failed.");
                    Py_DECREF(pystack);
                    return 1;
                }

                /*
                 * Loop over each item in the Python traceback and build a Java
                 * StackTraceElement for each one.
                 */
                count = 0;
                for (i = 0; i < stackSize; i++) {
                    PyObject *stackEntry, *pyLine;
                    char *charPyFile, *charPyFunc = NULL;
                    int pyLineNum;

                    stackEntry = PyList_GetItem(pystack, i);
                    // java order is classname, method name, filename, line number
                    // python order is filename, line number, function name, line
                    charPyFile = PyString_AsString(
                                     PySequence_GetItem(stackEntry, 0));
                    pyLineNum = (int) PyInt_AsLong(
                                    PySequence_GetItem(stackEntry, 1));
                    charPyFunc = PyString_AsString(
                                     PySequence_GetItem(stackEntry, 2));
                    pyLine = PySequence_GetItem(stackEntry, 3);

                    /*
                     * If pyLine is None, this seems to imply it was an eval,
                     * making the stack element fairly useless, so we will
                     * skip it.
                     */
                    if (pyLine != Py_None) {
                        char *charPyFileNoExt, *lastDot;
                        char *charPyFileNoDir, *lastBackslash;
                        int namelen;
                        jobject element;
                        jstring pyFileNoDir, pyFileNoExt, pyFunc;

                        // remove the .py to look more like a Java StackTraceElement
                        namelen = (int) strlen(charPyFile);
                        charPyFileNoExt = malloc(sizeof(char) * (namelen + 1));
                        strcpy(charPyFileNoExt, charPyFile);
                        lastDot = strrchr(charPyFileNoExt, '.');
                        if (lastDot != NULL) {
                            *lastDot = '\0';
                        }

                        // remove the dir path to look more like a Java StackTraceElement
                        charPyFileNoDir = malloc(sizeof(char) * (namelen + 1));
                        lastBackslash = strrchr(charPyFile, FILE_SEP);
                        if (lastBackslash != NULL) {
                            strcpy(charPyFileNoDir, lastBackslash + 1);
                        } else {
                            strcpy(charPyFileNoDir, charPyFile);
                        }

                        pyFileNoDir = (*env)->NewStringUTF(env,
                                                           (const char *) charPyFileNoDir);
                        pyFileNoExt = (*env)->NewStringUTF(env,
                                                           (const char *) charPyFileNoExt);
                        pyFunc = (*env)->NewStringUTF(env,
                                                      (const char *) charPyFunc);

                        /*
                         * Make the stack trace element from Python look like a normal
                         * Java StackTraceElement.  The order may seem wrong but
                         * this makes it look best.
                         */
                        element = (*env)->NewObject(env, stackTraceElemClazz,
                                                    stackTraceElemInit, pyFileNoExt, pyFunc, pyFileNoDir,
                                                    pyLineNum);
                        if ((*env)->ExceptionCheck(env) || !element) {
                            PyErr_Format(PyExc_RuntimeError,
                                         "failed to create java.lang.StackTraceElement for python %s:%i.",
                                         charPyFile, pyLineNum);
                            free(charPyFileNoDir);
                            free(charPyFileNoExt);
                            Py_DECREF(pystack);
                            return 1;
                        }
                        (*env)->SetObjectArrayElement(env, stackArray, (jsize) i,
                                                      element);
                        count++;
                        free(charPyFileNoDir);
                        free(charPyFileNoExt);
                        (*env)->DeleteLocalRef(env, pyFileNoDir);
                        (*env)->DeleteLocalRef(env, pyFileNoExt);
                        (*env)->DeleteLocalRef(env, pyFunc);
                        (*env)->DeleteLocalRef(env, element);
                    }
                } // end of looping over Python traceback items
                Py_DECREF(pystack);

                /*
                 * Get the current Java stack trace.
                 */
                javaStack = java_lang_Throwable_getStackTrace(env, jepException);
                if ((*env)->ExceptionCheck(env) || !javaStack) {
                    PyErr_Format(PyExc_RuntimeError,
                                 "Lookup of current java stack failed.");
                    return 1;
                }
                javaStackLength = (*env)->GetArrayLength(env, javaStack);

                /*
                 * Reverse order of the Python stack and ensure no null
                 * elements so it will appear like a Java stack trace, then
                 * add the Java stack trace.
                 */
                reverse = (*env)->NewObjectArray(env, (jsize) (count + javaStackLength),
                                                 stackTraceElemClazz, NULL);
                if ((*env)->ExceptionCheck(env) || !reverse) {
                    PyErr_Format(PyExc_RuntimeError,
                                 "creating reverse java.lang.StackTraceElement[] failed.");
                    return 1;
                }

                index = 0;
                for (i = stackSize - 1; i > -1; i--) {
                    jobject element = (*env)->GetObjectArrayElement(env, stackArray, (jsize) i);
                    if (element != NULL) {
                        (*env)->SetObjectArrayElement(env, reverse, index, element);
                        (*env)->DeleteLocalRef(env, element);
                        index++;
                    }
                }
                for (i = 0; i < javaStackLength; i += 1) {
                    jobject element = (*env)->GetObjectArrayElement(env, javaStack, (jsize) i);
                    (*env)->SetObjectArrayElement(env, reverse, index, element);
                    (*env)->DeleteLocalRef(env, element);
                    index += 1;
                }
                (*env)->DeleteLocalRef(env, stackArray);
                (*env)->DeleteLocalRef(env, javaStack);


                /*
                 * Finally we have a useful-looking stack trace.
                 */
                java_lang_Throwable_setStackTrace(env, jepException, reverse);
                if ((*env)->ExceptionCheck(env)) {
                    fprintf(stderr,
                            "Error while processing a Python exception, unexpected java exception.\n");
                    PyErr_Restore(ptype, pvalue, ptrace);
                    PyErr_Print();
                    return 1;
                }
                (*env)->DeleteLocalRef(env, reverse);
            }
        }
    }

    Py_XDECREF(ptype);
    Py_XDECREF(pvalue);
    Py_XDECREF(ptrace);

    if (jepException != NULL) {
        Py_DECREF(message);
        THROW_JEP_EXC(env, jepException);
    } else if (message && PyString_Check(message)) {
        // should only get here if there was a ptype but no pvalue
        m = PyString_AsString(message);
        THROW_JEP(env, m);
        Py_DECREF(message);
    }

    return 1;
}


// convert java exception to ImportError.
// true (1) if an exception was processed.
int process_import_exception(JNIEnv *env)
{
    jstring     estr;
    jthrowable  exception    = NULL;
    PyObject   *pyException  = PyExc_ImportError;
    char       *message;
    JepThread  *jepThread;

    if (!(*env)->ExceptionCheck(env)) {
        return 0;
    }

    if ((exception = (*env)->ExceptionOccurred(env)) == NULL) {
        return 0;
    }

    jepThread = pyembed_get_jepthread();
    if (!jepThread) {
        printf("Error while processing a Java exception, "
               "invalid JepThread.\n");
        return 1;
    }

    if (jepThread->printStack) {
        (*env)->ExceptionDescribe(env);
    }

    // we're already processing this one, clear the old
    (*env)->ExceptionClear(env);

    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        return 1;
    }

    estr = jobject_tostring(env, exception);
    if ((*env)->ExceptionCheck(env) || !estr) {
        PyErr_Format(PyExc_RuntimeError, "toString() on exception failed.");
        return 1;
    }

    message = (char *) jstring2char(env, estr);
    PyErr_SetString(pyException, message);
    release_utf_char(env, estr, message);

    (*env)->DeleteLocalRef(env, exception);
    return 1;
}


/*
 * Converts a Java exception to a PyErr.  Returns true if an exception was
 * processed.  If an error was processed here, it can be caught in Python code
 * or if uncaught it will reach the method process_py_exception(...).
 */
int process_java_exception(JNIEnv *env)
{
    jthrowable exception = NULL;
    PyObject *pyExceptionType;
    PyObject *jpyExc;
    JepThread *jepThread;
    jobjectArray stack;

    if (!(*env)->ExceptionCheck(env)) {
        return 0;
    }

    if ((exception = (*env)->ExceptionOccurred(env)) == NULL) {
        return 0;
    }

    if (PyErr_Occurred()) {
        /*
         * There's a bug in Jep somewhere, hopefully this printout will help
         * diagnose it.  Then clear the error so the code can attempt to
         * continue on.
         */
        printf("WARNING: Jep internal error. Python exception detected at start of process_java_exception():\n");
        PyErr_Print();
    }

    jepThread = pyembed_get_jepthread();
    if (!jepThread) {
        printf("Error while processing a Java exception, "
               "invalid JepThread.\n");
        return 1;
    }

    if (jepThread->printStack) {
        (*env)->ExceptionDescribe(env);
    }

    // we're already processing this one, clear the old
    (*env)->ExceptionClear(env);

    /*
     * Java does not fill in a stack trace until getStackTrace() is called. If
     * it is not called now then the stack trace can be lost so even though
     * this looks like a no-op it is very important.
     */
    stack = java_lang_Throwable_getStackTrace(env, exception);
    if ((*env)->ExceptionCheck(env)) {
        PyErr_Format(PyExc_RuntimeError,
                     "wrapping java exception in pyjobject failed.");
        return 1;

    }
    (*env)->DeleteLocalRef(env, stack);
    pyExceptionType = pyerrtype_from_throwable(env, exception);

    // turn the Java exception into a PyJObject so the interpreter can handle it
    jpyExc = PyJObject_New(env, exception);
    if ((*env)->ExceptionCheck(env) || !jpyExc) {
        PyErr_Format(PyExc_RuntimeError,
                     "wrapping java exception in pyjobject failed.");
        return 1;
    }

    PyErr_SetObject(pyExceptionType, jpyExc);
    Py_DECREF(jpyExc);
    (*env)->DeleteLocalRef(env, exception);
    return 1;
}

/*
 * Matches a jthrowable to an equivalent built-in Python exception type.  This
 * is to enable more precise try: except: blocks in Python for Java
 * exceptions.  If there is not a corresponding match, the jthrowable will
 * default to being marked as a Python RuntimeError.
 */
static PyObject* pyerrtype_from_throwable(JNIEnv *env, jthrowable exception)
{

    // map ClassNotFoundException to ImportError
    if ((*env)->IsInstanceOf(env, exception, CLASSNOTFOUND_EXC_TYPE)) {
        return PyExc_ImportError;
    }

    // map IndexOutOfBoundsException exception to IndexError
    if ((*env)->IsInstanceOf(env, exception, INDEX_EXC_TYPE)) {
        return PyExc_IndexError;
    }

    // map IOException to IOError
    if ((*env)->IsInstanceOf(env, exception, IO_EXC_TYPE)) {
        return PyExc_IOError;
    }

    // map ClassCastException to TypeError
    if ((*env)->IsInstanceOf(env, exception, CLASSCAST_EXC_TYPE)) {
        return PyExc_TypeError;
    }

    // map IllegalArgumentException to ValueError
    if ((*env)->IsInstanceOf(env, exception, ILLEGALARG_EXC_TYPE)) {
        return PyExc_ValueError;
    }

    // map ArithmeticException to ArithmeticError
    if ((*env)->IsInstanceOf(env, exception, ARITHMETIC_EXC_TYPE)) {
        return PyExc_ArithmeticError;
    }

    // map OutOfMemoryError to MemoryError
    if ((*env)->IsInstanceOf(env, exception, OUTOFMEMORY_EXC_TYPE)) {
        // honestly if you hit this you're probably screwed
        return PyExc_MemoryError;
    }

    // map AssertionError to AssertionError
    if ((*env)->IsInstanceOf(env, exception, ASSERTION_EXC_TYPE)) {
        return PyExc_AssertionError;
    }

    // Reuse the python type of the exception that caused the JepException if it is available
    if ((*env)->IsInstanceOf(env, exception, JEP_EXC_TYPE)) {
        PyObject* oldType = (PyObject*) jep_JepException_getPythonType(env, exception);
        if (oldType) {
            return oldType;
        }
    }

    // default
    return PyExc_RuntimeError;
}

