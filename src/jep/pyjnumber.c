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

// cached methodids
jmethodID intValue    = 0;
jmethodID longValue   = 0;
jmethodID doubleValue = 0;

static PyObject* java_number_to_python(JNIEnv*, PyObject*);
static PyObject* java_number_to_pythonintlong(JNIEnv*, PyObject*);
static PyObject* java_number_to_pythonfloat(JNIEnv*, PyObject*);

/*
 * News up a pyjnumber, which is just a pyjobject with some number methods
 * attached to it.  This should only be called from pyjobject_new().
 */
PyJNumberObject* pyjnumber_new()
{
    // pyjobject will have already initialized PyJNumber_Type
    return PyObject_NEW(PyJNumberObject, &PyJNumber_Type);
}

/*
 * Checks if the object is a pyjnumber.
 */
int pyjnumber_check(PyObject *obj)
{
    if (PyObject_TypeCheck(obj, &PyJNumber_Type)) {
        return 1;
    }
    return 0;
}

#define TO_PYTHON_NUMBER(env, var)\
    if (pyjnumber_check(var)) {\
        var = java_number_to_python(env, var);\
        if (var == NULL){\
            return NULL;\
        }\
    } else if (PyNumber_Check(var)) {\
        Py_INCREF(var);\
    }\
    else {\
        Py_INCREF(Py_NotImplemented);       \
        return Py_NotImplemented;           \
    }\

#define CALL_UNARY(env, methodname, var)\
    TO_PYTHON_NUMBER(env, var);\
    result = methodname(var);\
    Py_DECREF(var);\

#define CALL_BINARY(env, methodname, var1, var2)\
    TO_PYTHON_NUMBER(env, var1);\
    TO_PYTHON_NUMBER(env, var2);\
    result = methodname(var1, var2);\
    Py_DECREF(var1);\
    Py_DECREF(var2);\


static PyObject* pyjnumber_add(PyObject *x, PyObject *y)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_BINARY(env, PyNumber_Add, x, y);
    return result;
}

static PyObject* pyjnumber_subtract(PyObject *x, PyObject *y)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_BINARY(env, PyNumber_Subtract, x, y);
    return result;
}

static PyObject* pyjnumber_multiply(PyObject *x, PyObject *y)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_BINARY(env, PyNumber_Multiply, x, y);
    return result;
}

#if PY_MAJOR_VERSION < 3
static PyObject* pyjnumber_divide(PyObject *x, PyObject *y)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_BINARY(env, PyNumber_Divide, x, y);
    return result;
}
#endif

static PyObject* pyjnumber_remainder(PyObject *x, PyObject *y)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_BINARY(env, PyNumber_Remainder, x, y);
    return result;
}

static PyObject* pyjnumber_divmod(PyObject *x, PyObject *y)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_BINARY(env, PyNumber_Divmod, x, y);
    return result;
}

static PyObject* pyjnumber_power(PyObject *x, PyObject *y, PyObject *z)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    TO_PYTHON_NUMBER(env, x);
    TO_PYTHON_NUMBER(env, y);
    if (z != Py_None) {
        TO_PYTHON_NUMBER(env, z);
    } else {
        Py_INCREF(z);
    }

    result = PyNumber_Power(x, y, z);
    Py_DECREF(x);
    Py_DECREF(y);
    Py_DECREF(z);
    return result;
}

static PyObject* pyjnumber_negative(PyObject *x)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_UNARY(env, PyNumber_Negative, x);
    return result;
}

static PyObject* pyjnumber_positive(PyObject *x)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_UNARY(env, PyNumber_Positive, x);
    return result;
}

static PyObject* pyjnumber_absolute(PyObject *x)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_UNARY(env, PyNumber_Absolute, x);
    return result;
}

static int pyjnumber_nonzero(PyObject *x)
{
    JNIEnv *env    = pyembed_get_env();
    int     result = -1;

    if (pyjnumber_check(x)) {
        x = java_number_to_python(env, x);
        if (x == NULL) {
            return result;
        }
    }

    result = PyObject_IsTrue(x);
    Py_DECREF(x);
    return result;
}

