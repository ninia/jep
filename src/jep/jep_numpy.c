/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/*
   jep - Java Embedded Python

   Copyright (c) JEP AUTHORS.

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

/* this whole file is a no-op if numpy support is disabled */
#if JEP_NUMPY_ENABLED

/* get numpy config so we can check the version of numpy */
#include "numpy/numpyconfig.h"
#define PY_ARRAY_UNIQUE_SYMBOL JEP_ARRAY_API
/* if we have at least numpy 1.7, let's force the code to be 1.7 compliant */
#ifdef NPY_1_7_API_VERSION
    #define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#endif
#include "numpy/arrayobject.h"


/* this is ugly but the method signature of import_array() changes in Python 3 */
static int numpyInitialized = 0;
#if PY_MAJOR_VERSION >= 3
    static PyObject* init_numpy(void);
#else
    static void init_numpy(void);
#endif


/* internal method */
static PyObject* convert_jprimitivearray_pyndarray(JNIEnv*, jobject, int,
        npy_intp*, int);

/* cache jmethodIDs for performance */
jmethodID ndarrayInit    = NULL;
jmethodID ndarrayGetDims = NULL;
jmethodID ndarrayGetData = NULL;
jmethodID ndarrayIsUnsigned = NULL;


/*
 * Initializes the numpy extension library.  This is required to be called
 * once and only once, before any PyArray_ methods are called. Unfortunately
 * it needs to have a different return type in Python 2 vs Python 3.
 */
#if PY_MAJOR_VERSION >= 3
static PyObject* init_numpy(void)
{
    if (!numpyInitialized) {
        numpyInitialized = 1;
        import_array();
    }
    return NULL;
}
#else
static void init_numpy(void)
{
    if (!numpyInitialized) {
        import_array();
        numpyInitialized = 1;
    }
}
#endif // Python 3 compatibility


int npy_array_check(PyObject *obj)
{
    init_numpy();
    return PyArray_Check(obj);
}


/*
 * Checks if a jobject is an instance of a jep.NDArray
 *
 * @param env   the JNI environment
 * @param obj   the jobject to check
 *
 * @return true if it is an NDArray and jep was compiled with numpy support,
 *          otherwise false
 */
int jndarray_check(JNIEnv *env, jobject obj)
{
    int ret = (*env)->IsInstanceOf(env, obj, JEP_NDARRAY_TYPE);
    if (process_java_exception(env)) {
        return JNI_FALSE;
    }

    return ret;
}


/*
 * Converts a numpy ndarray to a Java primitive array.
 *
 * @param env          the JNI environment
 * @param param        the ndarray to convert
 * @param desiredType  the desired type of the resulting primitive array
 *
 * @return a Java primitive array, or NULL if there were errors
 */
