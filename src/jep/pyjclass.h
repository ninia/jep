/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/* 
   jep - Java Embedded Python

   Copyright (c) 2015 JEP AUTHORS.

   This file is licenced under the the zlib/libpng License.

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

// shut up the compiler
#ifdef _POSIX_C_SOURCE
#  undef _POSIX_C_SOURCE
#endif
#include <jni.h>
#include <Python.h>
#include "pyjobject.h"

#ifndef _Included_pyjclass
#define _Included_pyjclass

PyAPI_DATA(PyTypeObject) PyJclass_Type;
/*
 * a pyjclass is a pyjobject with a __call__ method attached, where
 * the call method will invoke constructors.
 */
typedef struct {
    PyJobject_Object  obj;            /* magic inheritance */
    jobjectArray      initArray;      /* constructor array */
    int               initLen;        /* length of initArray */
    int              *numArgsPerInit; /* pointer to init arg count */
} PyJclass_Object;

int pyjclass_init(JNIEnv*, PyObject*);
PyObject* pyjclass_call(PyJclass_Object*, PyObject*, PyObject*);
int pyjclass_check(PyObject*);

#endif // ndef pyjclass
