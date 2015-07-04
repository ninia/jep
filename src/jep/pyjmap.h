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


#ifndef _Included_pyjmap
#define _Included_pyjmap

PyAPI_DATA(PyTypeObject) PyJmap_Type;

/*
 * A pyjmap is just a pyjobject with some extra methods attached to it to meet
 * the python Mapping protocol (interface).  It should only be used where
 * the underlying jobject of the pyjobject is an implementation of java.util.Map.
 */
typedef struct {
    PyJobject_Object obj; /* magic inheritance */
} PyJmap_Object;


PyJmap_Object* pyjmap_new(void);
int pyjmap_check(PyObject*);


#endif // ndef pyjmap
