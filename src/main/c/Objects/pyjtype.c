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
 *
 * Returns 0 on success and -1 on failure
 */
static int populateBaseTypeDict(PyObject* fqnToPyType)
{
    int i;
    for (i = 0 ; baseTypes[i] != NULL ; i += 1) {
        PyTypeObject* type = baseTypes[i];
        if (PyDict_SetItemString(fqnToPyType, type->tp_name, (PyObject*) type)) {
            return -1;
        }
    }
    return 0;
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

/*
 * Create a tuple containing the Python types that map to Java super class
 * and interfaces implemented by a class. The super class of a Java class will
 * be the first type and any interfaces are addded after. If the class is an
 * interface it will not have a super class so the result will contain only
 * interfaces that the class extends. Interfaces that do not extend any other
 * interfaces will return an empty tuple. During type creation the empty tuple
 * will cause the Python object type to be used as the base type. Interfaces
 * cannot use pyjobject as the base type because pyjobject uses a custom
 * struct and Python will not allow multiple inheritance of multiple types
 * with a struct, even if they are the same struct.
 */
static PyObject* getBaseTypes(JNIEnv *env, PyObject *fqnToPyType, jclass clazz)
{
    /* Need to count the base types before tuple creation */
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
        /* When clazz is not an interface there is at least one base for the super class */
        numBases += 1;
    }
    if (interfaces) {
        /* Each interface needs to be added as a base type */
        numBases += (*env)->GetArrayLength(env, interfaces);
    }
    PyObject* bases = PyTuple_New(numBases);
    if (!bases) {
        return NULL;
    }
    /* The next index to be set in bases */
    Py_ssize_t baseIdx = 0;
    if (interface != JNI_TRUE) {
        /* For classes that are not interfaces, the super class is the first type */
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
    /* The index of the next interface to get from interfaces */
    jint interfacesIdx = 0;
    while (baseIdx < numBases) {
        /* Add a type to bases for each interface */
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
    return bases;
}

/*
 * Create a new PythonType object for the given Java class. The Python type
 * hierarchy will mirror the Java class hierarchy.
 */
static PyTypeObject* pyjtype_get_new(JNIEnv *env, PyObject *fqnToPyType,
                                     PyObject *typeName, jclass clazz)
{
    /* The Python types for the Java super class and any interfaces. */
    PyObject* bases = getBaseTypes(env, fqnToPyType, clazz);
    if (!bases) {
        return NULL;
    }

    /*
     * A dict is required for creating a type. This will only contain
     * __module__ because pyjobject handles attribute look up. In the
     * future consider placing pyjfields or pyjmethods in this dict
     * to simplify the structure of each object.
     */
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
        /*
        * This is the actual type creation. It is equivalent of calling
         * type(shortName, bases, dict) in python.
         * See https://docs.python.org/3/library/functions.html#type
         */
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
 * has already been provided. This function was separated from PyJType_Get
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
    PyObject* modjep = PyImport_ImportModule("_jep");
    if (!modjep) {
        return NULL;
    }
    PyObject* fqnToPyType = PyObject_GetAttrString(modjep, "__javaTypeCache__");
    if (!fqnToPyType) {
        Py_DECREF(modjep);
        return NULL;
    } else if (PyDict_Size(fqnToPyType) == 0) {
        if (populateBaseTypeDict(fqnToPyType)) {
            Py_DECREF(modjep);
            Py_DECREF(fqnToPyType);
            return NULL;
        }
    }
    PyTypeObject* result = pyjtype_get_cached(env, fqnToPyType, clazz);
    Py_DECREF(modjep);
    Py_DECREF(fqnToPyType);
    return result;
}
