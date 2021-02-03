/*
   jep - Java Embedded Python

   Copyright (c) 2015-2019 JEP AUTHORS.

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
 * Contains includes and macros to enable compilation across multiple
 * platforms with different compilers while supporting both Python 2 and 3.
 * This file aims to be for Jep what pyport.h is for CPython.
 */

/**
 * If enabled Compile against the limited API, for version 3.8
 *
 * NOTE: This is higher than the minimum python version we normally require.
 * However, the Limited API has been slowly expanded over time and we need to
 * make use of the latest functionality. Almost all the "new" features in the Stable API
 * are still present on old versions, only unstable.
 *
 * TODO: Make this configuration optional
 */
#define Py_LIMITED_API 0x03080000

// Python.h needs to be included first, see http://bugs.python.org/issue1045893
#include <Python.h>
#include <assert.h>

static_assert(PY_MAJOR_VERSION >= 3,"There is no Python 2 support!");
#ifdef WIN32
    #include "winconfig.h"
#endif

#if HAVE_CONFIG_H
    #include <config.h>
#endif

#if HAVE_UNISTD_H
    #include <sys/types.h>
    #include <unistd.h>
#endif


#include <jni.h>

#ifndef _Included_jep_platform
    #define _Included_jep_platform



    /**
     * **********HACK**********
     * PyUnicode_AsUTF8 isn't part of the stable ABI until Python 3.10.
     * See bpo-41784 and Github PR #22252: https://github.com/python/cpython/pull/22252
     *
     * Despite this fact, we need it for accessing the UTF8 representation of a string
     * without any unnecessary allocations.
     * To work around it, we define the extern function ref by hand.
     */
    #if defined(Py_LIMITED_API) && Py_LIMITED_API+0 < 0x030A0000
    extern const char *PyUnicode_AsUTF8AndSize(PyObject *unicode, Py_ssize_t *size);
    #endif
    #if Py_LIMITED_API
    /*
     * ########################################
     * API Emulation for Py_LIMITED_API.
     * Here, we emulate some missing APIS with inline functions.
     * These are JEP internal
     * ########################################
     */

    /**
     * On the limited API `PyUnicode_AsUTF8` is missing.
     * We define it as a thin wrapper over `PyUnicode_AsUTF8AndSize`
     */
    static inline const char *PyUnicode_AsUTF8(PyObject *unicode) {
        Py_ssize_t size;
        return PyUnicode_AsUTF8AndSize(unicode, &size);
    }

    /*
     * Emulate the 'fast sequence' API.
     *
     * The fast sequence API is based upon the assumption everything is a (well behaved)
     * list or tuple. Its not hard to emulate.....
     * Essentially `assume(PyList_Check(target) || PyTuple_Check(target))` before each call.
     */

    #undef PySequence_Fast_GET_SIZE
    #undef PySequence_Fast_GET_ITEM
    static inline Py_ssize_t PySequence_Fast_GET_SIZE(PyObject *target) {
        if (PyList_Check(target) || PyTuple_Check(target)) {
            return Py_SIZE(target); // see code
        } else {
            abort(); // Not a fast sequence: Undefined behavior
        }
    }
    static inline PyObject *PySequence_Fast_GET_ITEM(PyObject *target, Py_ssize_t index) {
        PyObject *res;
        if (index < 0) {
            abort(); // Negative index: Undefined behavior
        }
        if (PyList_Check(target)) {
            res = PyList_GetItem(target, index);
        } else if (PyTuple_Check(target)) {
            res = PyTuple_GetItem(target, index);
        } else {
            abort(); // Not a fast sequence: Undefined behavior
        }
        if (res == NULL) {
            /**
             * One of those stupid 'high-level' functions through an exception.
             * Normally this would be undefined behavior if you used a macro.
             */
            Py_FatalError("Failure: Likely invalid index");
            abort(); // unreachable
        } else {
            return res;
        }
    }
    #endif

    /* Windows compatibility */
    #ifdef WIN32
        #define FILE_SEP               '\\'
    #else
        #define FILE_SEP               '/'
    #endif // Windows compatibility

    /*
    * A default number of local references to reserve when using the
    * PushLocalFrame JNI method. Most native jep methods need a few local java
    * references that are deleted before the method returns. Rather than trying
    * to get an exact number of local references for every frame it is simpler
    * to overallocate. The JNI specification mandates that there are at least
    * 16 local references avaialble when enetering native code from java so
    * using the same value as a default for creating new local frames means
    * that native methods will have the same number of local references
    * available regardless of whether the frame was created by JNI or by a call
    * from PushLocalFrame.
    */
    #define JLOCAL_REFS 16

#endif // ifndef _Included_jep_platform
