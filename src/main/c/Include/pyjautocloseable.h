/*
   jep - Java Embedded Python

   Copyright (c) 2017 JEP AUTHORS.

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

#ifndef _Included_pyjautocloseable
#define _Included_pyjautocloseable

extern PyTypeObject PyJAutoCloseable_Type;

typedef struct {
    PyJObject obj; /* magic inheritance */
} PyJAutoCloseableObject;


/*
 * Returns a new PyJAutoCloseable, which is a PyJObject that implements
 * close().
 */
PyJObject* PyJAutoCloseable_New(void);

/*
 * Returns true if the object is a PyJAutoCloseable.
 */
int PyJAutoCloseable_Check(PyObject*);

#endif // ndef pyjautocloseable
