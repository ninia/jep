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

#include "jep_platform.h"

#ifndef _Included_pyjobject
#define _Included_pyjobject


PyAPI_DATA(PyTypeObject) PyJObject_Type;

// c storage for our stuff, managed by python interpreter.
// doesn't need much, just a dictionary for attributes and
// a jobject reference.
typedef struct {
    PyObject_HEAD
    jobject          object;      /* the jni object */
    jclass           clazz;       /* java class object */
    PyObject        *attr;        /* list of tuples for get/set attr */
    PyObject        *methods;     /* list of method names */
    PyObject        *fields;      /* list of field names */
    int              finishAttr;  /* true if object attributes are finished */
    PyObject        *javaClassName; /* string of the fully-qualified name of
                                       the object's Java clazz */
} PyJObject;

PyObject* pyjobject_new(JNIEnv*, jobject);
PyObject* pyjobject_new_class(JNIEnv*, jclass);
int pyjobject_check(PyObject *obj);

// this method needs to be available to pyjclass
void pyjobject_addfield(PyJObject*, PyObject*);

// these methods need to be available to pyjlist
int pyjobject_setattr(PyJObject*, char*, PyObject*);
PyObject* pyjobject_getattr(PyJObject*, char*);
void pyjobject_dealloc(PyJObject*);
PyObject* pyjobject_str(PyJObject*);

#endif // ndef pyjobject
