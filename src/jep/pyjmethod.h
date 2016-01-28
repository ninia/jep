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

#include "jep_platform.h"
#include "pyjobject.h"

#ifndef _Included_pyjmethod
#define _Included_pyjmethod


PyAPI_DATA(PyTypeObject) PyJmethod_Type;

// i needed an object to store methods in. this is a callable
// object and instances of these are dynamically added to a PyJobject
// using setattr.

typedef struct {
    PyObject_HEAD
    jmethodID         methodId;            /* resolved methodid */
    jobject           rmethod;             /* reflect/Method object */
    int               returnTypeId;        /* type id of return */
    PyObject         *pyMethodName;        /* python name... :-) */
    jobjectArray      parameters;          /* array of jclass parameter types */
    int               lenParameters;       /* length of parameters above */
    int               isStatic;            /* if method is static */
} PyJMethodObject;

PyJMethodObject* pyjmethod_new(JNIEnv*,
                               jobject,
                               PyJObject*);
PyJMethodObject* pyjmethod_new_static(JNIEnv*, jobject, PyJObject*);
int pyjmethod_init(JNIEnv*, PyJMethodObject*);

int pyjmethod_check(PyObject *obj);

/*
 * Test if a method has a specified name and number of parameters.
 */
int pyjmethod_check_simple_compat(PyJMethodObject*, JNIEnv*, PyObject*,
                                  Py_ssize_t);

/*
 * Check if a method is compatible with the types of a tuple of arguments. It is
 * not safe to call this method if pyjmethod_check_simple_compat does not return
 * true. This method does not need to be called before using call_internal, it
 * is only necessary for resolving method overloading.
 */
int pyjmethod_check_complex_compat(PyJMethodObject*, JNIEnv*, PyObject*);

#endif // ndef pyjmethod
