/*
   jep - Java Embedded Python

   Copyright (c) 2015-2019 JEP AUTHORS.

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
 * A PyJIterableObject is a PyJObject that has tp_iter implemented to support iteration. It should
 * only be used where the underlying jobject of the PyJObject is an
 * implementation of java.lang.Iterable.
 */

#include "jep_platform.h"
#include "pyjobject.h"

#ifndef _Included_pyjiterable
#define _Included_pyjiterable

extern PyTypeObject PyJIterable_Type;

#define PyJIterable_Wrap(env, jobj, jcls) \
    PyJObject_New(env, &PyJIterable_Type, jobj, jcls)

#define PyJIterable_Check(pyobj) \
    PyObject_TypeCheck(pyobj, &PyJIterable_Type)

#endif // ndef pyjiterable
