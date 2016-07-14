/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/*
   jep - Java Embedded Python

   Copyright (c) 2015 JEP_AUTHORS.

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
#include "pyjmethod.h"

#ifndef _Included_pyjmultimethod
#define _Included_pyjmultimethod


PyAPI_DATA(PyTypeObject) PyJMultiMethod_Type;

typedef struct {
    PyObject_HEAD
    PyObject* methodList;
} PyJMultiMethodObject;

/*
 * Both args must be PyJMethodObjects. A minimum of 2 methods is required to
 * make a PyJMultiMethod, after creation it is possible to add more than two
 * methods using PyJMultiMethod_Append.
 */
PyAPI_FUNC(PyObject*) PyJMultiMethod_New(PyObject*, PyObject*);
/* Args must be a PyJMultiMethodObject and a PyJMethodObject */
PyAPI_FUNC(int) PyJMultiMethod_Append(PyObject*, PyObject*);
/* Check if the arg is a PyJMultiMethodObject */
PyAPI_FUNC(int) PyJMultiMethod_Check(PyObject*);
/* Get the name of a PyJMultiMethodObject, returns a new reference to the name */
PyAPI_FUNC(PyObject*) PyJMultiMethod_GetName(PyObject*);

#endif // ndef pyjmultimethod