jarray convert_pyndarray_jprimitivearray(JNIEnv* env,
        PyObject *param,
        jclass desiredType)
{
    jarray         arr    = NULL;
    PyArrayObject *copy   = NULL;
    enum NPY_TYPES paType;
    jsize          sz;

    if (!npy_array_check(param)) {
        PyErr_Format(PyExc_TypeError, "convert_pyndarray must receive an ndarray");
        return NULL;
    }

    // determine what we can about the pyarray that is to be converted
    sz = (jsize) PyArray_Size(param);
    paType = PyArray_TYPE((PyArrayObject *) param);

    copy = PyArray_GETCONTIGUOUS((PyArrayObject*) param);
    if ((*env)->IsSameObject(env, desiredType, JBOOLEAN_ARRAY_TYPE)
            && (paType == NPY_BOOL)) {
        arr = (*env)->NewBooleanArray(env, sz);
    } else if ((*env)->IsSameObject(env, desiredType, JBYTE_ARRAY_TYPE)
               && (paType == NPY_BYTE || paType == NPY_UBYTE)) {
        arr = (*env)->NewByteArray(env, sz);
    } else if ((*env)->IsSameObject(env, desiredType, JSHORT_ARRAY_TYPE)
               && (paType == NPY_INT16 || paType == NPY_UINT16)) {
        arr = (*env)->NewShortArray(env, sz);
    } else if ((*env)->IsSameObject(env, desiredType, JINT_ARRAY_TYPE)
               && (paType == NPY_INT32 || paType == NPY_UINT32)) {
        arr = (*env)->NewIntArray(env, sz);
    } else if ((*env)->IsSameObject(env, desiredType, JLONG_ARRAY_TYPE)
               && (paType == NPY_INT64 || paType == NPY_UINT64)) {
        arr = (*env)->NewLongArray(env, sz);
    } else if ((*env)->IsSameObject(env, desiredType, JFLOAT_ARRAY_TYPE)
               && (paType == NPY_FLOAT32)) {
        arr = (*env)->NewFloatArray(env, sz);
    } else if ((*env)->IsSameObject(env, desiredType, JDOUBLE_ARRAY_TYPE)
               && (paType == NPY_FLOAT64)) {
        arr = (*env)->NewDoubleArray(env, sz);
    } else {
        Py_XDECREF(copy);
        PyErr_Format(PyExc_RuntimeError,
                     "Error matching ndarray.dtype to Java primitive type");
        return NULL;
    }

    /*
     * java exception could potentially be OutOfMemoryError if it
     * couldn't allocate the array
     */
    if (process_java_exception(env) || !arr) {
        Py_XDECREF(copy);
        return NULL;
    }

    // if arr was allocated, we already know it matched the python array type
    if (paType == NPY_BOOL) {
        (*env)->SetBooleanArrayRegion(env, arr, 0, sz,
                                      (const jboolean *) PyArray_DATA(copy));
    } else if (paType == NPY_BYTE || paType == NPY_UBYTE) {
        (*env)->SetByteArrayRegion(env, arr, 0, sz, (const jbyte *) PyArray_DATA(copy));
    } else if (paType == NPY_INT16 || paType == NPY_UINT16) {
        (*env)->SetShortArrayRegion(env, arr, 0, sz,
                                    (const jshort *) PyArray_DATA(copy));
    } else if (paType == NPY_INT32 || paType == NPY_UINT32) {
        (*env)->SetIntArrayRegion(env, arr, 0, sz, (const jint *) PyArray_DATA(copy));
    } else if (paType == NPY_INT64 || paType == NPY_UINT64) {
        (*env)->SetLongArrayRegion(env, arr, 0, sz, (const jlong *) PyArray_DATA(copy));
    } else if (paType == NPY_FLOAT32) {
        (*env)->SetFloatArrayRegion(env, arr, 0, sz,
                                    (const jfloat *) PyArray_DATA(copy));
    } else if (paType == NPY_FLOAT64) {
        (*env)->SetDoubleArrayRegion(env, arr, 0, sz,
                                     (const jdouble *) PyArray_DATA(copy));
    }

    Py_XDECREF(copy);

    if (process_java_exception(env)) {
        PyErr_Format(PyExc_RuntimeError, "Error setting Java primitive array region");
        return NULL;
    }

    return arr;
}


/*
 * Convert a numpy ndarray to a jep.NDArray.
 *
 * @param env    the JNI environment
 * @param pyobj  the numpy ndarray to convert
 *
 * @return a new jep.NDArray or NULL if errors are encountered
 */
