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

#ifndef _Included_pyembed
#define _Included_pyembed

void pyembed_startup(void);
void pyembed_shutdown(void);

void pyembed_thread_init(JNIEnv *_env, const char*);
void pyembed_thread_close(const char*);

void pyembed_close(void);
void pyembed_run(JNIEnv*, const char*, char*, jobject);

PyThreadState* pyembed_mainthread_swap(void);
PyThreadState* pyembed_swap_thread(PyThreadState*);

int pyembed_modjep_has(PyObject*);
int pyembed_modjep_add(char*, PyObject*);
PyObject* pyembed_modjep_get(PyObject*);

// -------------------------------------------------- set() methods

void pyembed_setparameter_object(JNIEnv*, const char*, const char*, jobject);
void pyembed_setparameter_string(JNIEnv*, const char*, const char*, const char*);
void pyembed_setparameter_int(JNIEnv*, const char*, const char*, int);
void pyembed_setparameter_long(JNIEnv*, const char*, const char*, long long);
void pyembed_setparameter_double(JNIEnv*, const char*, const char*, double);
void pyembed_setparameter_float(JNIEnv*, const char*, const char*, float);

#endif
