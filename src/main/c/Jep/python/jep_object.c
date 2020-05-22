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
*/

#include "Jep.h"
#include "jep_python_PyObject.h"

/*
 * Class:     jep_python_PyObject
 * Method:    getAttr
 * Signature: (JJLjava/lang/String;Ljava/lang/Class;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_jep_python_PyObject_getAttr
(JNIEnv *env, jobject obj, jlong tstate, jlong pyobj, jstring str, jclass clazz)
{
    JepThread  *jepThread;
    PyObject   *pyObject;
    const char *attrName;
    PyObject   *attr;
    jobject     ret = NULL;

    jepThread = (JepThread *) tstate;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return ret;
    }

    if (str == NULL) {
        THROW_JEP(env, "Attribute name cannot be null.");
        return ret;
    }

    pyObject = (PyObject*) pyobj;
    attrName = jstring2char(env, str);

    PyEval_AcquireThread(jepThread->tstate);

    attr = PyObject_GetAttrString(pyObject, attrName);
    if (process_py_exception(env)) {
        goto EXIT;
    }

    ret = PyObject_As_jobject(env, attr, clazz);
    process_py_exception(env);

EXIT:
    Py_XDECREF(attr);
    PyEval_ReleaseThread(jepThread->tstate);
    release_utf_char(env, str, attrName);
    return ret;
}


/*
 * Class:     jep_python_PyObject
 * Method:    setAttr
 * Signature: (JJLjava/lang/String;Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_setAttr
(JNIEnv *env, jobject obj, jlong tstate, jlong pyobj, jstring str,
 jobject jAttr)
{
    JepThread  *jepThread;
    PyObject   *pyObject;
    const char *attrName;
    PyObject   *pyAttr;
    int         ret;

    jepThread = (JepThread *) tstate;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return;
    }

    if (str == NULL) {
        THROW_JEP(env, "Attribute name cannot be null.");
        return;
    }

    pyObject = (PyObject*) pyobj;
    attrName = jstring2char(env, str);

    PyEval_AcquireThread(jepThread->tstate);
    pyAttr = jobject_As_PyObject(env, jAttr);
    if (process_py_exception(env)) {
        goto EXIT;
    }
    ret = PyObject_SetAttrString(pyObject, attrName, pyAttr);
    if (ret == -1) {
        process_py_exception(env);
    }

EXIT:
    PyEval_ReleaseThread(jepThread->tstate);
    release_utf_char(env, str, attrName);
}


/*
 * Class:     jep_python_PyObject
 * Method:    delAttr
 * Signature: (JJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_jep_python_PyObject_delAttr
(JNIEnv *env, jobject obj, jlong tstate, jlong pyobj, jstring str)
{
    JepThread  *jepThread;
    PyObject   *pyObject;
    const char *attrName;
    int         ret;

    jepThread = (JepThread *) tstate;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return;
    }

    if (str == NULL) {
        THROW_JEP(env, "Attribute name cannot be null.");
        return;
    }

    pyObject = (PyObject*) pyobj;
    attrName = jstring2char(env, str);

    PyEval_AcquireThread(jepThread->tstate);
    ret = PyObject_DelAttrString(pyObject, attrName);
    if (ret == -1) {
        process_py_exception(env);
    }

    PyEval_ReleaseThread(jepThread->tstate);
    release_utf_char(env, str, attrName);
}

/*
 * Class:     jep_python_PyObject
 * Method:    equals
 * Signature: (JJLjava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_jep_python_PyObject_equals
(JNIEnv *env, jobject obj, jlong tstate, jlong pyObj, jobject otherObj)
{
    JepThread    *jepThread;
    PyObject     *pyObject;
    PyObject     *otherPyObject;
    int result   = 0;
    jboolean ret = JNI_FALSE;

    jepThread = (JepThread *) tstate;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return ret;
    }

    pyObject = (PyObject*) pyObj;
    PyEval_AcquireThread(jepThread->tstate);
    otherPyObject = jobject_As_PyObject(env, otherObj);
    if (process_py_exception(env)) {
        goto EXIT;
    }

    result = PyObject_RichCompareBool(pyObject, otherPyObject, Py_EQ);
    if (result == -1) {
        process_py_exception(env);
    } else if (result == 1) {
        ret = JNI_TRUE;
    }


EXIT:
    Py_XDECREF(otherPyObject);
    PyEval_ReleaseThread(jepThread->tstate);
    return ret;
}

/*
 * Class:     jep_python_PyObject
 * Method:    toString
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_jep_python_PyObject_toString
(JNIEnv *env, jobject obj, jlong tstate, jlong pyobj)
{
    JepThread  *jepThread;
    PyObject   *pyObject;
    jstring     ret = NULL;

    jepThread = (JepThread *) tstate;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return ret;
    }

    pyObject = (PyObject*) pyobj;

    PyEval_AcquireThread(jepThread->tstate);
    ret = PyObject_As_jstring(env, pyObject);
    process_py_exception(env);
    PyEval_ReleaseThread(jepThread->tstate);
    return ret;
}

/*
 * Class:     jep_python_PyObject
 * Method:    hashCode
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_jep_python_PyObject_hashCode
(JNIEnv *env, jobject obj, jlong tstate, jlong pyobj)
{
    JepThread  *jepThread;
    PyObject   *pyObject;
    Py_hash_t        hash = -1;

    jepThread = (JepThread *) tstate;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return (jlong) hash;
    }

    pyObject = (PyObject*) pyobj;
    PyEval_AcquireThread(jepThread->tstate);
    hash = PyObject_Hash(pyObject);
    process_py_exception(env);
    PyEval_ReleaseThread(jepThread->tstate);
    return (jlong) hash;
}

/*
 * Class:     jep_python_PyObject
 * Method:    as
 * Signature: (Ljava/lang/Class;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_jep_python_PyObject_as
(JNIEnv *env, jobject obj, jlong tstate, jlong pyobj, jclass expectedType)
{
    JepThread  *jepThread;
    PyObject   *pyObject;
    jobject     ret       = NULL;

    jepThread = (JepThread *) tstate;
    if (!jepThread) {
        THROW_JEP(env, "Couldn't get thread objects.");
        return ret;
    }

    pyObject = (PyObject*) pyobj;
    PyEval_AcquireThread(jepThread->tstate);
    ret = PyObject_As_jobject(env, pyObject, expectedType);
    if (!ret) {
        process_py_exception(env);
    }
    PyEval_ReleaseThread(jepThread->tstate);
    return ret;
}
