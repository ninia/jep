/* 
   jep - Java Embedded Python

   Copyright (C) 2004 Mike Johnson

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
*/ 	


// shut up the compiler
#ifdef _POSIX_C_SOURCE
#  undef _POSIX_C_SOURCE
#endif
#include <jni.h>
#include <Python.h>

#ifndef _Included_pyjarray
#define _Included_pyjarray

// c storage for our stuff, managed by python interpreter.
typedef struct {
    PyObject_HEAD
    jobjectArray     object;         /* array object */
    jclass           clazz;          /* useful for later calls */
    JNIEnv          *env;            /* keep pointer to environment */
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
