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

jint pyembed_thread_init(JNIEnv*, jobject);
void pyembed_thread_close(jint);

void pyembed_close(void);
void pyembed_run(JNIEnv*, jint, char*);
void pyembed_eval(JNIEnv *, jint, char*);
int pyembed_compile_string(JNIEnv*, jint, char*);
void pyembed_setloader(JNIEnv*, jint, jobject);
jobject pyembed_getvalue(JNIEnv*, jint, char*);

JepThread* pyembed_get_jepthread(void);

// -------------------------------------------------- set() methods

void pyembed_setparameter_object(JNIEnv*, jint, const char*, jobject);
void pyembed_setparameter_string(JNIEnv*, jint, const char*, const char*);
void pyembed_setparameter_int(JNIEnv*, jint, const char*, int);
void pyembed_setparameter_long(JNIEnv*, jint, const char*, jeplong);
void pyembed_setparameter_double(JNIEnv*, jint, const char*, double);
void pyembed_setparameter_float(JNIEnv*, jint, const char*, float);

#endif
