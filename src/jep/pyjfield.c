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

   ---
   Fields really don't have to be represented as python objects,
   but it is nice to have garbage collection and to be able
   to cast to PyObject.
*/

#include "Jep.h"

static void pyjfield_dealloc(PyJFieldObject *self);

static jmethodID fieldGetName = 0;
static jmethodID fieldGetType = 0;
static jmethodID fieldGetMod  = 0;
static jmethodID modIsStatic  = 0;

PyJFieldObject* pyjfield_new(JNIEnv *env,
                             jobject rfield,
                             PyJObject *pyjobject)
{
    PyJFieldObject *pyf;
    jstring          jstr        = NULL;
    const char      *fieldName   = NULL;

    if (PyType_Ready(&PyJField_Type) < 0) {
        return NULL;
    }

    pyf              = PyObject_NEW(PyJFieldObject, &PyJField_Type);
    pyf->rfield      = (*env)->NewGlobalRef(env, rfield);
    pyf->pyjobject   = pyjobject;
    pyf->pyFieldName = NULL;
    pyf->fieldTypeId = -1;
    pyf->isStatic    = -1;
    pyf->init        = 0;

    // ------------------------------ get field name

    if (!JNI_METHOD(fieldGetName, env, JFIELD_TYPE, "getName",
                    "()Ljava/lang/String;")) {
        process_java_exception(env);
        goto EXIT_ERROR;
    }

    jstr = (jstring) (*env)->CallObjectMethod(env, rfield, fieldGetName);
    if (process_java_exception(env) || !jstr) {
        goto EXIT_ERROR;
    }

    fieldName        = (*env)->GetStringUTFChars(env, jstr, 0);
    pyf->pyFieldName = PyString_FromString(fieldName);

    (*env)->ReleaseStringUTFChars(env, jstr, fieldName);
    (*env)->DeleteLocalRef(env, jstr);


    return pyf;

EXIT_ERROR:
    if (pyf) {
        pyjfield_dealloc(pyf);
    }
    return NULL;
}


static int pyjfield_init(JNIEnv *env, PyJFieldObject *self)
{
    jint             modifier    = -1;
    jboolean         isStatic    = JNI_TRUE;

    if ((*env)->PushLocalFrame(env, JLOCAL_REFS) != 0) {
        process_java_exception(env);
        return 0;
    }

    // ------------------------------ get fieldid

    self->fieldId = (*env)->FromReflectedField(env,
                    self->rfield);


    // ------------------------------ get return type

    if (!JNI_METHOD(fieldGetType, env, JFIELD_TYPE, "getType",
                    "()Ljava/lang/Class;")) {
        process_java_exception(env);
        goto EXIT_ERROR;
    }

    self->fieldType = (*env)->CallObjectMethod(env, self->rfield, fieldGetType);
    if (process_java_exception(env) || !self->fieldType) {
        goto EXIT_ERROR;
    }

    self->fieldTypeId = get_jtype(env, self->fieldType);
    if (process_java_exception(env)) {
        goto EXIT_ERROR;
    }

    // ------------------------------ get isStatic

    // call getModifers()
    if (!JNI_METHOD(fieldGetMod, env, JFIELD_TYPE, "getModifiers", "()I")) {
        process_java_exception(env);
        goto EXIT_ERROR;
    }
    modifier = (*env)->CallIntMethod(env, self->rfield, fieldGetMod);
    if (process_java_exception(env)) {
        goto EXIT_ERROR;
    }

    if (modIsStatic == 0) {
        modIsStatic = (*env)->GetStaticMethodID(env,
                                                JMODIFIER_TYPE,
                                                "isStatic",
                                                "(I)Z");
        if (process_java_exception(env) || !modIsStatic) {
            goto EXIT_ERROR;
        }
    }

    isStatic = (*env)->CallStaticBooleanMethod(env,
               JMODIFIER_TYPE,
               modIsStatic,
               modifier);
    if (process_java_exception(env)) {
        goto EXIT_ERROR;
    }

    if (!self->pyjobject->object && !isStatic) {
        PyErr_SetString(PyExc_TypeError, "Field is not static.");
        goto EXIT_ERROR;
    }
    if (isStatic == JNI_TRUE) {
        self->isStatic = 1;
    } else {
        self->isStatic = 0;
    }
    self->fieldType = (*env)->NewGlobalRef(env, self->fieldType);

    (*env)->PopLocalFrame(env, NULL);
    self->init = 1;
    return 1;

EXIT_ERROR:
    (*env)->PopLocalFrame(env, NULL);

    if (!PyErr_Occurred()) {
        PyErr_SetString(PyExc_RuntimeError, "Unknown");
    }

    return 0;
}