static PyObject* pyjnumber_floordivide(PyObject *x, PyObject *y)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_BINARY(env, PyNumber_FloorDivide, x, y);
    return result;
}

static PyObject* pyjnumber_truedivide(PyObject *x, PyObject *y)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    CALL_BINARY(env, PyNumber_TrueDivide, x, y);
    return result;
}

static PyObject* pyjnumber_index(PyObject *x)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();
    TO_PYTHON_NUMBER(env, x);

    if (PyLong_Check(x)) {
        result = PyNumber_Index(x);
        Py_DECREF(x);
        return result;
    }
#if PY_MAJOR_VERSION < 3
    else if (PyInt_Check(x)) {
        result = PyNumber_Index(x);
        Py_DECREF(x);
        return result;
    }
#endif
    else {
        PyErr_Format(PyExc_TypeError, "list indices must be integers, not %s",
                     Py_TYPE(x)->tp_name);
        return NULL;
    }
}

static PyObject* pyjnumber_int(PyObject *x)
{
    JNIEnv *env = pyembed_get_env();
    return java_number_to_pythonintlong(env, x);
}

#if PY_MAJOR_VERSION < 3
static PyObject* pyjnumber_long(PyObject *x)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    result = java_number_to_pythonintlong(env, x);
    if (result == NULL) {
        return result;
    } else if (PyInt_Check(result)) {
        PyObject *longResult = PyLong_FromLong(PyInt_AS_LONG(result));
        Py_DECREF(result);
        return longResult;
    }
    return result;
}
#endif

static PyObject* pyjnumber_float(PyObject *x)
{
    JNIEnv *env = pyembed_get_env();
    return java_number_to_pythonfloat(env, x);
}

static PyObject* pyjnumber_richcompare(PyObject *self,
                                       PyObject *other,
                                       int opid)
{
    PyObject *result = NULL;
    JNIEnv   *env    = pyembed_get_env();

    TO_PYTHON_NUMBER(env, self);
    TO_PYTHON_NUMBER(env, other);
    result = PyObject_RichCompare(self, other, opid);
    Py_DECREF(self);
    Py_DECREF(other);
    return result;
}


static PyObject* java_number_to_python(JNIEnv *env, PyObject* n)
{
    PyJObject *jnumber  = (PyJObject*) n;

    if ((*env)->IsInstanceOf(env, jnumber->object, JBYTE_OBJ_TYPE) ||
            (*env)->IsInstanceOf(env, jnumber->object, JSHORT_OBJ_TYPE) ||
            (*env)->IsInstanceOf(env, jnumber->object, JINT_OBJ_TYPE) ||
            (*env)->IsInstanceOf(env, jnumber->object, JLONG_OBJ_TYPE)) {
        return java_number_to_pythonintlong(env, n);
    } else {
        return java_number_to_pythonfloat(env, n);
    }
}

static PyObject* java_number_to_pythonintlong(JNIEnv *env, PyObject* n)
{
    jlong      value;
    PyJObject *jnumber  = (PyJObject*) n;

    if (!JNI_METHOD(longValue, env, JNUMBER_TYPE, "longValue", "()J")) {
        process_java_exception(env);
        return NULL;
    }

#if PY_MAJOR_VERSION < 3
    if (!JNI_METHOD(intValue, env, JNUMBER_TYPE, "intValue", "()I")) {

        process_java_exception(env);
        return NULL;
    }

    if ((*env)->IsInstanceOf(env, jnumber->object, JBYTE_OBJ_TYPE) ||
            (*env)->IsInstanceOf(env, jnumber->object, JSHORT_OBJ_TYPE) ||
            (*env)->IsInstanceOf(env, jnumber->object, JINT_OBJ_TYPE)) {
        jint result = (*env)->CallIntMethod(env, jnumber->object, intValue);
        if (process_java_exception(env)) {
            return NULL;
        }
        return PyInt_FromSsize_t(result);
    }
#endif

    value = (*env)->CallLongMethod(env, jnumber->object, longValue);
    if (process_java_exception(env)) {
        return NULL;
    }
    return PyLong_FromLongLong(value);
}


