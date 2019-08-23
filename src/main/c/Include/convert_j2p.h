/*
   jep - Java Embedded Python

   Copyright (c) 2017-2019 JEP AUTHORS.

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
 * Contains functions for converting java objects/primitives to python objects.
 *
 * All the functions return a PyObject* that will be NULL if there are errors,
 * Errors are raised as python errors.
 */

#include "jep_platform.h"

#ifndef _Included_convert_j2p
#define _Included_convert_j2p

#define jboolean_As_PyObject PyBool_FromLong

#if PY_MAJOR_VERSION >= 3
    #define jbyte_As_PyObject PyLong_FromLong
    #define jshort_As_PyObject PyLong_FromLong
    #define jint_As_PyObject PyLong_FromLong
#else
    #define jbyte_As_PyObject PyInt_FromLong
    #define jshort_As_PyObject PyInt_FromLong
    #define jint_As_PyObject PyInt_FromLong
#endif

#define jlong_As_PyObject PyLong_FromLongLong

#define jfloat_As_PyObject PyFloat_FromDouble
#define jdouble_As_PyObject PyFloat_FromDouble

PyObject* jchar_As_PyObject(jchar);

PyObject* jobject_As_PyObject(JNIEnv*, jobject);

/*
 * This will return only objects that are PyJObject or a subtype, things like
 * strings and numbers will not be converted to the equivalent python type.
 * This behavior is only desirable from constructors.
 */
PyObject* jobject_As_PyJObject(JNIEnv*, jobject, jclass);

PyObject* jstring_As_PyString(JNIEnv*, jstring);
/*
 * Equivalent to java_lang_Object_toString() and passing the result to
 * jstring_As_PyString
 */
PyObject* jobject_As_PyString(JNIEnv*, jobject);


#endif // ifndef _Included_convert_j2p
