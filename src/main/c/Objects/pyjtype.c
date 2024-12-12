/*
   jep - Java Embedded Python

   Copyright (c) 2020-2022 JEP AUTHORS.

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

static PyTypeObject PyJType_Type;

static PyTypeObject* pyjtype_get_cached(JNIEnv*, PyObject*, jclass);
static int addMethods(JNIEnv*, PyObject*, jclass);

/*
* Flag to indicate if methods have been added to static types. Most types are
* reinitialized for each interpreter but static types are shared between
* interpreters and must be initialized only once.
*/
static int staticTypesInitialized = 0;

/*
 * Populate pyjmethods for a type and add it to the cache. This is for custom
 * types with extra logic in c since we are not able to add pyjmethods before
 * creating the type.
 *
 * Returns a borrowed reference to the type on success, NULL on failure.
 */
static PyTypeObject* addCustomTypeToTypeDict(JNIEnv *env, PyObject* fqnToPyType,
        jclass class, PyTypeObject *type)
{
    if (PyDict_SetItemString(fqnToPyType, type->tp_name, (PyObject*) type)) {
        return NULL;
    }
    /*
     * The custom wrapper types need to have their methods added since they are
     * not defined when the type is created. None of the types have fields so
     * no need to addFields.
     */
    if (addMethods(env, type->tp_dict, class) != 0) {
        return NULL;
    }
    return type;
}

/*
 * Create a type from a spec and add it to the cache. This accepts a base type
 * which can be NULL for interfaces that are not extending another interface.
 * This will also populate the pyjmethods for the created type.
 *
 * Returns a borrowed reference to the type on success, NULL on failure.
 */
static PyTypeObject* addSpecToTypeDict(JNIEnv *env, PyObject* fqnToPyType,
                                       jclass class, PyType_Spec *spec, PyTypeObject *base)
{
    /* TODO Starting in 3.10 bases can be a single type so there will be no need to make a tuple. */
    PyObject *bases = NULL;
    if (base) {
        bases = PyTuple_Pack(1, base);
        if (!bases) {
            return NULL;
        }
    }
    PyTypeObject *type = (PyTypeObject*) PyType_FromSpecWithBases(spec, bases);
    Py_XDECREF(bases);
    if (!type) {
        return NULL;
    }
    PyTypeObject *result = addCustomTypeToTypeDict(env, fqnToPyType, class, type);
    Py_DECREF(type);
    return result;
}

/*
 * Populate the cache of types with the types that have custom logic defined
 * in c. We need to ensure that the inheritance tree is built in the correct
 * order, i.e. from the top down.  For example, we need to set the base type
 * for PyJCollection to PyJIterable before we set the base type for PyJList
 * to PyJCollection. Interfaces that are not extending another interface
 * should not set the base type because interfaces are added to Python types
 * using multiple inheritance and only one superclass can define a custom
 * structure.
 *
 * Returns 0 on success and -1 on failure
 */
