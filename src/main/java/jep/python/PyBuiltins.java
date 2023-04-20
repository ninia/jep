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
package jep.python;

import jep.Interpreter;

/*
 * Interface for accessing some of the functions of the Python builtins module from Java.
 * More information on specific functions can be found at https://docs.python.org/3.5/library/functions.html
 *
 * To use this interface create an {@link Interpreter} and call {@link PyBuiltins#get(Interpreter)}.
 *
 * @since 4.2
 */
public interface PyBuiltins {

    public boolean callable(Object object);

    public PyObject compile(Object source, String filename, String mode);

    public PyObject compile(Object source, String filename, String mode, int flags);

    public PyObject compile(Object source, String filename, String mode, int flags, boolean dont_inherit);

    public PyObject compile(Object source, String filename, String mode, int flags, boolean dont_inherit, int optimize);

    public PyObject dict();

    public void delattr(Object object, String name);

    public String[] dir(Object object);

    public Object eval(Object object, PyObject globals);

    public Object eval(Object object, PyObject globals, PyObject locals);

    public void exec(Object object, PyObject globals);

    public void exec(Object object, PyObject globals, PyObject locals);

    public PyObject frozenset();

    public PyObject frozenset(Object iterable);

    public Object getattr(Object object, String name);

    public boolean hasattr(Object object, String name);

    public long id(PyObject object);

    public boolean isinstance(Object object, PyObject classinfo);

    public boolean issubclass(PyObject cls, PyObject classinfo);

    public PyObject list();

    public PyObject list(Object iterable);

    public PyObject object();

    public PyObject set();

    public PyObject set(Object iterable);

    public void setattr(Object object, String name, Object value);

    public PyObject tuple();

    public PyObject tuple(Object iterable);

    public PyCallable type(PyObject object);

    public static PyBuiltins get(Interpreter interpreter) {
        return interpreter.getValue("__import__('builtins')", PyObject.class).proxy(PyBuiltins.class);
    }
}
