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

#include "jep_platform.h"

#ifndef _Included_pyjobject
#define _Included_pyjobject


extern PyTypeObject PyJObject_Type;

// c storage for our stuff, managed by python interpreter.
// doesn't need much, just a dictionary for attributes and
// a jobject reference.
typedef struct {
    PyObject_HEAD
    jobject          object;      /* the jni object */
    jclass           clazz;       /* java class object */
    PyObject        *attr;        /* dict for get/set attr */
    PyObject        *javaClassName; /* string of the fully-qualified name of
                                       the object's Java clazz */
} PyJObject;

PyObject* PyJObject_New(JNIEnv*, jobject);
PyObject* PyJObject_NewClass(JNIEnv*, jclass);
int PyJObject_Check(PyObject*);

void pyjobject_dealloc(PyJObject*);

#endif // ndef pyjobject
