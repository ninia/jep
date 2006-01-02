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

#ifndef _Included_pyjmethod
#define _Included_pyjmethod

#include "pyjobject.h"
#include "pyjclass.h"

// i needed an object to store methods in. this is a callable
// object and instances of these are dynamically added to a PyJobject
// using setattr.

typedef struct {
    PyObject_HEAD
    jmethodID         methodId;            /* resolved methodid */
    jobject           rmethod;             /* reflect/Method object */
    PyJobject_Object *pyjobject;           /* parent, should point to
                                              PyJObject_Object */
    int               returnTypeId;        /* type id of return */
    PyObject         *pyMethodName;        /* python name... :-) */
    jobjectArray      parameters;          /* array of jclass parameter types */
    int               lenParameters;       /* length of parameters above */
    int               isStatic;            /* if method is static */
} PyJmethod_Object;

PyJmethod_Object* pyjmethod_new(JNIEnv*,
                                jobject,
                                PyJobject_Object*);
PyJmethod_Object* pyjmethod_new_static(JNIEnv*, jobject, PyJobject_Object*);
int pyjmethod_init(JNIEnv*, PyJmethod_Object*);

PyObject* pyjmethod_call_internal(PyJmethod_Object*, PyObject*);
int pyjmethod_check(PyObject *obj);

#endif // ndef pyjmethod
