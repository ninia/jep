/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 c-style: "K&R" -*- */
/*
   jep - Java Embedded Python

   Copyright (c) JEP AUTHORS.

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

/*
 * Contains functions for converting python objects to java objects/primitives.
 *
 * Any errors are thrown as Python Errors, most methods also return a special
 * value to indicate it is necessary to check for errors.
 */

#include "jep_platform.h"

#ifndef _Included_convert_p2j
#define _Included_convert_p2j

PyAPI_FUNC(jboolean) PyObject_As_jboolean(PyObject*);

/*
 * These four methods use the PyNumber_Index protocol from PEP 357 to convert
 * the input argument into a python int or long and then perform range checking
 * and conversion to the appropriate java type.
 *
 * The parameter should be a PyLong or another numeric type that can be
 * converted to a long, other types will result in an error.
 *
 * -1 will be returned if an error occurs.
 */
PyAPI_FUNC(jbyte)    PyObject_As_jbyte(PyObject*);
PyAPI_FUNC(jshort)   PyObject_As_jshort(PyObject*);
PyAPI_FUNC(jint)     PyObject_As_jint(PyObject*);
PyAPI_FUNC(jlong)    PyObject_As_jlong(PyObject*);

/*
 * These two methods have behavior matching PyFloat_AsDouble.
 *
 * The parameter should be a PyFloat or another numeric type that can be cast to
 * a float, other types will result in an error.
 *
 * -1.0 will be returned if an error occurs.
 */
PyAPI_FUNC(jfloat)   PyObject_As_jfloat(PyObject*);
PyAPI_FUNC(jdouble)  PyObject_As_jdouble(PyObject*);

/*
 * This works with PyUnicode in python 3 and PyString in python 2.
 *
 * 0 will be returned if an error occurs.
 */
PyAPI_FUNC(jchar)    PyObject_As_jchar(PyObject*);

/*
 * Very similar to PyObject_Str but also converts result to a jstring.
 *
 * NULL will be returned if an error occurs
 */
PyAPI_FUNC(jstring)  PyObject_As_jstring(JNIEnv*, PyObject*);

/*
 * NULL will be returned if an error occurs, but may also be returned if
 * PyObject* is None so use PyErr_Occurred() to check for errors if NULL
 * is returned.
 */
PyAPI_FUNC(jobject)  PyObject_As_jobject(JNIEnv*, PyObject*, jclass);

/*
 * Use PyErr_Occurred() after this to check for errors
 */
PyAPI_FUNC(jvalue)   PyObject_As_jvalue(JNIEnv*, PyObject*, jclass);

#endif // ifndef _Included_convert_py2j