static int populateCustomTypeDict(JNIEnv *env, PyObject* fqnToPyType)
{
    if (!addSpecToTypeDict(env, fqnToPyType, JAUTOCLOSEABLE_TYPE,
                           &PyJAutoCloseable_Spec, NULL)) {
        return -1;
    }
    if (!addSpecToTypeDict(env, fqnToPyType, JITERATOR_TYPE, &PyJIterator_Spec,
                           NULL)) {
        return -1;
    }
    PyTypeObject *pyjiterable = addSpecToTypeDict(env, fqnToPyType, JITERABLE_TYPE,
                                &PyJIterable_Spec, NULL);
    if (!pyjiterable) {
        return -1;
    }
    PyTypeObject *pyjcollection = addSpecToTypeDict(env, fqnToPyType,
                                  JCOLLECTION_TYPE, &PyJCollection_Spec, pyjiterable);
    if (!pyjcollection) {
        return -1;
    }
    if (!addSpecToTypeDict(env, fqnToPyType, JLIST_TYPE, &PyJList_Spec,
                           pyjcollection)) {
        return -1;
    }
    if (!addSpecToTypeDict(env, fqnToPyType, JMAP_TYPE, &PyJMap_Spec, NULL)) {
        return -1;
    }
    if (!addSpecToTypeDict(env, fqnToPyType, JNUMBER_TYPE, &PyJNumber_Spec,
                           &PyJObject_Type)) {
        return -1;
    }
    if (staticTypesInitialized) {
        if (PyDict_SetItemString(fqnToPyType, PyJBuffer_Type.tp_name,
                                 (PyObject * ) &PyJBuffer_Type)) {
            return -1;
        }
        if (PyDict_SetItemString(fqnToPyType, PyJObject_Type.tp_name,
                                 (PyObject * ) &PyJObject_Type)) {
            return -1;
        }
    } else {
        /* TODO In python 3.8 buffer protocol was added to spec so pybuffer type can use a spec */
        if (!addCustomTypeToTypeDict(env, fqnToPyType, JBUFFER_TYPE, &PyJBuffer_Type)) {
            return -1;
        }
        if (!addCustomTypeToTypeDict(env, fqnToPyType, JOBJECT_TYPE, &PyJObject_Type)) {
            return -1;
        }
        staticTypesInitialized = 1;
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
 * Adding empty slots to the type dict to prevent creation of __dict__ for
 * every instance and ensures all attribute access goes through the original
 * Java object using pyjfield, pyjmethod or pyjmultimethod.
 */
static int addSlots(PyObject* dict)
{
    PyObject *slots = PyTuple_New(0);
    if (!slots) {
        return -1;
    }
    if (PyDict_SetItemString(dict, "__slots__", slots)) {
        Py_DECREF(slots);
        return -1;
    }
    Py_DECREF(slots);
    return 0;
}

/*
 * Look up all Java methods and create pyjmethods that are added to the type
 * dict.
 */
static int addMethods(JNIEnv* env, PyObject* dict, jclass clazz)
{
    jobjectArray methodArray = java_lang_Class_getMethods(env, clazz);
    if (!methodArray) {
        process_java_exception(env);
        return -1;
    }
    /**
     * If the clazz is an interface assume it is a functional interface until
     * we find more than one abstract method or no abstract methods.
     * FunctionalInterfaces are automatically callable in Python.
     */
    jboolean functionalInterface = java_lang_Class_isInterface(env, clazz);
    if (process_java_exception(env)) {
        return -1;
    }
    PyObject* oneAbstractPyJMethod = NULL;

    int i;
    int len = (*env)->GetArrayLength(env, methodArray);
    for (i = 0; i < len; i += 1) {
        jobject rmethod = (*env)->GetObjectArrayElement(env, methodArray, i);
        PyJMethodObject *pymethod = PyJMethod_New(env, rmethod);

        if (!pymethod) {
            return -1;
        }

        if (functionalInterface == JNI_TRUE) {
            jint modifiers = java_lang_reflect_Member_getModifiers(env, rmethod);
            jboolean isAbstract;
            if (process_java_exception(env)) {
                Py_DECREF(pymethod);
                return -1;
            }
            isAbstract = java_lang_reflect_Modifier_isAbstract(env, modifiers);
            if (process_java_exception(env)) {
                Py_DECREF(pymethod);
                return -1;
            }
            if (isAbstract) {
                if (oneAbstractPyJMethod) {
                    /*
                    * If there is already one abstract method and this method is also
                    * abstract then this isn't a functional interface and there is no need
                    * to keep track of abstract methods.
                    */
                    functionalInterface = JNI_FALSE;
                    oneAbstractPyJMethod = NULL;
                } else {
                    oneAbstractPyJMethod = (PyObject*) pymethod;
                }
            }
        }

        /*
         * For every method of this name, check to see if a PyJMethod or
         * PyJMultiMethod is already in the cache with the same name. If
         * so, turn it into a PyJMultiMethod or add it to the existing
         * PyJMultiMethod.
         */
        PyObject* cached = PyDict_GetItem(dict, pymethod->pyMethodName);
        if (cached == NULL) {
            if (PyDict_SetItem(dict, pymethod->pyMethodName,
                               (PyObject*) pymethod) != 0) {
                Py_DECREF(pymethod);
                return -1;
            }
        } else if (PyJMethod_Check(cached)) {
            PyObject* multimethod = PyJMultiMethod_New((PyObject*) pymethod, cached);
            if (PyDict_SetItem(dict, pymethod->pyMethodName, multimethod) != 0) {
                Py_DECREF(multimethod);
                return -1;
            }
            Py_DECREF(multimethod);
        } else if (PyJMultiMethod_Check(cached)) {
            PyJMultiMethod_Append(cached, (PyObject*) pymethod);
        }

        Py_DECREF(pymethod);
        (*env)->DeleteLocalRef(env, rmethod);
    }
    (*env)->DeleteLocalRef(env, methodArray);
    if (functionalInterface == JNI_TRUE && oneAbstractPyJMethod) {
        return PyDict_SetItemString(dict, "__call__", oneAbstractPyJMethod);
    }
    return 0;
}

/*
 * Look up all Java fields and create pyjmethods that are added to the type
 * dict.
 */
static int addFields(JNIEnv* env, PyObject* dict, jclass clazz)
{
    jobjectArray fieldArray = java_lang_Class_getDeclaredFields(env, clazz);
    if (!fieldArray) {
        process_java_exception(env);
        return -1;
    }
    int i;
    int len = (*env)->GetArrayLength(env, fieldArray);
    for (i = 0; i < len; i += 1) {
        jobject rfield = (*env)->GetObjectArrayElement(env, fieldArray, i);
        jint mods = java_lang_reflect_Member_getModifiers(env, rfield);
        if (process_java_exception(env)) {
            return -1;
        }
        jboolean public = java_lang_reflect_Modifier_isPublic(env, mods);
        if (process_java_exception(env)) {
            return -1;
        }
        if (public) {
            PyJFieldObject *pyjfield = PyJField_New(env, rfield);

            if (!pyjfield) {
                return -1;
            }

            if (PyDict_SetItem(dict, pyjfield->pyFieldName, (PyObject*) pyjfield) != 0) {
                return -1;
            }

            Py_DECREF(pyjfield);
        }
        (*env)->DeleteLocalRef(env, rfield);
    }
    (*env)->DeleteLocalRef(env, fieldArray);
    return 0;
}

/*
 * Create a new PythonType object for the given Java class. The Python type
 * hierarchy will mirror the Java class hierarchy.
 */
static PyTypeObject* pyjtype_get_new(JNIEnv *env, PyObject *fqnToPyType,
                                     PyObject *typeName, jclass clazz)
{

    if (!(*env)->IsAssignableFrom(env, clazz, JOBJECT_TYPE)) {
        PyErr_Format(PyExc_TypeError, "Cannot create a pyjtype for primitive type: %s",
                     PyUnicode_AsUTF8(typeName));
        return NULL;
    }
    if (!PyJType_Type.tp_base) {
        PyJType_Type.tp_base = &PyType_Type;
    }
    if (PyType_Ready(&PyJType_Type) < 0) {
        return NULL;
    }

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
    if ((addSlots(dict) != 0) || (addMethods(env, dict, clazz) != 0)
            || (addFields(env, dict, clazz) != 0)) {
        Py_DECREF(bases);
        Py_DECREF(dict);
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
        type = (PyTypeObject*) PyObject_CallFunctionObjArgs((PyObject*) &PyJType_Type,
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
    PyObject* modjep = pyembed_get_jep_module();
    if (!modjep) {
        return NULL;
    }
    PyObject* fqnToPyType = PyObject_GetAttrString(modjep, "__javaTypeCache__");
    if (!fqnToPyType) {
        return NULL;
    } else if (PyDict_Size(fqnToPyType) == 0) {
        if (populateCustomTypeDict(env, fqnToPyType)) {
            Py_DECREF(fqnToPyType);
            return NULL;
        }
    }
    PyTypeObject* result = pyjtype_get_cached(env, fqnToPyType, clazz);
    Py_DECREF(fqnToPyType);
    return result;
}

/*
 * Given a base type for a class, merge the mro from the base into the mro list
 * for a new type. The mro entries from the base type that are not in the list
 * will be appended to the list.
 *
 * Returns 0 on success. Returns -1 and sets an exception if an error occurs.
 */
static int merge_mro(PyTypeObject* base, PyObject* mro_list)
{
    PyObject* base_mro = base->tp_mro;
    if (!base_mro || !PyTuple_Check(base_mro)) {
        PyErr_SetString(PyExc_TypeError, "Invalid mro in base type.");
        return -1;
    }
    Py_ssize_t n = PyTuple_Size(base_mro);
    Py_ssize_t i = 0;
    for (i = 0; i < n; i++) {
        PyObject *next = PyTuple_GetItem(base_mro, i);
        int contains = PySequence_Contains(mro_list, next);
        if (contains < 0) {
            return -1;
        } else if (contains == 0) {
            if (PyList_Append(mro_list, next)) {
                return -1;
            }
        }
    }
    return 0;
}

/*
 * Define a custom MRO for Python types that mirror Java classes. Java classes
 * can define a class hierarchy that is not able to be resolved with the
 * default Python MRO. Since Java does not support actual multiple inheritance
 * a simplified MRO does not cause problems as long as the non-interface
 * classes are in order. This simply merges all the mro's from the base types
 * in order. The first base will be the non-interface super class so all
 * non-interface inheritance takes precedence over interfaces.
 */
static PyObject* pyjtype_mro(PyObject* self, PyObject* unused)
{
    PyTypeObject* type = (PyTypeObject*) self;
    PyObject* mro_list = PyList_New(0);
    if (!mro_list) {
        return NULL;
    }
    if (PyList_Append(mro_list, self)) {
        Py_DECREF(mro_list);
        return NULL;
    }

    PyObject *bases = type->tp_bases;
    Py_ssize_t n = PyTuple_Size(bases);
    Py_ssize_t i = 0;
    for (i = 0; i < n; i++) {
        PyObject *base = PyTuple_GetItem(bases, i);
        int contains = PySequence_Contains(mro_list, base);
        if (contains < 0) {
            Py_DECREF(mro_list);
            return NULL;
        } else if (contains == 0) {
            if (PyList_Append(mro_list, base)) {
                Py_DECREF(mro_list);
                return NULL;
            }
        }
        if (merge_mro(((PyTypeObject*) base), mro_list)) {
            Py_DECREF(mro_list);
            return NULL;
        }
    }
    PyObject* mro_tuple = PySequence_Tuple(mro_list);
    Py_DECREF(mro_list);
    return mro_tuple;
}


static PyMethodDef pyjtype_methods[] = {
    {
        "mro",
        pyjtype_mro,
        METH_NOARGS,
        "Implements a custom MRO algorithm compatible with all Java Class hierarchies"
    },

    { NULL, NULL }
};


static PyTypeObject PyJType_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "PyJType",                                /* tp_name */
    0,                                        /* tp_basicsize */
    0,                                        /* tp_itemsize */
    0,                                        /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_TYPE_SUBCLASS,                 /* tp_flags */
    0,                                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    pyjtype_methods,                          /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};

