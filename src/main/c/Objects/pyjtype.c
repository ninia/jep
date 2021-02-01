/*
   jep - Java Embedded Python

   Copyright (c) 2020-2021 JEP AUTHORS.

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

/* NULL terminated array of type that are statically defined */
static PyTypeObject* baseTypes[] = {
    &PyJAutoCloseable_Type,
    &PyJBuffer_Type,
    &PyJCollection_Type,
    &PyJIterable_Type,
    &PyJIterator_Type,
    &PyJList_Type,
    &PyJMap_Type,
    &PyJNumber_Type,
    &PyJObject_Type,
    NULL
};

static PyTypeObject* pyjtype_get_cached(JNIEnv*, PyObject*, jclass);

/*
 * Add the statically defined wrapper types to the base cache of types.
 */
static PyObject* createBaseTypeDict()
{
    PyObject* fqnToPyType = PyDict_New();
    int i;
    for (i = 0 ; baseTypes[i] != NULL ; i += 1) {
        PyTypeObject* type = baseTypes[i];
        if (PyDict_SetItemString(fqnToPyType, type->tp_name, (PyObject*) type)) {
            Py_DECREF(fqnToPyType);
            return NULL;
        }
    }
    return fqnToPyType;
}

/*
 * For a python unicode object containing a java class name, such as java.util.Date
 * this will split it into a package/module name such as java.util and the short
 * class name of Date.
 */
static void parseModule(PyObject* typeName, PyObject** moduleName,
                        PyObject** shortName)
{
    Py_ssize_t cnameLen;
    const char* cname = PyUnicode_AsUTF8AndSize(typeName, &cnameLen);
    const char *s = strrchr(cname, '.');
    if (s != NULL) {
        Py_ssize_t modLen = (Py_ssize_t)(s - cname);
        *moduleName = PyUnicode_FromStringAndSize(cname, modLen);
        *shortName = PyUnicode_FromStringAndSize(s + 1, cnameLen - modLen - 1);
    } else {
        *moduleName = PyUnicode_FromStringAndSize("__jep__", 7);
        *shortName = typeName;
        Py_INCREF(typeName);
    }
}

static PyTypeObject* pyjtype_get_new(JNIEnv *env, PyObject *fqnToPyType,
                                     PyObject *typeName, jclass clazz)
{
    jint numBases = 0;
    jboolean interface = java_lang_Class_isInterface(env, clazz);
    if (process_java_exception(env)) {
        return NULL;
    }
    jobjectArray interfaces = java_lang_Class_getInterfaces(env, clazz);
    if (!interfaces && process_java_exception(env)) {
        return NULL;
    }
    if (interface != JNI_TRUE) {
        numBases += 1;
    }
    if (interfaces) {
        numBases += (*env)->GetArrayLength(env, interfaces);
    }
    PyObject* bases = PyTuple_New(numBases);
    if (!bases) {
        return NULL;
    }
    Py_ssize_t baseIdx = 0;
    if (interface != JNI_TRUE) {
        jclass super = java_lang_Class_getSuperclass(env, clazz);
        PyObject* superType = (PyObject*) pyjtype_get_cached(env, fqnToPyType, super);
        (*env)->DeleteLocalRef(env, super);
        if (!superType) {
            Py_DECREF(bases);
            return NULL;
        }
        PyTuple_SET_ITEM(bases, baseIdx, superType);
        baseIdx += 1;
    }
    jint interfacesIdx = 0;
    while (baseIdx < numBases) {
        jclass superI = (jclass) (*env)->GetObjectArrayElement(env, interfaces,
                        interfacesIdx);
        interfacesIdx += 1;
        PyObject* superType = (PyObject*) pyjtype_get_cached(env, fqnToPyType, superI);
        (*env)->DeleteLocalRef(env, superI);
        if (!superType) {
            Py_DECREF(bases);
            return NULL;
        }
        PyTuple_SET_ITEM(bases, baseIdx, superType);
        baseIdx += 1;
    }
    (*env)->DeleteLocalRef(env, interfaces);
    PyObject *dict = PyDict_New();
    if (!dict) {
        Py_DECREF(bases);
        return NULL;
    }
    PyObject* moduleName = NULL;
    PyObject* shortName = NULL;
    parseModule(typeName, &moduleName, &shortName);
    PyTypeObject *type = NULL;
    if (moduleName && shortName
            && !PyDict_SetItemString(dict, "__module__", moduleName)) {
        type = (PyTypeObject*) PyObject_CallFunctionObjArgs((PyObject*) &PyType_Type,
                shortName, bases, dict, NULL);
    }
    Py_DECREF(bases);
    Py_DECREF(dict);
    Py_XDECREF(moduleName);
    Py_XDECREF(shortName);
    if (type) {
        PyDict_SetItem(fqnToPyType, typeName, (PyObject*) type);
    }
    return type;
}

/*
 * Either create a new type for the class or return a cached type if a type
 * has already been provided. This function was seperated from PyJType_Get
 * to allow for recursion while looking up super classes without needing to
 * look up the jepThread every time.
 */
static PyTypeObject* pyjtype_get_cached(JNIEnv *env, PyObject *fqnToPyType,
                                        jclass clazz)
{
    jstring className = java_lang_Class_getName(env, clazz);
    if (process_java_exception(env) || !className) {
        return NULL;
    }
    PyObject *pyClassName = jstring_As_PyString(env, className);
    (*env)->DeleteLocalRef(env, className);
    PyTypeObject *pyType = (PyTypeObject*) PyDict_GetItem(fqnToPyType, pyClassName);
    if (pyType) {
        Py_INCREF(pyType);
    } else {
        pyType = pyjtype_get_new(env, fqnToPyType, pyClassName, clazz);
    }
    Py_DECREF(pyClassName);
    return pyType;
}

PyTypeObject* PyJType_Get(JNIEnv *env, jclass clazz)
{
    JepThread* jepThread  = pyembed_get_jepthread();
    if (!jepThread) {
        return NULL;
    }
    if (jepThread->fqnToPyType == NULL) {
        jepThread->fqnToPyType = createBaseTypeDict();
        if (jepThread->fqnToPyType == NULL) {
            return NULL;
        }
    }
    return pyjtype_get_cached(env, jepThread->fqnToPyType, clazz);
}
