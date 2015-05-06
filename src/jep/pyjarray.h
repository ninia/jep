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

#ifndef _Included_pyjarray
#define _Included_pyjarray

PyAPI_DATA(PyTypeObject) PyJarray_Type;

// c storage for our stuff, managed by python interpreter.
typedef struct {
    PyObject_HEAD
    jobjectArray     object;         /* array object */
    jclass           clazz;          /* useful for later calls */
    int              componentType;  /* type of array elements */
    jclass           componentClass; /* component type of object arrays, but not strings */
    int              length;         /* better than querying all the time */
    void            *pinnedArray;    /* i.e.: cast to (int *) for an int array */
    jboolean         isCopy;         /* true if pinned array was copied */
} PyJarray_Object;

PyObject* pyjarray_new(JNIEnv*, jobjectArray);
PyObject* pyjarray_new_v(PyObject*, PyObject*);
int pyjarray_check(PyObject*);
void pyjarray_release_pinned(PyJarray_Object*, jint);
void pyjarray_pin(PyJarray_Object*);

#endif // ndef pyjarray
