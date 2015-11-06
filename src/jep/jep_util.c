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

#include "jep_util.h"
#include "jep_numpy.h"
#include "pyjobject.h"
#include "pyjarray.h"
#include "pyjmethod.h"
#include "pyjclass.h"
#include "pyembed.h"

static PyObject* pyerrtype_from_throwable(JNIEnv*, jthrowable);


// -------------------------------------------------- primitive class types
// these are shared for all threads, you shouldn't change them.

jclass JINT_TYPE     = NULL;
jclass JLONG_TYPE    = NULL;
jclass JOBJECT_TYPE  = NULL;
jclass JSTRING_TYPE  = NULL;
jclass JBOOLEAN_TYPE = NULL;
jclass JVOID_TYPE    = NULL;
jclass JDOUBLE_TYPE  = NULL;
jclass JSHORT_TYPE   = NULL;
jclass JFLOAT_TYPE   = NULL;
jclass JCHAR_TYPE    = NULL;
jclass JBYTE_TYPE    = NULL;
jclass JCLASS_TYPE   = NULL;

#if USE_NUMPY
// cached types for primitive arrays
jclass JBOOLEAN_ARRAY_TYPE = NULL;
jclass JBYTE_ARRAY_TYPE    = NULL;
jclass JSHORT_ARRAY_TYPE   = NULL;
jclass JINT_ARRAY_TYPE     = NULL;
jclass JLONG_ARRAY_TYPE    = NULL;
jclass JFLOAT_ARRAY_TYPE   = NULL;
jclass JDOUBLE_ARRAY_TYPE  = NULL;
#endif

// cached types for interfaces
jclass JLIST_TYPE       = NULL;
jclass JMAP_TYPE        = NULL;
jclass JITERABLE_TYPE   = NULL;
jclass JITERATOR_TYPE   = NULL;
jclass JCOLLECTION_TYPE = NULL;
jclass JCOMPARABLE_TYPE = NULL;

// cached types for Object equivalents of primitives
jclass JBOOL_OBJ_TYPE   = NULL;
jclass JINT_OBJ_TYPE    = NULL;
jclass JLONG_OBJ_TYPE   = NULL;
jclass JDOUBLE_OBJ_TYPE = NULL;

// cached types for frequently used classes
jclass JMODIFIER_TYPE    = NULL;
jclass JARRAYLIST_TYPE   = NULL;
jclass JHASHMAP_TYPE     = NULL;
jclass JCOLLECTIONS_TYPE = NULL;
#if USE_NUMPY
jclass JEP_NDARRAY_TYPE = NULL;
#endif

// exception cached types
jclass classNotFoundExc_Type;
jclass indexOutOfBoundsExc_Type;
jclass ioExc_Type;
jclass classCastExc_Type;
jclass illegalArgumentExc_Type;
jclass arithmeticExc_Type;
jclass outOfMemoryErr_Type;
jclass assertionErr_Type;

// cached methodids
jmethodID objectToString        = 0;
jmethodID objectIsArray         = 0;
jmethodID classGetComponentType = 0;

// for convert_jobject
jmethodID getBooleanValue    = 0;
jmethodID getIntValue        = 0;
jmethodID getLongValue       = 0;
jmethodID getDoubleValue     = 0;
jmethodID getFloatValue      = 0;
jmethodID getCharValue       = 0;

// exception handling
jmethodID jepExcInitStr = NULL;
jmethodID jepExcInitStrThrow = NULL;
jmethodID stackTraceElemInit = NULL;
jmethodID setStackTrace = NULL;


// call toString() on jobject, make a python string and return
// sets error conditions as needed.
// returns new reference to PyObject
PyObject* jobject_topystring(JNIEnv *env, jobject obj, jclass clazz) {
    const char *result;
    PyObject   *pyres;
    jstring     jstr;
    
    jstr = jobject_tostring(env, obj, clazz);
    // it's possible, i guess. don't throw an error....
    if(process_java_exception(env) || jstr == NULL)
        return PyString_FromString("");
    
    result = (*env)->GetStringUTFChars(env, jstr, 0);
    pyres  = PyString_FromString(result);
    (*env)->ReleaseStringUTFChars(env, jstr, result);
    (*env)->DeleteLocalRef(env, jstr);
    
    // method returns new reference.
    return pyres;
}


PyObject* pystring_split_item(PyObject *str, char *split, int pos) {
    PyObject  *splitList, *ret;
    Py_ssize_t len;

    if(pos < 0) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Invalid position to return.");
        return NULL;
    }

    splitList = PyObject_CallMethod(str, "split", "s", split);
    if(PyErr_Occurred() || !splitList)
        return NULL;
    
    if(!PyList_Check(splitList)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Oops, split string return is not a list.");
        return NULL;
    }
    
    len = PyList_Size(splitList);
    if(pos > len - 1) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Not enough items to return split position.");
        return NULL;
    }
    
    // get requested item
    ret = PyList_GetItem(splitList, pos);
    if(PyErr_Occurred())
        return NULL;
    if(!PyString_Check(ret)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Oops, item is not a string.");
        return NULL;
    }
    
    Py_INCREF(ret);
    Py_DECREF(splitList);
    return ret;
}


PyObject* pystring_split_last(PyObject *str, char *split) {
    PyObject   *splitList, *ret;
    Py_ssize_t  len;

    splitList = PyObject_CallMethod(str, "split", "s", split);
    if(PyErr_Occurred() || !splitList)
        return NULL;
    
    if(!PyList_Check(splitList)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Oops, split string return is not a list.");
        return NULL;
    }
    
    len = PyList_Size(splitList);
    
    // get the last one
    ret = PyList_GetItem(splitList, len - 1);
    if(PyErr_Occurred())
        return NULL;
    if(!PyString_Check(ret)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Oops, item is not a string.");
        return NULL;
    }
    
    Py_INCREF(ret);
    Py_DECREF(splitList);
    return ret;
}

