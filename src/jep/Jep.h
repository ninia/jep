/*
   jep - Java Embedded Python

   Copyright (c) 2017 JEP AUTHORS.

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
 * Convenience header for including all of the headers of Jep.  Not to be
 * confused with jep.h which is generated at build time by javah.  This file
 * is inspired by the ease of use of including Python.h.
 */

/* jep_platform needs to be included first, see comments in jep_platform.h */
#include "jep_platform.h"
#include "jep_util.h"
#include "jep_exceptions.h"
#include "jep_numpy.h"

#include "pyembed.h"
#include "pyjarray.h"
#include "pyjclass.h"
#include "pyjcollection.h"
#include "pyjfield.h"
#include "pyjiterable.h"
#include "pyjiterator.h"
#include "pyjlist.h"
#include "pyjmap.h"
#include "pyjmethod.h"
#include "pyjconstructor.h"
#include "pyjmultimethod.h"
#include "pyjnumber.h"
#include "pyjautocloseable.h"
#include "pyjmonitor.h"
#include "pyjobject.h"
#include "jbox.h"
#include "convert_p2j.h"
#include "java_access/AutoCloseable.h"
#include "java_access/Boolean.h"
#include "java_access/Character.h"
#include "java_access/Class.h"
#include "java_access/Collection.h"
#include "java_access/Collections.h"
#include "java_access/Comparable.h"
#include "java_access/Constructor.h"
#include "java_access/Field.h"
#include "java_access/Iterable.h"
#include "java_access/Iterator.h"
#include "java_access/JepException.h"
#include "java_access/List.h"
#include "java_access/Map.h"
#include "java_access/Member.h"
#include "java_access/Method.h"
#include "java_access/Modifier.h"
#include "java_access/Number.h"
#include "java_access/Object.h"
#include "java_access/String.h"
#include "java_access/Throwable.h"