jobject convert_pyndarray_jndarray(JNIEnv *env, PyObject *pyobj)
{
    npy_intp      *dims      = NULL;
    jint          *jdims     = NULL;
    jobject        jdimObj   = NULL;
    jobject        primitive = NULL;
    jobject        result    = NULL;
    PyArrayObject *pyarray   = (PyArrayObject*) pyobj;
    enum NPY_TYPES paType    = 0;
    jclass         desiredType = NULL;
    int            ndims     = 0;
    int            i;
    jboolean       usigned   = 0;

    init_numpy();
    if (!JNI_METHOD(ndarrayInit, env, JEP_NDARRAY_TYPE, "<init>",
                    "(Ljava/lang/Object;Z[I)V")) {
        process_java_exception(env);
        return NULL;
    }

    // setup the int[] constructor arg
    ndims = PyArray_NDIM(pyarray);
    dims = PyArray_DIMS(pyarray);
    jdims = malloc(((int) ndims) * sizeof(jint));
    for (i = 0; i < ndims; i++) {
        jdims[i] = (jint) dims[i];
    }

    jdimObj = (*env)->NewIntArray(env, ndims);
    if (process_java_exception(env) || !jdimObj) {
        free(jdims);
        return NULL;
    }

    (*env)->SetIntArrayRegion(env, jdimObj, 0, ndims, jdims);
    free(jdims);
    if (process_java_exception(env)) {
        return NULL;
    }

    paType = PyArray_TYPE(pyarray);
    if (paType == NPY_BOOL) {
        desiredType = JBOOLEAN_ARRAY_TYPE;
    } else if (paType == NPY_BYTE) {
        desiredType = JBYTE_ARRAY_TYPE;
    } else if (paType == NPY_UBYTE) {
        desiredType = JBYTE_ARRAY_TYPE;
        usigned = 1;
    } else if (paType == NPY_INT16) {
        desiredType = JSHORT_ARRAY_TYPE;
    } else if (paType == NPY_UINT16) {
        desiredType = JSHORT_ARRAY_TYPE;
        usigned = 1;
    } else if (paType == NPY_INT32) {
        desiredType = JINT_ARRAY_TYPE;
    } else if (paType == NPY_INT32) {
        desiredType = JINT_ARRAY_TYPE;
        usigned = 1;
    } else if (paType == NPY_INT64) {
        desiredType = JLONG_ARRAY_TYPE;
    } else if (paType == NPY_UINT64) {
        desiredType = JLONG_ARRAY_TYPE;
        usigned = 1;
    } else if (paType == NPY_FLOAT32) {
        desiredType = JFLOAT_ARRAY_TYPE;
    } else if (paType == NPY_FLOAT64) {
        desiredType = JDOUBLE_ARRAY_TYPE;
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Unable to determine corresponding Java type for ndarray: %d", paType);
        return NULL;
    }

    // setup the primitive array arg
    primitive = convert_pyndarray_jprimitivearray(env, pyobj, desiredType);
    if (!primitive) {
        return NULL;
    }

    result = (*env)->NewObject(env, JEP_NDARRAY_TYPE, ndarrayInit, primitive,
                               usigned, jdimObj);
    if (process_java_exception(env) || !result) {
        return NULL;
    }

    return result;
}

/*
 * Converts a Java primitive array to a numpy ndarray.
 *
 * @param env   the JNI environment
 * @param jo    the Java primitive array
 * @param ndims the number of dimensions of the output ndarray
 * @param dims  the dimensions of the output ndarray
 *
 * @return an ndarray of matching dtype and dimensions
 */
PyObject* convert_jprimitivearray_pyndarray(JNIEnv *env,
        jobject jo,
        int ndims,
        npy_intp *dims,
        int usigned)
{
    PyObject *pyob = NULL;
    int i           = 0;
    size_t dimsize  = 1;

    for (i = 0; i < ndims; i++) {
        dimsize *= (size_t) dims[i];
    }

    if ((*env)->IsInstanceOf(env, jo, JBOOLEAN_ARRAY_TYPE)) {
        pyob = PyArray_SimpleNew(ndims, dims, NPY_BOOL);
        (*env)->GetBooleanArrayRegion(env, jo, 0, (jsize) dimsize,
                                      PyArray_DATA((PyArrayObject *) pyob));
    } else if ((*env)->IsInstanceOf(env, jo, JBYTE_ARRAY_TYPE)) {
        pyob = PyArray_SimpleNew(ndims, dims,  usigned ? NPY_UBYTE : NPY_BYTE);
        (*env)->GetByteArrayRegion(env, jo, 0, (jsize) dimsize,
                                   PyArray_DATA((PyArrayObject *) pyob));
    } else if ((*env)->IsInstanceOf(env, jo, JSHORT_ARRAY_TYPE)) {
        pyob = PyArray_SimpleNew(ndims, dims, usigned ? NPY_UINT16 : NPY_INT16);
        (*env)->GetShortArrayRegion(env, jo, 0, (jsize) dimsize,
                                    PyArray_DATA((PyArrayObject *) pyob));
    } else if ((*env)->IsInstanceOf(env, jo, JINT_ARRAY_TYPE)) {
        pyob = PyArray_SimpleNew(ndims, dims, usigned ? NPY_UINT32 : NPY_INT32);
        (*env)->GetIntArrayRegion(env, jo, 0, (jsize) dimsize,
                                  PyArray_DATA((PyArrayObject *) pyob));
    } else if ((*env)->IsInstanceOf(env, jo, JLONG_ARRAY_TYPE)) {
        pyob = PyArray_SimpleNew(ndims, dims, usigned ? NPY_UINT64 : NPY_INT64);
        (*env)->GetLongArrayRegion(env, jo, 0, (jsize) dimsize,
                                   PyArray_DATA((PyArrayObject *) pyob));
    } else if ((*env)->IsInstanceOf(env, jo, JFLOAT_ARRAY_TYPE)) {
        pyob = PyArray_SimpleNew(ndims, dims, NPY_FLOAT32);
        (*env)->GetFloatArrayRegion(env, jo, 0, (jsize) dimsize,
                                    PyArray_DATA((PyArrayObject *) pyob));
    } else if ((*env)->IsInstanceOf(env, jo, JDOUBLE_ARRAY_TYPE)) {
        pyob = PyArray_SimpleNew(ndims, dims, NPY_FLOAT64);
        (*env)->GetDoubleArrayRegion(env, jo, 0, (jsize) dimsize,
                                     PyArray_DATA((PyArrayObject *) pyob));
    }

    return pyob;
}

