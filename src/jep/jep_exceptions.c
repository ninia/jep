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
*/

#include "Jep.h"

static PyObject* pyerrtype_from_throwable(JNIEnv*, jthrowable);

// exception handling
jmethodID jepExcInitStr       = 0;
jmethodID jepExcInitStrThrow  = 0;
jmethodID stackTraceElemInit  = 0;
jmethodID setStackTrace       = 0;
jmethodID getStackTrace       = 0;
jmethodID getLocalizedMessage = 0;


/*
 * Converts a Python exception to a JepException.  Returns true if an
 * exception was processed.
 */
int process_py_exception(JNIEnv *env, int printTrace)
{
    JepThread *jepThread;
    PyObject *ptype, *pvalue, *ptrace, *pystack = NULL;
    PyObject *message = NULL;
    char *m = NULL;
    PyJObject *jexc = NULL;
    jobject jepException = NULL;
    jclass jepExcClazz;
    jstring jmsg;

    if (!PyErr_Occurred()) {
        return 0;
    }

    // let's not turn this into a Java exception if the user exited
    if (PyErr_ExceptionMatches(PyExc_SystemExit)) {
        return 0;
    }

    /*
     * If there's a trace, we'll try and attach it as the cause on the
     * JepException so the application can determine if/how stacktraces should
     * be logged.
     */

    PyErr_Fetch(&ptype, &pvalue, &ptrace);

    jepThread = pyembed_get_jepthread();
    if (!jepThread) {
        printf("Error while processing a Python exception, "
               "invalid JepThread.\n");
        if (jepThread->printStack) {
            PyErr_Print();
            if (!PyErr_Occurred()) {
                return 0;
            }
        }
    }

    if (ptype) {
        message = PyObject_Str(ptype);

        if (pvalue) {
            PyObject *v = NULL;
            if (pyjobject_check(pvalue)) {
                // it's a java exception that came from process_java_exception
                jstring jmessage;
                jexc = (PyJObject*) pvalue;

                if (getLocalizedMessage == 0) {
                    getLocalizedMessage = (*env)->GetMethodID(env, JTHROWABLE_TYPE,
                                          "getLocalizedMessage",
                                          "()Ljava/lang/String;");
                }

                jmessage = (*env)->CallObjectMethod(env, jexc->object,
                                                    getLocalizedMessage);
                if ((*env)->ExceptionCheck(env)) {
                    fprintf(stderr,
                            "Error while processing a Python exception, unexpected java exception.\n");
                    PyErr_Restore(ptype, pvalue, ptrace);
                    PyErr_Print();
                    return 1;
                }
                if (jmessage != NULL) {
                    const char* charMessage;
                    charMessage = jstring2char(env, jmessage);
                    if (charMessage != NULL) {
                        v = PyString_FromString(charMessage);
                        release_utf_char(env, jmessage, charMessage);
                    }
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
            jepExcClazz = (*env)->FindClass(env, JEPEXCEPTION);
            if (jexc != NULL) {
                // constructor JepException(String, Throwable)
                if (jepExcInitStrThrow == 0) {
                    jepExcInitStrThrow = (*env)->GetMethodID(env, jepExcClazz,
                                         "<init>",
                                         "(Ljava/lang/String;Ljava/lang/Throwable;)V");
                }
                jepException = (*env)->NewObject(env, jepExcClazz,
                                                 jepExcInitStrThrow, jmsg, jexc->object);
            } else {
                // constructor JepException(String)
                if (jepExcInitStr == 0) {
                    jepExcInitStr = (*env)->GetMethodID(env, jepExcClazz,
                                                        "<init>", "(Ljava/lang/String;)V");
                }
                jepException = (*env)->NewObject(env, jepExcClazz,
                                                 jepExcInitStr, jmsg);
            }
            (*env)->DeleteLocalRef(env, jmsg);
            if ((*env)->ExceptionCheck(env) || !jepException) {
                PyErr_Format(PyExc_RuntimeError,
                             "creating jep.JepException failed.");
                return 1;
            }

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
                     * well this isn't good, we got an error while we're trying
                     * to process errors, let's just print it out
                     */
                    PyErr_Print();
                }
                Py_XDECREF(modTB);
                Py_XDECREF(extract);
            }

            /*
             * this could go in the above if statement but I got tired of
             * incrementing so far to the right
             */
            if (pystack != NULL) {
                Py_ssize_t stackSize, i, count, index;
                jobjectArray stackArray, reverse;
                jclass stackTraceElemClazz;

                stackTraceElemClazz = (*env)->FindClass(env,
                                                        "java/lang/StackTraceElement");
                if (stackTraceElemInit == 0) {
                    stackTraceElemInit =
                        (*env)->GetMethodID(env, stackTraceElemClazz,
                                            "<init>",
                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
                }

                stackSize = PyList_Size(pystack);
                stackArray = (*env)->NewObjectArray(env, (jsize) stackSize,
                                                    stackTraceElemClazz, NULL);
                if ((*env)->ExceptionCheck(env) || !stackArray) {
                    PyErr_Format(PyExc_RuntimeError,
                                 "creating java.lang.StackTraceElement[] failed.");
                    Py_DECREF(pystack);
                    return 1;
                }

                count = 0;
                for (i = 0; i < stackSize; i++) {
                    PyObject *stackEntry, *pyLine;
                    char *charPyFile, *charPyFunc = NULL;
                    int pyLineNum;

                    stackEntry = PyList_GetItem(pystack, i);
                    // java order is classname, methodname, filename, lineNumber
                    // python order is filename, line number, function name, line
                    charPyFile = PyString_AsString(
                                     PySequence_GetItem(stackEntry, 0));
                    pyLineNum = (int) PyInt_AsLong(
                                    PySequence_GetItem(stackEntry, 1));
                    charPyFunc = PyString_AsString(
                                     PySequence_GetItem(stackEntry, 2));
                    pyLine = PySequence_GetItem(stackEntry, 3);

                    /*
                     * if pyLine is None, this seems to imply it was an eval,
                     * making the stack element fairly useless, so we will
                     * skip it
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
                         * Make the stack trace element from python look like a normal
                         * Java stack trace element.  The order may seem wrong but
                         * this makes it look best.
                         */
                        element = (*env)->NewObject(env, stackTraceElemClazz,
                                                    stackTraceElemInit, pyFileNoExt, pyFunc, pyFileNoDir,
                                                    pyLineNum);
                        if ((*env)->ExceptionCheck(env) || !element) {
                            PyErr_Format(PyExc_RuntimeError,
                                         "failed to create java.lang.StackTraceElement for python %s:%i.",
                                         charPyFile, pyLineNum);
                            release_utf_char(env, pyFileNoDir, charPyFileNoDir);
                            release_utf_char(env, pyFileNoExt, charPyFileNoExt);
                            free(charPyFileNoDir);
                            free(charPyFileNoExt);
                            release_utf_char(env, pyFunc, charPyFunc);
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
                } // end of stack for loop
                Py_DECREF(pystack);

                /*
                 * reverse order of stack and ensure no null elements so it will
                 * appear like a java stacktrace
                 */
                reverse = (*env)->NewObjectArray(env, (jsize) count,
                                                 stackTraceElemClazz, NULL);
                if ((*env)->ExceptionCheck(env) || !reverse) {
                    PyErr_Format(PyExc_RuntimeError,
                                 "creating reverse java.lang.StackTraceElement[] failed.");
                    return 1;
                }

                index = 0;
                for (i = stackSize - 1; i > -1; i--) {
                    jobject element;
                    element = (*env)->GetObjectArrayElement(env, stackArray, (jsize) i);
                    if (element != NULL) {
                        (*env)->SetObjectArrayElement(env, reverse, (jsize) index,
                                                      element);
                        index++;
                    }
                }
                (*env)->DeleteLocalRef(env, stackArray);

                if (jepException != NULL) {
                    if (setStackTrace == 0) {
                        setStackTrace = (*env)->GetMethodID(env, jepExcClazz,
                                                            "setStackTrace",
                                                            "([Ljava/lang/StackTraceElement;)V");
                    }
                    (*env)->CallObjectMethod(env, jepException, setStackTrace,
                                             reverse);
                    if ((*env)->ExceptionCheck(env)) {
                        fprintf(stderr,
                                "Error while processing a Python exception, unexpected java exception.\n");
                        PyErr_Restore(ptype, pvalue, ptrace);
                        PyErr_Print();
                        return 1;
                    }
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
    PyErr_Format(pyException, "%s", message);
    release_utf_char(env, estr, message);

    (*env)->DeleteLocalRef(env, exception);
    return 1;
}


/*
 * Converts a Java exception to a PyErr.  Returns true if an exception was
 * processed.
 */
int process_java_exception(JNIEnv *env)
{
    jthrowable exception = NULL;
    PyObject *pyExceptionType;
    PyObject *jpyExc;
    JepThread *jepThread;
    jobjectArray stack;
    PyThreadState *_save;

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

    /*
     * Java does not fill in a stack trace until getStackTrace is called, if it
     * is not called now then the stack trace can be lost so even though this
     * looks like a noop it is very important.
     */
    Py_UNBLOCK_THREADS
    if (getStackTrace == 0) {
        getStackTrace = (*env)->GetMethodID(env, JTHROWABLE_TYPE, "getStackTrace",
                                            "()[Ljava/lang/StackTraceElement;");
    }
    stack = (*env)->CallObjectMethod(env, exception, getStackTrace);
    if ((*env)->ExceptionCheck(env)) {
        PyErr_Format(PyExc_RuntimeError,
                     "wrapping java exception in pyjobject failed.");
        return 1;

    }
    (*env)->DeleteLocalRef(env, stack);
    pyExceptionType = pyerrtype_from_throwable(env, exception);
    Py_BLOCK_THREADS

    // turn the Java exception into a PyJObject so the interpreter can handle it
    jpyExc = pyjobject_new(env, exception);
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
 * is to enable more precise except/catch blocks in Python for Java exceptions.
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

    // default
    return PyExc_RuntimeError;
}

