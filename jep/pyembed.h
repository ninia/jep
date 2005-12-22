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

#ifndef _Included_pyembed
#define _Included_pyembed

// shut up the compiler
#ifdef _POSIX_C_SOURCE
# undef _POSIX_C_SOURCE
#endif
#include <jni.h>

// shut up the compiler
#ifdef _POSIX_C_SOURCE
# undef _POSIX_C_SOURCE
#endif
#ifdef _FILE_OFFSET_BITS
# undef _FILE_OFFSET_BITS
#endif
#include "Python.h"

#include "util.h"

#define DICT_KEY "jep"

struct __JepThread {
    PyObject      *modjep;
    PyThreadState *tstate;
    JNIEnv        *env;
    jobject        classloader;
};
typedef struct __JepThread JepThread;


void pyembed_startup(void);
void pyembed_shutdown(void);

intptr_t pyembed_thread_init(JNIEnv*, jobject);
void pyembed_thread_close(intptr_t);

void pyembed_close(void);
void pyembed_run(JNIEnv*, intptr_t, char*);
void pyembed_eval(JNIEnv *, intptr_t, char*);
int pyembed_compile_string(JNIEnv*, intptr_t, char*);
void pyembed_setloader(JNIEnv*, intptr_t, jobject);
jobject pyembed_getvalue(JNIEnv*, intptr_t, char*);

JepThread* pyembed_get_jepthread(void);

// -------------------------------------------------- set() methods

void pyembed_setparameter_object(JNIEnv*, intptr_t, const char*, jobject);
void pyembed_setparameter_string(JNIEnv*, intptr_t, const char*, const char*);
void pyembed_setparameter_int(JNIEnv*, intptr_t, const char*, int);
void pyembed_setparameter_long(JNIEnv*, intptr_t, const char*, jeplong);
void pyembed_setparameter_double(JNIEnv*, intptr_t, const char*, double);
void pyembed_setparameter_float(JNIEnv*, intptr_t, const char*, float);

#endif
