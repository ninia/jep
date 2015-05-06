/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/* 
   jep - Java Embedded Python

   Copyright (c) 2015 JEP_AUTHORS.

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


#ifndef _Included_pyjmethodwrapper
#define _Included_pyjmethodwrapper

#include "pyjobject.h"
#include "pyjmethod.h"

PyAPI_DATA(PyTypeObject) PyJmethodWrapper_Type;

/*
 * PyJmethodWrapper_Object enables the ability to reuse a pyjmethod for
 * multiple instances of pyjobjects of the same underlying Java type.
 *
 * Pyjmethods are tied to java.lang.Methods, which are tied
 * to java.lang.Classes, which are shared across all instances of a particular
 * Class.  To ensure the right object is called with the method, the pyjmethod
 * wrapper includes both the pyjobject instance doing the calling and the
 * pyjmethod to be called.
 */
typedef struct {
    PyObject_HEAD
    PyJmethod_Object *method; /* the original pyjmethod tied to a java.lang.reflect.Method */
    PyJobject_Object *object; /* the pyjobject that called this method */
} PyJmethodWrapper_Object;

PyJmethodWrapper_Object* pyjmethodwrapper_new(PyJobject_Object*,
        PyJmethod_Object*);

#endif // ndef pyjmethodwrapper
