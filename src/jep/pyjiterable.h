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

/*
 * A PyJIterableObject is a PyJObject that has tp_iter implemented. It should
 * only be used where the underlying jobject of the PyJObject is an
 * implementation of java.util.Iterable.
 */

#include "jep_platform.h"
#include "pyjobject.h"

#ifndef _Included_pyjiterable
#define _Included_pyjiterable

PyAPI_DATA(PyTypeObject) PyJIterable_Type;

typedef struct {
    PyJObject obj; /* magic inheritance */
} PyJIterableObject;


PyJIterableObject* pyjiterable_new(void);
int pyjiterable_check(PyObject*);
PyObject* pyjiterable_getiter(PyObject*);


#endif // ndef pyjiterable
