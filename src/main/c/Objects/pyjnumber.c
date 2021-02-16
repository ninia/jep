/*
   jep - Java Embedded Python

   Copyright (c) 2016-2019 JEP AUTHORS.

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


static PyObject* java_number_to_pythonintlong(JNIEnv *env, PyObject* n)
{
    jlong      value;
    PyJObject *jnumber  = (PyJObject*) n;

    value = java_lang_Number_longValue(env, jnumber->object);
    if (process_java_exception(env)) {
        return NULL;
    }
    return PyLong_FromLongLong(value);
}


static PyObject* java_number_to_pythonfloat(JNIEnv *env, PyObject* n)
{
    jdouble    value;
    PyJObject *jnumber  = (PyJObject*) n;

    value = java_lang_Number_doubleValue(env, jnumber->object);
    if (process_java_exception(env)) {
        return NULL;
    }
    return PyFloat_FromDouble(value);
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

#define TO_PYTHON_NUMBER(env, var)\
    if (PyJNumber_Check(var)) {\
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

    if (PyJNumber_Check(x)) {
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

static Py_hash_t pyjnumber_hash(PyObject *self)
{
    JNIEnv   *env    = pyembed_get_env();
    Py_hash_t result = -1;

    if (PyJNumber_Check(self)) {
        self = java_number_to_python(env, self);
        if (self == NULL) {
            return result;
        }
    }

    result = PyObject_Hash(self);
    Py_DECREF(self);
    return result;
}

/*
 * Inherits from PyJObject_Type
 */
PyTypeObject *PyJNumber_Type = NULL;
int jep_jnumber_type_ready() {
    PyType_Spec spec = {
            .name = "java.lang.Number",
            .basicsize = sizeof(PyJNumber),
            .flags = Py_TPFLAGS_DEFAULT,
            .slots = &[
                {Py_tp_hash, (void*) pyjnumber_hash},
                {Py_tp_richcompare, (void*) pyjnumber_richcompare},
                {Py_tp_doc, "Jep java.lang.Number"},
                // number slots
                {Py_nb_index, (void*) pyjnumber_index},
                {Py_nb_truedivide, (void*) pyjnumber_truedivide},
                {Py_nb_floordivide, (void*) pyjnumber_floordivide},
                {Py_nb_float, (void*) pyjnumber_float},
                {Py_nb_int, (void*) pyjnumber_int},
                {Py_nb_nonzero, (void*) pyjnumber_nonzero},
                {Py_nb_abs, (void*) pyjnumber_absolute},
                {Py_nb_pos, (void*) pyjnumber_positive},
                {Py_nb_neg, (void*) pyjnumer_negative},
                {Py_nb_power, (void*) pyjnumber_power},
                {Py_nb_divmode, (void*) pyjnumber_divmod},
                {Py_nb_remainder, (void*) pyjnumber_remainder},
                {Py_nb_multiply, (void*) pyjnumber_multiply},
                {Py_nb_add, (void*) pyjnumber_add},
                {Py_nb_subtract, (void*) pyjnumber_subtract},
                {Py_nb_multiply, (void*) pyjnumber_multiply},
                {0, NULL}
            ]
    };
    PyJNumber_Type = PyType_FromSpecWithBases(&spec, PyJObject_Type);
    return PyType_Ready(PyJNumber_Type);
}