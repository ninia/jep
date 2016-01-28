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
 * A PyJMapObject is a PyJObject with some extra methods attached to meet
 * the python Mapping protocol/interface.  It should only be used where the
 * underlying jobject of the PyJObject is an implementation of java.util.Map.
 */

#include "jep_platform.h"
#include "pyjobject.h"

#ifndef _Included_pyjmap
#define _Included_pyjmap

PyAPI_DATA(PyTypeObject) PyJMap_Type;

typedef struct {
    PyJObject obj; /* magic inheritance */
} PyJMapObject;


PyJMapObject* pyjmap_new(void);
int pyjmap_check(PyObject*);


#endif // ndef pyjmap
