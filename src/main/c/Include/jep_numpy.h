/*
   jep - Java Embedded Python

   Copyright (c) 2015-2022 JEP AUTHORS.

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
 * Contains functions to support transformation between numpy ndarrays and
 * Java primitive arrays and jep.NDArrays.
 */

#include "jep_platform.h"

#ifndef _Included_jep_numpy
#define _Included_jep_numpy


#ifndef JEP_NUMPY_ENABLED
    #define JEP_NUMPY_ENABLED 1
#endif

/* this whole file is a no-op if numpy support is disabled */
#if JEP_NUMPY_ENABLED

    /* methods to support numpy -> java conversion */
    int npy_scalar_check(PyObject*);
    jobject convert_npy_scalar_jobject(JNIEnv*, PyObject*, jclass);
    int npy_array_check(PyObject*);
    jobject convert_pyndarray_jobject(JNIEnv*, PyObject*, jclass);

    int load_numpy_conversions(JNIEnv*);
#endif // if numpy is enabled


#endif // ifndef _Included_jep_numpy
