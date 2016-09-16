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


PyAPI_DATA(PyTypeObject) PyJMethod_Type;

/*
 * A callable python object which wraps a java method and is dynamically added
 * to a PyJObject using setattr. Most of the fields in this object are lazy
 * loaded and care should be taken to ensure they are populated before accessing
 * them. The only fields that are not lazily loaded are rmethod and pyMethodName.
 */
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

/* Create a new PyJMethod from a java.lang.reflect.Method*/
PyAPI_FUNC(PyJMethodObject*) PyJMethod_New(JNIEnv*, jobject);

/* Check if the arg is a PyJMethodObject */
PyAPI_FUNC(int) PyJMethod_Check(PyObject *obj);

/*
 * Get the number of parameters the method is expecting. If the method has not
 * been initialized yet this will trigger initialization.
 */
PyAPI_FUNC(int) PyJMethod_GetParameterCount(PyJMethodObject*, JNIEnv*);

/*
 * Check if a method is compatible with the types of a tuple of arguments.
 * This will return a 0 if the arguments are not valid for this method and a
 * positive integer if the arguments are valid. Larger numbers indicate a better
 * match between the arguments and the expected parameter types. This function
 * uses pyarg_matches_jtype to determine how well arguments match. This function
 * does not need to be called before using calling this method, it is only
 * necessary for resolving method overloading.
 */
PyAPI_FUNC(int) PyJMethod_CheckArguments(PyJMethodObject*, JNIEnv*, PyObject*);

#endif // ndef pyjmethod
