/**
 * Copyright (c) 2023 JEP AUTHORS.
 *
 * This file is licensed under the the zlib/libpng License.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 * 
 *     1. The origin of this software must not be misrepresented; you
 *     must not claim that you wrote the original software. If you use
 *     this software in a product, an acknowledgment in the product
 *     documentation would be appreciated but is not required.
 * 
 *     2. Altered source versions must be plainly marked as such, and
 *     must not be misrepresented as being the original software.
 * 
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 */
package jep;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Can be used with a method or constructor to customize the behavior of Jep
 * when the method is called from Python.
 *
 * @since 4.2
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.METHOD, ElementType.CONSTRUCTOR})
public @interface PyMethod {

    /**
     * Set to true to indicate that this Java method should accept varargs when
     * it is called from Python. This is not necessary for Objects which
     * use Java style varargs, Jep automatically maps Python varargs to java
     * varargs. The last argument(or second to last argument if kwargs is also
     * true) must be a type that is compatible with a Python tuple, such as a
     * List, array, or PyObject. If the argument is an array of a specific type
     * then jep will attempt to convert all varargs to that type and throw an
     * exception if conversion is not defined.
     */
    public boolean varargs() default false;

    /**
     * Set to true to indicate that this Java method should accept keyword
     * arguments when it is called from Python. The last argument must be a type
     * that is compatible with a Jep dict, such as a Map or PyObject.
     */
    public boolean kwargs() default false;
}
