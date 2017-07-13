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
 * A PyJListObject is a PyJCollectionObject with some extra methods attached to meet
 * the Python Sequence protocol/interface.  It should only be used where the
 * underlying jobject of the PyJObject is an implementation of java.util.List.
 */

#include "jep_platform.h"
#include "pyjcollection.h"

#ifndef _Included_pyjlist
#define _Included_pyjlist

extern PyTypeObject PyJList_Type;

typedef struct {
    PyJCollectionObject obj; /* magic inheritance */
} PyJListObject;


/*
 * Returns a new PyJList, which is a PyJCollection with some sequence methods
 * attached to it.
 */
PyJObject* PyJList_New(void);

/*
 * Returns true if the object is a PyJList.
 */
int PyJList_Check(PyObject*);


#endif // ndef pyjlist
