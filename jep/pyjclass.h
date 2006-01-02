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

#ifndef _Included_pyjclass
#define _Included_pyjclass

typedef struct {
    PyObject_HEAD
    jobjectArray      initArray;    /* constructor array */
    int               initLen;      /* length of initArray */
    PyObject         *pyjobject;    /* pointer to parent */
} PyJclass_Object;

PyJclass_Object* pyjclass_new(JNIEnv*, PyObject*);
PyObject* pyjclass_call(PyJclass_Object*, PyObject*, PyObject*);

#endif // ndef pyjclass
