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

/*
 * A PyJAutoCloseableObject is a PyJObject that has __enter__ and __exit__
 * implemented. It should only be used where the underlying jobject
 * of the PyJObject is an implementation of java.lang.AutoCloseable.
 */

#include "jep_platform.h"
#include "pyjobject.h"

#ifndef _Included_pyjbuffer
#define _Included_pyjbuffer

extern PyTypeObject *PyJBuffer_Type;
extern int jep_jbuffer_type_ready();

/**
 * *********************************
 * Wrappers for the buffer API
 * **********************************
 */

#ifdef Py_LIMITED_API
    /**
     * The limited ABI currently doesn't expose access to the "buffer interface"
     *
     * We work around this by defining the buffer API as using our wrapper types.
     * NOTE: We only support 'C contiguous' memory in our limited implementaiton.
     *
     * Supported types:
     * 1. bytes
     * 2. bytearray
     */
    struct jep_limited_buffer {
        PyObject *_target_object;
        Py_ssize_t len;
        char *buf;
        const char *format;
        Py_ssize_t itemsize;
    };
    typedef struct jep_limited_buffer JepBuffer;

    static inline int JepBuffer_Check(PyObject *target) {
        return PyBytes_Check(target) || PyByteArray_Check(target);
    }
    static inline int JepBuffer_FromObject(PyObject *target, JepBuffer *view) {
        if (PyByteArray_Check(target)) {
            char *memory = PyByteArray_AsString(target);
            Py_ssize_t  len = PyByteArray_Size(target);
            if (memory == NULL || len < 0) return -1;
            Py_INCREF(target);
            JepBuffer res = {
                    ._target_object = target,
                    .len = len,
                    .buf = memory,
                    .format = "b",
                    .itemsize = 1
            };
            *view = res;
            return 0;
        } else if (PyBytes_Check(target)) {
            PyErr_SetString(PyExc_TypeError, "Can't view immutable `bytes` as writable buffer");
            return -1;
        } else {
            PyErr_Format(
                    PyExc_TypeError,
                    "JEP is using the Python Stable ABI, so can't view type as a buffer: %S",
                    Py_TYPE(target)
            );
            return -1;;
        }
    }
    static inline int JepBuffer_FromObject_ReadOnly(PyObject *target, JepBuffer *view) {
        char *memory;
        Py_ssize_t len;
        if (PyByteArray_Check(target)) {
            memory = PyByteArray_AsString(target);
            len = PyByteArray_Size(target);
            if (memory == NULL || len < 0) return -1;
        } else if (PyBytes_Check(target)) {
            if (PyBytes_AsStringAndSize(target, &memory, &len) != 0) return -1;
        } else {
            PyErr_Format(
                    PyExc_TypeError,
                    "JEP is using the Python Stable ABI, so can't view type as a buffer: %S",
                    Py_TYPE(target)
            );
            return -1;;
        }
        Py_INCREF(target);
        JepBuffer res = {
                ._target_object = target,
                .len = len,
                .buf = memory,
                .format = "b",
                .itemsize = 1
        };
        *view = res;
        return 0;
    }
    static inline void JepBuffer_Release(JepBuffer *view) {
        Py_DECREF(view->_target_object);
    }
#else
    typedef Py_buffer JepBuffer
    #define JepBuffer_Check(target) Py_BufferCheck(target)
    static inline int JepBuffer_FromObject(PyObject *target, JepBuffer *view) {
        return PyObject_GetBuffer(target, (Py_buffer*) view, PyBUF_FULL);
    }
    static inline int JepBuffer_FromObject_ReadOnly(PyObject *target, JepBuffer *view) {
        return PyObject_GetBuffer(target, (Py_buffer*) view, PyBUF_FULL_RO);
    }
    static inline void JepBuffer_Release(JepBuffer *target) {
        PyBuffer_Release((Py_buffer*) target);
    }
#endif

#define PyJBuffer_Check(pyobj) \
    PyObject_TypeCheck(pyobj, PyJBuffer_Type)

#endif // ndef pyjbuffer
