/*
   jep - Java Embedded Python

   Copyright (c) 2019 JEP AUTHORS.

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

static jobject BYTE_ORDER_NATIVE = NULL;
static jobject BYTE_ORDER_LITTLE = NULL;

struct bufferdescr {
    jobject *type;
    jobject (*order_func)(JNIEnv*, jobject);
    Py_ssize_t jitemsize;
    Py_ssize_t py_itemsize;
    char *native_format;
    char *little_format;
    char *big_format;
};

static const struct bufferdescr descriptors[] = {
    {&JBYTEBUFFER_TYPE, &java_nio_ByteBuffer_order, sizeof(jbyte), 1, "b", "b", "b"},
    {&JFLOATBUFFER_TYPE, &java_nio_FloatBuffer_order, sizeof(jfloat), sizeof(float), "f", "<f", ">f"},
    {&JDOUBLEBUFFER_TYPE, &java_nio_DoubleBuffer_order, sizeof(jdouble), sizeof(double), "d", "<d", ">d"},
    {&JINTBUFFER_TYPE, &java_nio_IntBuffer_order, sizeof(jint), sizeof(int), "i", "<i", ">i"},
    {&JLONGBUFFER_TYPE, &java_nio_LongBuffer_order, sizeof(jlong), sizeof(long long), "q", "<q", ">q"},
    {&JSHORTBUFFER_TYPE, &java_nio_ShortBuffer_order, sizeof(jshort), sizeof(short), "h", "<h", ">h"},
    {&JCHARBUFFER_TYPE, &java_nio_CharBuffer_order, sizeof(jchar), sizeof(char), "H", "<H", ">H"},
    {NULL, NULL, 0, 0, NULL, NULL, NULL},
};

static int initByteOrders(JNIEnv* env)
{
    if (BYTE_ORDER_NATIVE == NULL) {
        BYTE_ORDER_NATIVE = java_nio_ByteOrder_nativeOrder(env);
        if (process_java_exception(env)) {
            return -1;
        }
        BYTE_ORDER_NATIVE = (*env)->NewGlobalRef(env, BYTE_ORDER_NATIVE);
    }
    if (BYTE_ORDER_LITTLE == NULL) {
        jfieldID f = (*env)->GetStaticFieldID(env, JBYTEORDER_TYPE, "LITTLE_ENDIAN",
                                              "Ljava/nio/ByteOrder;");
        BYTE_ORDER_LITTLE = (*env)->GetStaticObjectField(env, JBYTEORDER_TYPE, f);
        if (process_java_exception(env)) {
            return -1;
        }
        BYTE_ORDER_LITTLE = (*env)->NewGlobalRef(env, BYTE_ORDER_LITTLE);
    }
    return 0;
}

static int
getbuf(PyObject* self, Py_buffer *view, int flags)
{
    PyJObject *pyjob     = (PyJObject*) self;
    JNIEnv    *env       = pyembed_get_env();
    jboolean   direct;
    jlong      capacity;
    const struct bufferdescr *descr;

    direct = java_nio_Buffer_isDirect(env, pyjob->object);
    if (process_java_exception(env)) {
        view->buf = NULL;
        return -1;
    } else if (!direct) {
        view->buf = NULL;
        PyErr_SetString(PyExc_TypeError,
                        "Python buffer access is only allowed for direct Java Buffers.");
        return -1;
    }

    view->buf = (*env)->GetDirectBufferAddress(env, pyjob->object);
    if (view->buf == NULL) {
        process_java_exception(env);
        return -1;
    }
    capacity = (*env)->GetDirectBufferCapacity(env, pyjob->object);

    for (descr = descriptors; descr->type != NULL; descr++) {
        if ((*env)->IsInstanceOf(env, pyjob->object, *(descr->type))) {
            break;
        }
    }
    if (descr->type == NULL) {
        view->buf = NULL;
        PyErr_Format(PyExc_TypeError, "Python buffer access is not allowed for %s",
                     PyString_AsString(pyjob->javaClassName));
        return -1;
    }
    view->obj = (PyObject*)self;
    Py_INCREF(self);
    view->len = descr->jitemsize * capacity;
    view->readonly = 0;
    view->ndim = 1;
    view->itemsize = descr->jitemsize;
    view->suboffsets = NULL;
    view->shape = NULL;
    if ((flags & PyBUF_ND) == PyBUF_ND) {
        /* Use internal so there is nothing to malloc/release */
        view->internal = (void*) (Py_ssize_t) capacity;
        view->shape = (Py_ssize_t*) & (view->internal);
    }
    view->strides = NULL;
    if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
        view->strides = &(view->itemsize);
    }
    view->format = NULL;
    if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
        jobject order;
        if (initByteOrders(env) != 0) {
            view->buf = 0;
            return -1;
        }
        order = descr->order_func(env, pyjob->object);
        if (process_java_exception(env)) {
            view->buf = NULL;
            return -1;
        }
        if ((*env)->IsSameObject(env, order, BYTE_ORDER_NATIVE)
                && descr->jitemsize == descr->py_itemsize) {
            view->format = descr->native_format;
        } else if ((*env)->IsSameObject(env, order, BYTE_ORDER_LITTLE)) {
            view->format = descr->little_format;
        } else {
            view->format = descr->big_format;
        }
    }
    return 0;
}


static PyBufferProcs buffer_as_buffer = {
#if PY_MAJOR_VERSION < 3
    (readbufferproc)NULL,
    (writebufferproc)NULL,
    (segcountproc)NULL,
    (charbufferproc)NULL,
#endif
    (getbufferproc)getbuf,
    (releasebufferproc)NULL
};



/*
 * Inherits from PyJObject_Type
 */
PyTypeObject PyJBuffer_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jep.PyJBuffer",
    sizeof(PyJObject),
    0,
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
    &buffer_as_buffer,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
#if PY_MAJOR_VERSION < 3
    Py_TPFLAGS_HAVE_NEWBUFFER |
#endif
    Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    "jbuffer",                                /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    0,                                        /* tp_methods */
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