// support for python 3.2
// unnecessary for python 2.6, 2.7, and 3.3+
char* pyunicode_to_utf8(PyObject *unicode) {
    PyObject *bytesObj;
    char     *c;
    bytesObj = PyUnicode_AsUTF8String(unicode);
    if(bytesObj == NULL) {
         if(PyErr_Occurred()) {
            printf("Error converting PyUnicode to PyBytes\n");
            PyErr_Print();
         }
       return NULL;
    }
    
    c = PyBytes_AsString(bytesObj);
    Py_DECREF(bytesObj);
    if(PyErr_Occurred()) {
       PyErr_Print();
       return NULL;
    }
    return c;
}



// convert python exception to java.
int process_py_exception(JNIEnv *env, int printTrace) {
    JepThread *jepThread;
    PyObject *ptype, *pvalue, *ptrace, *pystack = NULL;
    PyObject *message = NULL;
    char *m = NULL;
    PyJObject *jexc = NULL;
    jobject jepException = NULL;
    jclass jepExcClazz;
    jstring jmsg;

    if(!PyErr_Occurred())
        return 0;

    // let's not turn this into a Java exception if the user exited
    if(PyErr_ExceptionMatches(PyExc_SystemExit))
         return 0;

    /*
     * We only care about ptype and pvalue for the message.
     * Many people consider it a security vulnerability
     * to have the source code printed to the user's
     * screen.  We'll try and attach the trace as the
     * cause on the JepException so the application
     * can determine if/how traces should be logged or
     * shown.
     */

    PyErr_Fetch(&ptype, &pvalue, &ptrace);

    jepThread = pyembed_get_jepthread();
    if(!jepThread) {
        printf("Error while processing a Python exception, "
                "invalid JepThread.\n");
        if(jepThread->printStack) {
            PyErr_Print();
            if(!PyErr_Occurred())
                return 0;
        }
    }

    if(ptype) {
        message = PyObject_Str(ptype);

        if(pvalue) {
            PyObject *v = NULL;
            if(pyjobject_check(pvalue)) {
                // it's a java exception that came from process_java_exception
                jmethodID getMessage;
                jexc = (PyJObject*) pvalue;
                getMessage = (*env)->GetMethodID(env, jexc->clazz,
                        "getLocalizedMessage", "()Ljava/lang/String;");
                if(getMessage != NULL) {
                    jstring jmessage;
                    jmessage = (*env)->CallObjectMethod(env, jexc->object,
                            getMessage);
                    if(jmessage != NULL) {
                        const char* charMessage;
                        charMessage = jstring2char(env, jmessage);
                        if(charMessage != NULL) {
                            v = PyString_FromString(charMessage);
                            release_utf_char(env, jmessage, charMessage);
                        }
                    }
                } else {
                    printf(
                            "Error getting method getLocalizedMessage() on java exception\n");
                }
            }

            if(v == NULL) {
                // unsure of what we got, treat it as a string
                v = PyObject_Str(pvalue);
            }

            if(v != NULL && PyString_Check(v)) {
                PyObject *t;
#if PY_MAJOR_VERSION >= 3
                t = PyUnicode_FromFormat("%U: %U", message, v);
#else
                t = PyString_FromFormat("%s: %s", PyString_AsString(message), PyString_AsString(v));
#endif
                Py_DECREF(v);
                Py_DECREF(message);
                message = t;
            }
            m = PyString_AsString(message);

            // make a JepException
            jmsg = (*env)->NewStringUTF(env, (const char *) m);
            jepExcClazz = (*env)->FindClass(env, JEPEXCEPTION);
            if(jexc != NULL) {
                // constructor JepException(String, Throwable)
                if(jepExcInitStrThrow == NULL) {
                    jepExcInitStrThrow = (*env)->GetMethodID(env, jepExcClazz,
                            "<init>",
                            "(Ljava/lang/String;Ljava/lang/Throwable;)V");
                }
                jepException = (*env)->NewObject(env, jepExcClazz,
                        jepExcInitStrThrow, jmsg, jexc->object);
            } else {
                // constructor JepException(String)
                if(jepExcInitStr == NULL) {
                    jepExcInitStr = (*env)->GetMethodID(env, jepExcClazz,
                            "<init>", "(Ljava/lang/String;)V");
                }
                jepException = (*env)->NewObject(env, jepExcClazz,
                        jepExcInitStr, jmsg);
            }
            (*env)->DeleteLocalRef(env, jmsg);
            if((*env)->ExceptionCheck(env) || !jepException) {
                PyErr_Format(PyExc_RuntimeError,
                        "creating jep.JepException failed.");
                return 1;
            }

            if(ptrace) {
                PyObject *modTB, *extract = NULL;
                modTB = PyImport_ImportModule("traceback");
                if(modTB == NULL) {
                    printf("Error importing python traceback module\n");
                }
                extract = PyString_FromString("extract_tb");
                if(extract == NULL) {
                    printf("Error making PyString 'extract_tb'\n");
                }
                if(modTB != NULL && extract != NULL) {
                    pystack = PyObject_CallMethodObjArgs(modTB, extract, ptrace,
                            NULL);
                }
                if(PyErr_Occurred()) {
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
            if(pystack != NULL) {
                Py_ssize_t stackSize, i, count, index;
                jobjectArray stackArray, reverse;
                jclass stackTraceElemClazz;

                stackTraceElemClazz = (*env)->FindClass(env,
                        "Ljava/lang/StackTraceElement;");
                if(stackTraceElemInit == NULL) {
                    stackTraceElemInit =
                            (*env)->GetMethodID(env, stackTraceElemClazz,
                                    "<init>",
                                    "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
                }

                stackSize = PyList_Size(pystack);
                stackArray = (*env)->NewObjectArray(env, (jsize) stackSize,
                        stackTraceElemClazz, NULL);
                if((*env)->ExceptionCheck(env) || !stackArray) {
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
                            PyTuple_GetItem(stackEntry, 0));
                    pyLineNum = (int) PyInt_AsLong(
                            PyTuple_GetItem(stackEntry, 1));
                    charPyFunc = PyString_AsString(
                            PyTuple_GetItem(stackEntry, 2));
                    pyLine = PyTuple_GetItem(stackEntry, 3);

                    /*
                     * if pyLine is None, this seems to imply it was an eval,
                     * making the stack element fairly useless, so we will
                     * skip it
                     */
                    if(pyLine != Py_None) {
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
                        if(lastDot != NULL) {
                            *lastDot = '\0';
                        }

                        // remove the dir path to look more like a Java StackTraceElement
                        charPyFileNoDir = malloc(sizeof(char) * (namelen + 1));
                        lastBackslash = strrchr(charPyFile, FILE_SEP);
                        if(lastBackslash != NULL) {
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
                         * java stack trace element.  The order may seem wrong but
                         * this makes it look best.
                         */
                        element = (*env)->NewObject(env, stackTraceElemClazz,
                                stackTraceElemInit, pyFileNoExt, pyFunc, pyFileNoDir,
                                pyLineNum);
                        if((*env)->ExceptionCheck(env) || !element) {
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
                if((*env)->ExceptionCheck(env) || !reverse) {
                    PyErr_Format(PyExc_RuntimeError,
                            "creating reverse java.lang.StackTraceElement[] failed.");
                    return 1;
                }

                index = 0;
                for (i = stackSize - 1; i > -1; i--) {
                    jobject element;
                    element = (*env)->GetObjectArrayElement(env, stackArray, (jsize) i);
                    if(element != NULL) {
                        (*env)->SetObjectArrayElement(env, reverse, (jsize) index,
                                element);
                        index++;
                    }
                }
                (*env)->DeleteLocalRef(env, stackArray);

                if(jepException != NULL) {
                    if(setStackTrace == NULL) {
                        setStackTrace = (*env)->GetMethodID(env, jepExcClazz,
                                "setStackTrace",
                                "([Ljava/lang/StackTraceElement;)V");
                    }
                    (*env)->CallObjectMethod(env, jepException, setStackTrace,
                            reverse);
                }
                (*env)->DeleteLocalRef(env, reverse);
            }
        }
    }

    Py_XDECREF(ptype);
    Py_XDECREF(pvalue);
    Py_XDECREF(ptrace);

    if(jepException != NULL) {
        Py_DECREF(message);
        THROW_JEP_EXC(env, jepException);
    } else if(message && PyString_Check(message)) {
        // should only get here if there was a ptype but no pvalue
        m = PyString_AsString(message);
        THROW_JEP(env, m);
        Py_DECREF(message);
    }

    return 1;
}


// convert java exception to ImportError.
// true (1) if an exception was processed.
int process_import_exception(JNIEnv *env) {
    jstring     estr;
    jthrowable  exception    = NULL;
    jclass      clazz;
    PyObject   *pyException  = PyExc_ImportError;
    char       *message;
    JepThread  *jepThread;

    if(!(*env)->ExceptionCheck(env))
        return 0;

    if((exception = (*env)->ExceptionOccurred(env)) == NULL)
        return 0;

    jepThread = pyembed_get_jepthread();
    if(!jepThread) {
        printf("Error while processing a Java exception, "
               "invalid JepThread.\n");
        return 1;
    }

    if(jepThread->printStack)    
        (*env)->ExceptionDescribe(env);

    // we're already processing this one, clear the old
    (*env)->ExceptionClear(env);

    clazz = (*env)->GetObjectClass(env, exception);
    if((*env)->ExceptionCheck(env) || !clazz) {
        (*env)->ExceptionDescribe(env);
        return 1;
    }
    
    estr = jobject_tostring(env, exception, clazz);
    if((*env)->ExceptionCheck(env) || !estr) {
        PyErr_Format(PyExc_RuntimeError, "toString() on exception failed.");
        return 1;
    }

    message = (char *) jstring2char(env, estr);
    PyErr_Format(pyException, "%s", message);
    release_utf_char(env, estr, message);
    
    (*env)->DeleteLocalRef(env, clazz);
    (*env)->DeleteLocalRef(env, exception);
    return 1;
}


// convert java exception to pyerr.
// true (1) if an exception was processed.
int process_java_exception(JNIEnv *env) {
    jthrowable exception = NULL;
    jclass clazz;
    PyObject *pyException;
    PyObject *jpyExc;
    JepThread *jepThread;
    jmethodID fillInStacktrace;
    jobjectArray stack;

    if(!(*env)->ExceptionCheck(env))
        return 0;

    if((exception = (*env)->ExceptionOccurred(env)) == NULL)
        return 0;

    jepThread = pyembed_get_jepthread();
    if(!jepThread) {
        printf("Error while processing a Java exception, "
                "invalid JepThread.\n");
        return 1;
    }

    if(jepThread->printStack)
        (*env)->ExceptionDescribe(env);

    // we're already processing this one, clear the old
    (*env)->ExceptionClear(env);

    clazz = (*env)->GetObjectClass(env, exception);
    if((*env)->ExceptionCheck(env) || !clazz) {
        (*env)->ExceptionDescribe(env);
        return 1;
    }

    // fill in the stack trace here to make sure we don't lose it
    fillInStacktrace = (*env)->GetMethodID(env, clazz, "getStackTrace",
            "()[Ljava/lang/StackTraceElement;");
    stack = (*env)->CallObjectMethod(env, exception, fillInStacktrace);
    (*env)->DeleteLocalRef(env, stack);

    // turn the java exception into a pyjobject so the interpreter can handle it
    jpyExc = pyjobject_new(env, exception);
    if((*env)->ExceptionCheck(env) || !jpyExc) {
        PyErr_Format(PyExc_RuntimeError,
                "wrapping java exception in pyjobject failed.");
        return 1;
    }

    pyException = pyerrtype_from_throwable(env, exception);
    PyErr_SetObject(pyException, jpyExc);
    Py_DECREF(jpyExc);
    (*env)->DeleteLocalRef(env, clazz);
    (*env)->DeleteLocalRef(env, exception);
    return 1;
}

/*
 * Matches a jthrowable to an equivalent built-in python exception type.  This
 * is to enable more precise except/catch blocks in python for Java exceptions.
 */
static PyObject* pyerrtype_from_throwable(JNIEnv *env, jthrowable exception) {

    // map ClassNotFoundException to ImportError
    if((*env)->IsInstanceOf(env, exception, classNotFoundExc_Type)) {
        return PyExc_ImportError;
    }

    // map IndexOutOfBoundsException exception to IndexError
    if((*env)->IsInstanceOf(env, exception, indexOutOfBoundsExc_Type)) {
        return PyExc_IndexError;
    }

    // map IOException to IOError
    if((*env)->IsInstanceOf(env, exception, ioExc_Type)) {
        return PyExc_IOError;
    }

    // map ClassCastException to TypeError
    if((*env)->IsInstanceOf(env, exception, classCastExc_Type)) {
        return PyExc_TypeError;
    }

    // map IllegalArgumentException to ValueError
    if((*env)->IsInstanceOf(env, exception, illegalArgumentExc_Type)) {
        return PyExc_ValueError;
    }

    // map ArithmeticException to ArithmeticError
    if((*env)->IsInstanceOf(env, exception, arithmeticExc_Type)) {
        return PyExc_ArithmeticError;
    }

    // map OutOfMemoryError to MemoryError
    if((*env)->IsInstanceOf(env, exception, outOfMemoryErr_Type)) {
        // honestly if you hit this you're probably screwed
        return PyExc_MemoryError;
    }

    // map AssertionError to AssertionError
    if((*env)->IsInstanceOf(env, exception, assertionErr_Type)) {
        return PyExc_AssertionError;
    }

    // default
    return PyExc_RuntimeError;
}


// call toString() on jobject and return result.
// NULL on error
jstring jobject_tostring(JNIEnv *env, jobject obj, jclass clazz) {
    jstring     jstr;

    if(!env || !obj || !clazz)
        return NULL;

    if(objectToString == 0) {
        objectToString = (*env)->GetMethodID(env,
                                             clazz,
                                             "toString",
                                             "()Ljava/lang/String;");
        if(process_java_exception(env))
            return NULL;
    }

    if(!objectToString) {
        PyErr_Format(PyExc_RuntimeError, "%s", "Couldn't get methodId.");
        return NULL;
    }

    jstr = (jstring) (*env)->CallObjectMethod(env, obj, objectToString);
    if(process_java_exception(env))
        return NULL;

    return jstr;
}


// get a const char* string from java string.
// you *must* call release when you're finished with it.
// returns local reference.
const char* jstring2char(JNIEnv *env, jstring str) {
    if(str == NULL)
        return NULL;
    return (*env)->GetStringUTFChars(env, str, 0);
}


// release memory allocated by jstring2char
void release_utf_char(JNIEnv *env, jstring str, const char *v) {
    if(v != NULL && str != NULL) {
        (*env)->ReleaseStringUTFChars(env, str, v);
        (*env)->DeleteLocalRef(env, str);
    }
}


/* These macros are Only intended for use within the caching methods below. */
#define CACHE_CLASS(var, name)\
    if(var == NULL) {\
        clazz = (*env)->FindClass(env, name);\
        if((*env)->ExceptionOccurred(env))\
            return 0;\
        var = (*env)->NewGlobalRef(env, clazz);\
        (*env)->DeleteLocalRef(env, clazz);\
    }\

#define UNCACHE_CLASS(var)\
    if(var != NULL) {\
        (*env)->DeleteGlobalRef(env, var);\
        var = NULL;\
    }\

#define CACHE_PRIMITIVE_ARRAY(primitive, array, name)\
    if(primitive == NULL) {\
        if(array == NULL) {\
            clazz = (*env)->FindClass(env, name);\
            if((*env)->ExceptionOccurred(env))\
                return 0;\
            array = (*env)->NewGlobalRef(env, clazz);\
            (*env)->DeleteLocalRef(env, clazz);\
        }\
        clazz = (*env)->CallObjectMethod(env, array, classGetComponentType);\
        primitive = (*env)->NewGlobalRef(env, clazz);\
        (*env)->DeleteLocalRef(env, clazz);\
    }\


/*
 * In order to call methods that return primitives, we have to know their
 * return type.  That's easy, we're using the Reflection API to call
 * java.lang.reflect.Method.getReturnType().
 *
 * However, JNI requires us to match the return type to the corresponding
 * (*env)->Call<type>Method(args...) where <type> is the return type.
 * Therefore to quickly compare the return type we need the underlying
 * primitive types and not their Object types.  For example we need int,
 * not java.lang.Integer.
 *
 * We use two techniques to get at the primitive types:
 *
 *   1. Get the java.lang.Class for a primitive array, and then call
 *       Class.getComponentType().
 *
 *   2. Get the java.lang.Class for the Object and then access the static
 *       field Class.TYPE, e.g. java.lang.Integer.TYPE.
 *
 * Returns 1 if successful, 0 if failed.  Does not process Java exceptions.
 */
int cache_primitive_classes(JNIEnv *env) {
    jclass   clazz, tmpclazz = NULL;
    jfieldID fieldId;
    jobject  tmpobj          = NULL;

    if(classGetComponentType == 0) {
        classGetComponentType = (*env)->GetMethodID(env, JCLASS_TYPE, "getComponentType", "()Ljava/lang/Class;");
        if((*env)->ExceptionOccurred(env))
            return 0;
    }

    CACHE_PRIMITIVE_ARRAY(JBOOLEAN_TYPE, JBOOLEAN_ARRAY_TYPE, "[Z");
    CACHE_PRIMITIVE_ARRAY(JBYTE_TYPE, JBYTE_ARRAY_TYPE, "[B");
    CACHE_PRIMITIVE_ARRAY(JSHORT_TYPE, JSHORT_ARRAY_TYPE, "[S");
    CACHE_PRIMITIVE_ARRAY(JINT_TYPE, JINT_ARRAY_TYPE, "[I");
    CACHE_PRIMITIVE_ARRAY(JLONG_TYPE, JLONG_ARRAY_TYPE, "[J");
    CACHE_PRIMITIVE_ARRAY(JFLOAT_TYPE, JFLOAT_ARRAY_TYPE, "[F");
    CACHE_PRIMITIVE_ARRAY(JDOUBLE_TYPE, JDOUBLE_ARRAY_TYPE, "[D");


    if(JVOID_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Void");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                                                         clazz,
                                                         fieldId);
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        JVOID_TYPE = (*env)->NewGlobalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpobj);
        (*env)->DeleteLocalRef(env, clazz);
    }

    if(JCHAR_TYPE == NULL) {
        clazz = (*env)->FindClass(env, "java/lang/Character");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        fieldId = (*env)->GetStaticFieldID(env,
                                           clazz,
                                           "TYPE",
                                           "Ljava/lang/Class;");
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        tmpclazz = (jclass) (*env)->GetStaticObjectField(env,
                                                         clazz,
                                                         fieldId);
        if((*env)->ExceptionOccurred(env))
            return 0;
        
        JCHAR_TYPE = (*env)->NewGlobalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpclazz);
        (*env)->DeleteLocalRef(env, tmpobj);
        (*env)->DeleteLocalRef(env, clazz);
    }

    return 1;
}

// remove global references setup in above function.
// TODO is this method used?  Should it be called from pyembed_shutdown()?
void unref_cache_primitive_classes(JNIEnv *env) {
    UNCACHE_CLASS(JBOOLEAN_TYPE);
    UNCACHE_CLASS(JBYTE_TYPE);
    UNCACHE_CLASS(JSHORT_TYPE);
    UNCACHE_CLASS(JINT_TYPE);
    UNCACHE_CLASS(JLONG_TYPE);
    UNCACHE_CLASS(JFLOAT_TYPE);
    UNCACHE_CLASS(JDOUBLE_TYPE);

    // release the primitive array types
    UNCACHE_CLASS(JBOOLEAN_ARRAY_TYPE);
    UNCACHE_CLASS(JBYTE_ARRAY_TYPE);
    UNCACHE_CLASS(JSHORT_ARRAY_TYPE);
    UNCACHE_CLASS(JINT_ARRAY_TYPE);
    UNCACHE_CLASS(JLONG_ARRAY_TYPE);
    UNCACHE_CLASS(JFLOAT_ARRAY_TYPE);
    UNCACHE_CLASS(JDOUBLE_ARRAY_TYPE);
}

int cache_frequent_classes(JNIEnv *env) {
    jclass clazz;

    CACHE_CLASS(JOBJECT_TYPE, "java/lang/Object");
    CACHE_CLASS(JSTRING_TYPE, "java/lang/String");
    CACHE_CLASS(JCLASS_TYPE, "java/lang/Class");

    CACHE_CLASS(JLIST_TYPE, "java/util/List");
    CACHE_CLASS(JMAP_TYPE, "java/util/Map");
    CACHE_CLASS(JITERABLE_TYPE, "java/lang/Iterable");
    CACHE_CLASS(JITERATOR_TYPE, "java/util/Iterator");
    CACHE_CLASS(JCOLLECTION_TYPE, "java/util/Collection");
    CACHE_CLASS(JCOMPARABLE_TYPE, "java/lang/Comparable");
    CACHE_CLASS(JBOOL_OBJ_TYPE, "java/lang/Boolean");
    CACHE_CLASS(JINT_OBJ_TYPE, "java/lang/Integer");
    CACHE_CLASS(JLONG_OBJ_TYPE, "java/lang/Long");
    CACHE_CLASS(JDOUBLE_OBJ_TYPE, "java/lang/Double");
    CACHE_CLASS(JMODIFIER_TYPE, "java/lang/reflect/Modifier");
    CACHE_CLASS(JARRAYLIST_TYPE, "java/util/ArrayList");
    CACHE_CLASS(JHASHMAP_TYPE, "java/util/HashMap");
    CACHE_CLASS(JCOLLECTIONS_TYPE, "java/util/Collections");

#if USE_NUMPY
    CACHE_CLASS(JEP_NDARRAY_TYPE, "jep/NDArray");
#endif

    // find and cache exception types we check for
    CACHE_CLASS(classNotFoundExc_Type, "java/lang/ClassNotFoundException");
    CACHE_CLASS(indexOutOfBoundsExc_Type, "java/lang/IndexOutOfBoundsException");
    CACHE_CLASS(ioExc_Type, "java/io/IOException");
    CACHE_CLASS(classCastExc_Type, "java/lang/ClassCastException");
    CACHE_CLASS(illegalArgumentExc_Type, "java/lang/IllegalArgumentException");
    CACHE_CLASS(arithmeticExc_Type, "java/lang/ArithmeticException");
    CACHE_CLASS(outOfMemoryErr_Type, "java/lang/OutOfMemoryError");
    CACHE_CLASS(assertionErr_Type, "java/lang/AssertionError");

    return 1;
}

// TODO is this method used?  Should it be called from pyembed_shutdown()?
void unref_cache_frequent_classes(JNIEnv *env) {
    UNCACHE_CLASS(JOBJECT_TYPE);
    UNCACHE_CLASS(JSTRING_TYPE);
    UNCACHE_CLASS(JCLASS_TYPE);

    UNCACHE_CLASS(JLIST_TYPE);
    UNCACHE_CLASS(JMAP_TYPE);
    UNCACHE_CLASS(JITERABLE_TYPE);
    UNCACHE_CLASS(JITERATOR_TYPE);
    UNCACHE_CLASS(JCOLLECTION_TYPE);
    UNCACHE_CLASS(JCOMPARABLE_TYPE);
    UNCACHE_CLASS(JBOOL_OBJ_TYPE);
    UNCACHE_CLASS(JINT_OBJ_TYPE);
    UNCACHE_CLASS(JLONG_OBJ_TYPE);
    UNCACHE_CLASS(JDOUBLE_OBJ_TYPE);
    UNCACHE_CLASS(JMODIFIER_TYPE);
    UNCACHE_CLASS(JARRAYLIST_TYPE);
    UNCACHE_CLASS(JHASHMAP_TYPE);
    UNCACHE_CLASS(JCOLLECTIONS_TYPE);

#if USE_NUMPY
    UNCACHE_CLASS(JEP_NDARRAY_TYPE);
#endif

    // release exception types we check for
    UNCACHE_CLASS(classNotFoundExc_Type);
    UNCACHE_CLASS(indexOutOfBoundsExc_Type);
    UNCACHE_CLASS(ioExc_Type);
    UNCACHE_CLASS(classCastExc_Type);
    UNCACHE_CLASS(illegalArgumentExc_Type);
    UNCACHE_CLASS(arithmeticExc_Type);
    UNCACHE_CLASS(outOfMemoryErr_Type);
    UNCACHE_CLASS(assertionErr_Type);
}


// given the Class object, return the const ID.
// -1 on error or NULL.
// doesn't process errors!
int get_jtype(JNIEnv *env, jclass clazz) {
    jboolean equals = JNI_FALSE;
    jboolean array  = JNI_FALSE;

    // have to find Class.isArray() method
    if(objectIsArray == 0) {
        objectIsArray = (*env)->GetMethodID(env,
                                            JCLASS_TYPE,
                                            "isArray",
                                            "()Z");
        if((*env)->ExceptionCheck(env) || !objectIsArray)
            return -1;
    }

    // object checks
    if((*env)->IsAssignableFrom(env, clazz, JOBJECT_TYPE)) {
        // check for string
        equals = (*env)->IsSameObject(env, clazz, JSTRING_TYPE);
        if((*env)->ExceptionCheck(env))
            return -1;
        if(equals)
            return JSTRING_ID;


        // check if it's an array first
        array = (*env)->CallBooleanMethod(env, clazz, objectIsArray);
        if((*env)->ExceptionCheck(env))
            return -1;

        if(array)
            return JARRAY_ID;

        // check for class
        if((*env)->IsAssignableFrom(env, clazz, JCLASS_TYPE))
            return JCLASS_ID;

        /*
         * TODO: contemplate adding List and jep.NDArray check in here
         */

        // ok it's not a string, array, or class, so let's call it object
        return JOBJECT_ID;
    }

    /*
     * check primitive types
     */

    // int
    equals = (*env)->IsSameObject(env, clazz, JINT_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JINT_ID;

    // double
    equals = (*env)->IsSameObject(env, clazz, JDOUBLE_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JDOUBLE_ID;

    // float
    equals = (*env)->IsSameObject(env, clazz, JFLOAT_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JFLOAT_ID;

    // long
    equals = (*env)->IsSameObject(env, clazz, JLONG_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JLONG_ID;

    // boolean
    equals = (*env)->IsSameObject(env, clazz, JBOOLEAN_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JBOOLEAN_ID;

    // void
    equals = (*env)->IsSameObject(env, clazz, JVOID_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JVOID_ID;
    
    // char
    equals = (*env)->IsSameObject(env, clazz, JCHAR_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JCHAR_ID;

    // byte
    equals = (*env)->IsSameObject(env, clazz, JBYTE_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JBYTE_ID;
    
    // short
    equals = (*env)->IsSameObject(env, clazz, JSHORT_TYPE);
    if((*env)->ExceptionCheck(env))
        return -1;
    if(equals)
        return JSHORT_ID;

    return -1;
}


// returns if the type of python object matches jclass
int pyarg_matches_jtype(JNIEnv *env,
                        PyObject *param,
                        jclass paramType,
                        int paramTypeId) {
    
    switch(paramTypeId) {
        
    case JCHAR_ID:
        // must not be null...
        if(PyString_Check(param) && PyString_GET_SIZE(param) == 1)
            return 1;
        return 0;
        
    case JSTRING_ID:

        if(param == Py_None)
            return 1;

        if(PyString_Check(param))
            return 1;
        
        if(pyjobject_check(param)) {
            // check if the object itself can cast to parameter type.
            if((*env)->IsAssignableFrom(env,
                                        ((PyJObject *) param)->clazz,
                                        paramType))
                return 1;
        }
        
        break;

    case JARRAY_ID:
        if(param == Py_None)
            return 1;
        
        if(pyjarray_check(param)) {
            // check if the object itself can cast to parameter type.
            if((*env)->IsAssignableFrom(env,
                                        ((PyJArrayObject *) param)->clazz,
                                        paramType))
                return 1;
        }

        break;
        
    case JCLASS_ID:
        if(param == Py_None)
            return 1;
        
        if(pyjclass_check(param))
            return 1;

        break;

    case JOBJECT_ID:
        if(param == Py_None)
            return 1;
        
        if(pyjobject_check(param)) {
            // check if the object itself can cast to parameter type.
            if((*env)->IsAssignableFrom(env,
                                        ((PyJObject *) param)->clazz,
                                        paramType))
                return 1;
        }

        if(PyString_Check(param)) {
            if((*env)->IsAssignableFrom(env,
                                        JSTRING_TYPE,
                                        paramType))
                return 1;
        }
        
        break;

    case JBYTE_ID:
    case JSHORT_ID:
    case JINT_ID:
        if(PyInt_Check(param))
            return 1;
        break;

    case JFLOAT_ID:
    case JDOUBLE_ID:
        if(PyFloat_Check(param))
            return 1;
        break;

    case JLONG_ID:
        if(PyLong_Check(param))
            return 1;
        if(PyInt_Check(param))
            return 1;
        break;
        
    case JBOOLEAN_ID:
        if(PyInt_Check(param))
            return 1;
        break;
    }

    // no match
    return 0;
}


// convert java object to python. use this to unbox jobject
// throws java exception on error
PyObject* convert_jobject(JNIEnv *env, jobject val, int typeid) {
    PyThreadState *_save;

    if(getIntValue == 0) {
        jclass clazz;

        // get all the methodIDs here. Faster this way for Number
        // subclasses, then we'll just call the right methods below
        Py_UNBLOCK_THREADS;
        clazz = (*env)->FindClass(env, "java/lang/Number");

        getIntValue = (*env)->GetMethodID(env,
                                          clazz,
                                          "intValue",
                                          "()I");
        getLongValue = (*env)->GetMethodID(env,
                                           clazz,
                                           "longValue",
                                           "()J");
        getDoubleValue = (*env)->GetMethodID(env,
                                             clazz,
                                             "doubleValue",
                                             "()D");
        getFloatValue = (*env)->GetMethodID(env,
                                            clazz,
                                            "floatValue",
                                            "()F");

        (*env)->DeleteLocalRef(env, clazz);
        Py_BLOCK_THREADS;

        if((*env)->ExceptionOccurred(env))
            return NULL;
    }

    switch(typeid) {
    case -1:
        // null
        Py_RETURN_NONE;

    case JARRAY_ID:
        return (PyObject *) pyjarray_new(env, val);

    case JSTRING_ID: {
        const char *str;
        PyObject *ret;

        str = jstring2char(env, val);
        ret = PyString_FromString(str);
        release_utf_char(env, val, str);

        return ret;
    }

    case JCLASS_ID:
        return (PyObject *) pyjobject_new_class(env, val);

    case JVOID_ID:
        // pass through
        // wrap as a object... try to be diligent.

    case JOBJECT_ID:
        return (PyObject *) pyjobject_new(env, val);

    case JBOOLEAN_ID: {
        jboolean b;

        if(getBooleanValue == 0) {
            getBooleanValue = (*env)->GetMethodID(env,
                                                  JBOOL_OBJ_TYPE,
                                                  "booleanValue",
                                                  "()Z");
            if((*env)->ExceptionOccurred(env))
                return NULL;
        }

        b = (*env)->CallBooleanMethod(env, val, getBooleanValue);
        if((*env)->ExceptionOccurred(env))
            return NULL;

        if(b)
            return Py_BuildValue("i", 1);
        return Py_BuildValue("i", 0);
    }

    case JBYTE_ID:              /* pass through */
    case JSHORT_ID:             /* pass through */
    case JINT_ID: {
        jint b = (*env)->CallIntMethod(env, val, getIntValue);
        if((*env)->ExceptionOccurred(env))
            return NULL;

        return Py_BuildValue("i", b);
    }

    case JLONG_ID: {
        jlong b = (*env)->CallLongMethod(env, val, getLongValue);
        if((*env)->ExceptionOccurred(env))
            return NULL;

        return Py_BuildValue("i", b);
    }

    case JDOUBLE_ID: {
        jdouble b = (*env)->CallDoubleMethod(env, val, getDoubleValue);
        if((*env)->ExceptionOccurred(env))
            return NULL;

        return PyFloat_FromDouble(b);
    }

    case JFLOAT_ID: {
        jfloat b = (*env)->CallFloatMethod(env, val, getFloatValue);
        if((*env)->ExceptionOccurred(env))
            return NULL;

        return PyFloat_FromDouble(b);
    }


    case JCHAR_ID: {
        jchar c;

        if(getCharValue == 0) {
            jclass clazz;

            Py_UNBLOCK_THREADS;
            clazz = (*env)->FindClass(env, "java/lang/Character");

            getCharValue = (*env)->GetMethodID(env,
                                               clazz,
                                               "charValue",
                                               "()C");
            (*env)->DeleteLocalRef(env, clazz);
            Py_BLOCK_THREADS;

            if((*env)->ExceptionOccurred(env))
                return NULL;
        }

        c = (*env)->CallCharMethod(env, val, getCharValue);
        if((*env)->ExceptionOccurred(env))
            return NULL;

        return PyString_FromFormat("%c", (char) c);
    }

    default:
        break;
    }

    THROW_JEP(env, "util.c:convert_jobject invalid typeid.");
    return NULL;
}

/*
 * Converts java object to python.  Use this to unbox jobject.
 * Wraps convert_jobject() method above to provide for situations
 * where you don't know the jobject's type beforehand.
 */
PyObject* convert_jobject_pyobject(JNIEnv *env, jobject val) {
    int typeId         = -1;

    if(val != NULL) {
        jclass    retClass = NULL;
        jmethodID getClass = NULL;

        getClass = (*env)->GetMethodID(env, JOBJECT_TYPE, "getClass", "()Ljava/lang/Class;");
        if(process_java_exception(env) || !getClass) {
            return NULL;
        }

        retClass = (*env)->CallObjectMethod(env, val, getClass);
        if(process_java_exception(env) || !retClass) {
            return NULL;
        }
        typeId = get_jtype(env, retClass);
    }

    return convert_jobject(env, val, typeId);
}


// for parsing args.
// takes a python object and sets the right jvalue member for the given java type.
// returns uninitialized on error and raises a python exception.
jvalue convert_pyarg_jvalue(JNIEnv *env,
                            PyObject *param,
                            jclass paramType,
                            int paramTypeId,
                            int pos) {
    jvalue ret;
    ret.l = NULL;

    switch(paramTypeId) {

    case JCHAR_ID: {
        char *val;

        if(param == Py_None ||
           !PyString_Check(param) ||
           PyString_GET_SIZE(param) != 1) {
            
            PyErr_Format(PyExc_TypeError,
                         "Expected char parameter at %i",
                         pos + 1);
            return ret;
        }

        val = PyString_AsString(param);
        ret.c = (jchar) val[0];
        return ret;
    }

    case JSTRING_ID: {
        jstring   jstr;
        char     *val;
            
        // none is okay, we'll set a null
        if(param == Py_None) {
            ret.l = NULL;
        } else if(pyjobject_check(param)) {
            // if they pass in a pyjobject with java.lang.String inside it
            PyJObject *obj = (PyJObject*) param;
            if(!(*env)->IsInstanceOf(env, obj->object, JSTRING_TYPE)) {
                PyErr_Format(PyExc_TypeError,
                        "Expected string parameter at %i.",
                        pos + 1);
                return ret;
            }

            ret.l = obj->object;
            return ret;
        } else {
            // we could just convert it to a string...
            if(!PyString_Check(param)) {
                PyErr_Format(PyExc_TypeError,
                             "Expected string parameter at %i.",
                             pos + 1);
                return ret;
            }
                
            val   = PyString_AsString(param);
            jstr  = (*env)->NewStringUTF(env, (const char *) val);
            
            ret.l = jstr;
        }
        
        return ret;
    }

    case JARRAY_ID: {
        jobjectArray obj = NULL;
        
        if(param == Py_None) {
            ;
        }
#if USE_NUMPY
        else if(npy_array_check(param)) {
            jarray arr;
            jclass arrclazz;

            arr = convert_pyndarray_jprimitivearray(env, param, paramType);
            if(arr == NULL) {
                PyErr_Format(PyExc_TypeError,
                        "No JEP numpy support for type at parameter %i.",
                        pos + 1);
                return ret;
            }

            arrclazz = (*env)->GetObjectClass(env, arr);
            if(!(*env)->IsAssignableFrom(env, arrclazz, paramType)) {
                PyErr_Format(PyExc_TypeError,
                        "numpy array type at parameter %i is incompatible with Java.",
                        pos + 1);
                return ret;
            }

            ret.l = arr;
            return ret;
        }
#endif
        else {
            PyJArrayObject *ar;
            
            if(!pyjarray_check(param)) {
                PyErr_Format(PyExc_TypeError,
                             "Expected jarray parameter at %i.",
                             pos + 1);
                return ret;
            }
            
            ar = (PyJArrayObject *) param;
            
            if(!(*env)->IsAssignableFrom(env,
                                         ar->clazz,
                                         paramType)) {
                PyErr_Format(PyExc_TypeError,
                             "Incompatible array type at parameter %i.",
                             pos + 1);
                return ret;
            }

            // since this method is called before the value is used,
            // release the pinned array from here.
            pyjarray_release_pinned((PyJArrayObject *) param, 0);
            obj = ((PyJArrayObject *) param)->object;
        }
        
        ret.l = obj;
        return ret;
    }

    case JCLASS_ID: { 
        jobject obj = NULL;
        // none is okay, we'll translate to null
        if(param == Py_None)
            ;
        else {
            if(!pyjclass_check(param)) {
                PyErr_Format(PyExc_TypeError,
                             "Expected class parameter at %i.",
                             pos + 1);
                return ret;
            }

            obj = ((PyJObject *) param)->clazz;
        }

        ret.l = obj;
        return ret;
    }

    case JOBJECT_ID: { 
        jobject obj = NULL;
        // none is okay, we'll translate to null
        if(param == Py_None) {
            ;
        } else if(PyString_Check(param)) {
            char *val;

            // strings count as objects here
            if(!(*env)->IsAssignableFrom(env,
                                         JSTRING_TYPE,
                                         paramType)) {
                PyErr_Format(
                    PyExc_TypeError,
                    "Tried to set a string on an incomparable parameter %i.",
                    pos + 1);
                return ret;
            }

            val = PyString_AsString(param);
            obj = (*env)->NewStringUTF(env, (const char *) val);
        }
#if USE_NUMPY
        else if(npy_array_check(param)) {
            ret.l = convert_pyndarray_jndarray(env, param);
            return ret;
        }
#endif
        else {
            if(!pyjobject_check(param)) {
                PyErr_Format(PyExc_TypeError,
                             "Expected object parameter at %i.",
                             pos + 1);
                return ret;
            }

            // check object itself is assignable to that type.
            if(!(*env)->IsAssignableFrom(env,
                                         ((PyJObject *) param)->clazz,
                                         paramType)) {
                PyErr_Format(PyExc_TypeError,
                             "Incorrect object type at %i.",
                             pos + 1);
                return ret;
            }

            obj = ((PyJObject *) param)->object;
        }
        
        ret.l = obj;
        return ret;
    }

    case JSHORT_ID: {
        if(param == Py_None || !PyInt_Check(param)) {
            PyErr_Format(PyExc_TypeError,
                         "Expected short parameter at %i.",
                         pos + 1);
            return ret;
        }
        
        // precision loss...
        ret.s = (jshort) PyInt_AsLong(param);
        return ret;
    }

    case JINT_ID: {
        if(param == Py_None || !PyInt_Check(param)) {
            PyErr_Format(PyExc_TypeError,
                         "Expected int parameter at %i.",
                         pos + 1);
            return ret;
        }
        
        ret.i = (jint) PyInt_AS_LONG(param);
        return ret;
    }

    case JBYTE_ID: {
        if(param == Py_None || !PyInt_Check(param)) {
            PyErr_Format(PyExc_TypeError,
                         "Expected byte parameter at %i.",
                         pos + 1);
            return ret;
        }

        ret.b = (jbyte) PyInt_AS_LONG(param);
        return ret;
    }

    case JDOUBLE_ID: {
        if(param == Py_None || !PyFloat_Check(param)) {
            PyErr_Format(PyExc_TypeError,
                         "Expected double parameter at %i.",
                         pos + 1);
            return ret;
        }
            
        ret.d = (jdouble) PyFloat_AsDouble(param);
        return ret;
    }

    case JFLOAT_ID: {
        if(param == Py_None || !PyFloat_Check(param)) {
            PyErr_Format(PyExc_TypeError,
                         "Expected float parameter at %i.",
                         pos + 1);
            return ret;
        }
        
        // precision loss
        ret.f = (jfloat) PyFloat_AsDouble(param);
        return ret;
    }

    case JLONG_ID: {
        if(PyInt_Check(param))
            ret.j = (jlong) PyInt_AS_LONG(param);
        else if(PyLong_Check(param))
            ret.j = (jlong) PyLong_AsLongLong(param);
        else {
            PyErr_Format(PyExc_TypeError,
                         "Expected long parameter at %i.",
                         pos + 1);
            return ret;
        }
        
        return ret;
    }

    case JBOOLEAN_ID: {
        long bvalue;
        
        if(param == Py_None || !PyInt_Check(param)) {
            PyErr_Format(PyExc_TypeError,
                         "Expected boolean parameter at %i.",
                         pos + 1);
            return ret;
        }
        
        bvalue = PyInt_AsLong(param);
        if(bvalue > 0)
            ret.z = JNI_TRUE;
        else
            ret.z = JNI_FALSE;
        return ret;
    }
        
    } // switch
    
    PyErr_Format(PyExc_TypeError, "Unknown java type at %i.",
                 pos + 1);
    return ret;
}


// convenience function to pull a value from a list of tuples.
// expects tuples to be key, value format.
// parameters cannot be invalid!
// steals all references.
// returns new reference, new reference to Py_None if not found
PyObject* tuplelist_getitem(PyObject *list, PyObject *pyname) {
    Py_ssize_t i, listSize;
    PyObject *ret = NULL;
    
    listSize = PyList_GET_SIZE(list);
    for(i = 0; i < listSize; i++) {
        PyObject *tuple = PyList_GetItem(list, i);        /* borrowed */
        
        if(!tuple || !PyTuple_Check(tuple))
            continue;
        
        if(PyTuple_Size(tuple) == 2) {
            PyObject *key = PyTuple_GetItem(tuple, 0);    /* borrowed */
            if(!key || !PyString_Check(key))
                continue;
            
            if(PyObject_RichCompareBool(key, pyname, Py_EQ)) {
                ret   = PyTuple_GetItem(tuple, 1);        /* borrowed */
                break;
            }
        }
    }
    
    if(!ret)
        ret = Py_None;
    
    Py_INCREF(ret);
    return ret;
}