static PyObject* java_number_to_pythonfloat(JNIEnv *env, PyObject* n)
{
    jdouble    value;
    PyJObject *jnumber  = (PyJObject*) n;

    if (!JNI_METHOD(doubleValue, env, JNUMBER_TYPE, "doubleValue", "()D")) {
        process_java_exception(env);
        return NULL;
    }

    value = (*env)->CallDoubleMethod(env, jnumber->object, doubleValue);
    if (process_java_exception(env)) {
        return NULL;
    }
    return PyFloat_FromDouble(value);
}


static PyMethodDef pyjnumber_methods[] = {
    {NULL, NULL, 0, NULL}
};

static PyNumberMethods pyjnumber_number_methods = {
    (binaryfunc) pyjnumber_add,                 /* nb_add */
    (binaryfunc) pyjnumber_subtract,            /* nb_subtract */
    (binaryfunc) pyjnumber_multiply,            /* nb_multiply */
#if PY_MAJOR_VERSION < 3
    (binaryfunc) pyjnumber_divide,              /* nb_divide */
#endif
    (binaryfunc) pyjnumber_remainder,           /* nb_remainder */
    (binaryfunc) pyjnumber_divmod,              /* nb_divmod */
    (ternaryfunc) pyjnumber_power,              /* nb_power */
    (unaryfunc) pyjnumber_negative,             /* nb_neg */
    (unaryfunc) pyjnumber_positive,             /* nb_pos */
    (unaryfunc) pyjnumber_absolute,             /* nb_abs */
    (inquiry) pyjnumber_nonzero,                /* nb_nonzero */
    0,                                          /* nb_invert */
    0,                                          /* nb_lshift */
    0,                                          /* nb_rshift */
    0,                                          /* nb_and */
    0,                                          /* nb_xor */
    0,                                          /* nb_or */
#if PY_MAJOR_VERSION < 3
    0,                                          /* nb_coerce */
#endif
    (unaryfunc) pyjnumber_int,                  /* nb_int */
#if PY_MAJOR_VERSION < 3
    (unaryfunc) pyjnumber_long,                 /* nb_long */
#else
    0,                                          /* nb_reserved */
#endif
    (unaryfunc) pyjnumber_float,                /* nb_float */
#if PY_MAJOR_VERSION < 3
    0,                                          /* nb_oct */
    0,                                          /* nb_hex */
#endif
    0,                                          /* inplace_add */
    0,                                          /* inplace_subtract */
    0,                                          /* inplace_multiply */
#if PY_MAJOR_VERSION < 3
    0,                                          /* inplace_divide */
#endif
    0,                                          /* inplace_remainder */
    0,                                          /* inplace_power */
    0,                                          /* inplace_lshift */
    0,                                          /* inplace_rshift */
    0,                                          /* inplace_and */
    0,                                          /* inplace_xor */
    0,                                          /* inplace_or */

    (binaryfunc) pyjnumber_floordivide,         /* nb_floor_divide */
    (binaryfunc) pyjnumber_truedivide,          /* nb_true_divide */
    0,                                          /* nb_inplace_floor_divide */
    0,                                          /* nb_inplace_true_divide */
    (unaryfunc) pyjnumber_index,                /* nb_index */
};

/*
 * Inherits from PyJObject_Type
 */
PyTypeObject PyJNumber_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJNumber",
    sizeof(PyJNumberObject),
    0,
    0,                                        /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    &pyjnumber_number_methods,                /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
#if PY_MAJOR_VERSION < 3
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES,/* tp_flags */
#else
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
#endif
    "jnumber",                                /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    pyjnumber_richcompare,                    /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    pyjnumber_methods,                        /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0, // &PyJObject_Type                     /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    NULL,                                     /* tp_new */
};
