/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
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

#ifndef _Included_pyjobject
#define _Included_pyjobject

#include "pyjclass.h"

// c storage for our stuff, managed by python interpreter.
// doesn't need much, just a dictionary for attributes and
// a jobject reference.
typedef struct {
    PyObject_HEAD
    JNIEnv          *env;         /* keep pointer to environment */
    jobject          object;      /* the jni object */
    PyJclass_Object *pyjclass;    /* pointer to pyjclass if this
                                     object represents a type */
    jclass           clazz;       /* java class object */
    PyObject        *attr;        /* list of tuples for get/set attr */
    PyObject        *methods;     /* list of method names */
    PyObject        *fields;      /* list of field names */
    int              finishAttr;  /* true if object attributes are finished */
} PyJobject_Object;

PyObject* pyjobject_new(JNIEnv*, jobject);
PyObject* pyjobject_new_class(JNIEnv*, jclass);
PyObject* pyjobject_find_method(PyJobject_Object*, PyObject*, PyObject*);
int pyjobject_check(PyObject *obj);

#endif // ndef pyjobject