static void pyjfield_dealloc(PyJFieldObject *self)
{
#if USE_DEALLOC
    JNIEnv *env  = pyembed_get_env();
    if (env) {
        if (self->rfield) {
            (*env)->DeleteGlobalRef(env, self->rfield);
        }
    }

    Py_CLEAR(self->pyFieldName);

    PyObject_Del(self);
#endif
}


int pyjfield_check(PyObject *obj)
{
    if (PyObject_TypeCheck(obj, &PyJField_Type)) {
        return 1;
    }
    return 0;
}


// get value from java object field.
// returns new reference.
PyObject* pyjfield_get(PyJFieldObject *self)
{
    PyObject *result = NULL;
    JNIEnv   *env;

    env = pyembed_get_env();

    if (!self) {
        PyErr_Format(PyExc_RuntimeError, "Invalid self object.");
        return NULL;
    }

    if (!self->init) {
        if (!pyjfield_init(env, self) || PyErr_Occurred()) {
            return NULL;
        }
    }

    switch (self->fieldTypeId) {

    case JSTRING_ID: {
        jstring     jstr;
        const char *str;

        if (self->isStatic)
            jstr = (jstring) (*env)->GetStaticObjectField(
                       env,
                       self->pyjobject->clazz,
                       self->fieldId);
        else
            jstr = (jstring) (*env)->GetObjectField(env,
                                                    self->pyjobject->object,
                                                    self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        if (jstr == NULL) {
            Py_RETURN_NONE;
        }

        str    = (*env)->GetStringUTFChars(env, jstr, 0);
        result = PyString_FromString(str);

        (*env)->ReleaseStringUTFChars(env, jstr, str);
        (*env)->DeleteLocalRef(env, jstr);
        break;
    }

    case JCLASS_ID: {
        jobject obj;

        if (self->isStatic)
            obj = (*env)->GetStaticObjectField(env,
                                               self->pyjobject->clazz,
                                               self->fieldId);
        else
            obj = (*env)->GetObjectField(env,
                                         self->pyjobject->object,
                                         self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        if (obj == NULL) {
            Py_RETURN_NONE;
        }

        result = pyjobject_new_class(env, obj);
        (*env)->DeleteLocalRef(env, obj);
        break;
    }

    case JOBJECT_ID: {
        jobject obj;

        if (self->isStatic)
            obj = (*env)->GetStaticObjectField(env,
                                               self->pyjobject->clazz,
                                               self->fieldId);
        else
            obj = (*env)->GetObjectField(env,
                                         self->pyjobject->object,
                                         self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        if (obj == NULL) {
            Py_RETURN_NONE;
        }

        result = convert_jobject_pyobject(env, obj);
        (*env)->DeleteLocalRef(env, obj);
        break;
    }

    case JINT_ID: {
        jint ret;

        if (self->isStatic)
            ret = (*env)->GetStaticIntField(env,
                                            self->pyjobject->clazz,
                                            self->fieldId);
        else
            ret = (*env)->GetIntField(env,
                                      self->pyjobject->object,
                                      self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        result = Py_BuildValue("i", ret);
        break;
    }

    case JBYTE_ID: {
        jbyte ret;

        if (self->isStatic)
            ret = (*env)->GetStaticByteField(env,
                                             self->pyjobject->clazz,
                                             self->fieldId);
        else
            ret = (*env)->GetByteField(env,
                                       self->pyjobject->object,
                                       self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        result = Py_BuildValue("i", ret);
        break;
    }

    case JCHAR_ID: {
        jchar ret;
        char  val[2];

        if (self->isStatic)
            ret = (*env)->GetStaticCharField(env,
                                             self->pyjobject->clazz,
                                             self->fieldId);
        else
            ret = (*env)->GetCharField(env,
                                       self->pyjobject->object,
                                       self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        val[0] = (char) ret;
        val[1] = '\0';
        result = PyString_FromString(val);
        break;
    }

    case JSHORT_ID: {
        jshort ret;

        if (self->isStatic)
            ret = (*env)->GetStaticShortField(env,
                                              self->pyjobject->clazz,
                                              self->fieldId);
        else
            ret = (*env)->GetShortField(env,
                                        self->pyjobject->object,
                                        self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        result = Py_BuildValue("i", (int) ret);
        break;
    }

    case JDOUBLE_ID: {
        jdouble ret;

        if (self->isStatic)
            ret = (*env)->GetStaticDoubleField(env,
                                               self->pyjobject->clazz,
                                               self->fieldId);
        else
            ret = (*env)->GetDoubleField(env,
                                         self->pyjobject->object,
                                         self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        result = PyFloat_FromDouble(ret);
        break;
    }

    case JFLOAT_ID: {
        jfloat ret;

        if (self->isStatic)
            ret = (*env)->GetStaticFloatField(env,
                                              self->pyjobject->clazz,
                                              self->fieldId);
        else
            ret = (*env)->GetFloatField(env,
                                        self->pyjobject->object,
                                        self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        result = PyFloat_FromDouble((double) ret);
        break;
    }

    case JLONG_ID: {
        jlong ret;

        if (self->isStatic)
            ret = (*env)->GetStaticLongField(env,
                                             self->pyjobject->clazz,
                                             self->fieldId);
        else
            ret = (*env)->GetLongField(env,
                                       self->pyjobject->object,
                                       self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        result = PyLong_FromLongLong((PY_LONG_LONG) ret);
        break;
    }


    case JBOOLEAN_ID: {
        jboolean ret;

        if (self->isStatic)
            ret = (*env)->GetStaticBooleanField(env,
                                                self->pyjobject->clazz,
                                                self->fieldId);
        else
            ret = (*env)->GetBooleanField(env,
                                          self->pyjobject->object,
                                          self->fieldId);

        if (process_java_exception(env)) {
            return NULL;
        }

        result = Py_BuildValue("i", ret);
        break;
    }

    default:
        PyErr_Format(PyExc_RuntimeError,
                     "Unknown field type %i.",
                     self->fieldTypeId);
        Py_RETURN_NONE;
    }

    // shouldn't happen
    if (result == NULL) {
        Py_RETURN_NONE;
    }

    return result;
}


int pyjfield_set(PyJFieldObject *self, PyObject *value)
{
    JNIEnv *env = pyembed_get_env();

    if (!self) {
        PyErr_Format(PyExc_RuntimeError, "Invalid self object.");
        return -1;
    }

    if (!self->init) {
        if (!pyjfield_init(env, self) || PyErr_Occurred()) {
            return -1;
        }
    }

    switch (self->fieldTypeId) {
    case JSTRING_ID:
    case JCLASS_ID:
    case JOBJECT_ID: {
        jobject obj = PyObject_As_jobject(env, value, self->fieldType);
        if (!obj && PyErr_Occurred()) {
            return -1;
        }
        if (self->isStatic) {
            (*env)->SetStaticObjectField(env, self->pyjobject->clazz,
                                         self->fieldId, obj);
        } else {
            (*env)->SetObjectField(env, self->pyjobject->object,
                                   self->fieldId, obj);
        }
        (*env)->DeleteLocalRef(env, obj);
        return 0;
    }
    case JINT_ID: {
        jint i = PyObject_As_jint(value);
        if (i == -1 && PyErr_Occurred()) {
            return -1;
        }
        if (self->isStatic) {
            (*env)->SetStaticIntField(env, self->pyjobject->clazz,
                                      self->fieldId, i);
        } else {
            (*env)->SetIntField(env, self->pyjobject->object,
                                self->fieldId, i);
        }
        return 0;
    }
    case JCHAR_ID: {
        jchar c = PyObject_As_jchar(value);
        if (c == 0 && PyErr_Occurred()) {
            return -1;
        }
        if (self->isStatic) {
            (*env)->SetStaticCharField(env, self->pyjobject->clazz,
                                       self->fieldId, c);
        } else {
            (*env)->SetCharField(env, self->pyjobject->object,
                                 self->fieldId, c);
        }
        return 0;
    }
    case JBYTE_ID: {
        jbyte b = PyObject_As_jbyte(value);
        if (b == -1 && PyErr_Occurred()) {
            return -1;
        }
        if (self->isStatic) {
            (*env)->SetStaticByteField(env, self->pyjobject->clazz,
                                       self->fieldId, b);
        } else {
            (*env)->SetByteField(env, self->pyjobject->object,
                                 self->fieldId, b);
        }
        return 0;
    }
    case JSHORT_ID: {
        jshort s = PyObject_As_jshort(value);
        if (s == -1 && PyErr_Occurred()) {
            return -1;
        }
        if (self->isStatic) {
            (*env)->SetStaticShortField(env, self->pyjobject->clazz,
                                        self->fieldId, s);
        } else {
            (*env)->SetShortField(env, self->pyjobject->object,
                                  self->fieldId, s);
        }
        return 0;
    }
    case JDOUBLE_ID: {
        jdouble d = PyObject_As_jdouble(value);
        if (d == -1.0 && PyErr_Occurred()) {
            return -1;
        }
        if (self->isStatic) {
            (*env)->SetStaticDoubleField(env, self->pyjobject->clazz,
                                         self->fieldId, d);
        } else {
            (*env)->SetDoubleField(env, self->pyjobject->object,
                                   self->fieldId, d);
        }
        return 0;
    }
    case JFLOAT_ID: {
        jfloat f = PyObject_As_jfloat(value);
        if (f == -1.0 && PyErr_Occurred()) {
            return -1;
        }
        if (self->isStatic) {
            (*env)->SetStaticFloatField(env, self->pyjobject->clazz,
                                        self->fieldId, f);
        } else {
            (*env)->SetFloatField(env, self->pyjobject->object,
                                  self->fieldId, f);
        }
        return 0;
    }
    case JLONG_ID: {
        jlong j = PyObject_As_jlong(value);
        if (j == -1 && PyErr_Occurred()) {
            return -1;
        }
        if (self->isStatic) {
            (*env)->SetStaticLongField(env, self->pyjobject->clazz,
                                       self->fieldId, j);
        } else {
            (*env)->SetLongField(env, self->pyjobject->object,
                                 self->fieldId, j);
        }
        return 0;
    }
    case JBOOLEAN_ID: {
        jboolean z = PyObject_As_jboolean(value);
        if (PyErr_Occurred()) {
            return -1;
        }
        if (self->isStatic) {
            (*env)->SetStaticBooleanField(env, self->pyjobject->clazz,
                                          self->fieldId, z);
        } else {
            (*env)->SetBooleanField(env, self->pyjobject->object,
                                    self->fieldId, z);
        }
        return 0;
    }
    }
    PyErr_Format(PyExc_RuntimeError, "Unknown field type %i.", self->fieldTypeId);
    return -1;
}


static PyMethodDef pyjfield_methods[] = {
    {NULL, NULL, 0, NULL}
};


PyTypeObject PyJField_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJField",
    sizeof(PyJFieldObject),
    0,
    (destructor) pyjfield_dealloc,            /* tp_dealloc */
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
    Py_TPFLAGS_DEFAULT,                       /* tp_flags */
    "jfield",                                 /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    pyjfield_methods,                         /* tp_methods */
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