/*
 * Converts a jep.NDArray to a numpy ndarray.
 *
 * @param env    the JNI environment
 * @param obj    the jep.NDArray to convert
 *
 * @return       a numpy ndarray, or NULL if there were errors
 */
PyObject* convert_jndarray_pyndarray(JNIEnv *env, jobject obj)
{
    npy_intp  *dims    = NULL;
    jobject    jdimObj = NULL;
    jint      *jdims   = NULL;
    jobject    data    = NULL;
    PyObject  *result  = NULL;
    jsize      ndims   = 0;
    jboolean   usigned = 0;
    int        i;

    init_numpy();

    if (!JNI_METHOD(ndarrayGetDims, env, JEP_NDARRAY_TYPE, "getDimensions",
                    "()[I")) {
        process_java_exception(env);
        return NULL;
    }

    if (!JNI_METHOD(ndarrayGetData, env, JEP_NDARRAY_TYPE, "getData",
                    "()Ljava/lang/Object;")) {
        process_java_exception(env);
        return NULL;
    }

    if (!JNI_METHOD(ndarrayIsUnsigned, env, JEP_NDARRAY_TYPE, "isUnsigned",
                    "()Z")) {
        process_java_exception(env);
        return NULL;
    }

    usigned = (*env)->CallBooleanMethod(env, obj, ndarrayIsUnsigned);
    if (process_java_exception(env)) {
        return NULL;
    }

    // set up the dimensions for conversion
    jdimObj = (*env)->CallObjectMethod(env, obj, ndarrayGetDims);
    if (process_java_exception(env) || !jdimObj) {
        return NULL;
    }

    ndims = (*env)->GetArrayLength(env, jdimObj);
    if (ndims < 1) {
        PyErr_Format(PyExc_ValueError, "ndarrays must have at least one dimension");
        return NULL;
    }

    jdims = (*env)->GetIntArrayElements(env, jdimObj, 0);
    if (process_java_exception(env) || !jdimObj) {
        return NULL;
    }

    dims = malloc(((int) ndims) * sizeof(npy_intp));
    for (i = 0; i < ndims; i++) {
        dims[i] = jdims[i];
    }
    (*env)->ReleaseIntArrayElements(env, jdimObj, jdims, JNI_ABORT);
    (*env)->DeleteLocalRef(env, jdimObj);

    // get the primitive array and convert it
    data = (*env)->CallObjectMethod(env, obj, ndarrayGetData);
    if (process_java_exception(env) || !data) {
        return NULL;
    }

    result = convert_jprimitivearray_pyndarray(env, data, ndims, dims, usigned);
    if (!result) {
        process_java_exception(env);
    }

    // primitive arrays can be large, encourage garbage collection
    (*env)->DeleteLocalRef(env, data);
    free(dims);
    return result;
}


#endif // if numpy support is enabled


