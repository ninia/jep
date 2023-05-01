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

import java.util.Map;

import jep.Interpreter;

/**
 * Interface for accessing some of the functions of the Python builtins module from Java.
 * More information on specific functions can be found in the
 * <a href="https://docs.python.org/3/library/functions.html">Python Documentation</a>
 * <p>
 * To use this interface create an {@link Interpreter} and call {@link PyBuiltins#get(Interpreter)}.
 * <p>
 * PyBuiltins are not thread safe and have the same thread related restrictions as {@link PyObject}.
 * @since 4.2
 */
public interface PyBuiltins {

    public boolean callable(Object object);

    public void delattr(PyObject object, String name);

    public PyObject dict();

    public PyObject dict(Map<?,?> mapping);

    public String[] dir(PyObject object);

    public Object eval(String expression, PyObject globals);

    public Object eval(String expression, PyObject globals, PyObject locals);

    public void exec(String code, PyObject globals);

    public void exec(String code, PyObject globals, PyObject locals);

    public PyObject frozenset();

    public PyObject frozenset(Iterable<?> iterable);

    public PyObject frozenset(Object[] array);

    public Object getattr(PyObject object, String name);

    public boolean hasattr(PyObject object, String name);

    public long id(PyObject object);

    public boolean isinstance(PyObject object, PyObject classinfo);

    public boolean issubclass(PyObject cls, PyObject classinfo);

    public PyObject list();

    public PyObject list(Iterable<?> iterable);

    public PyObject list(Object[] array);

    public PyObject object();

    public PyObject set();

    public PyObject set(Iterable<?> iterable);

    public PyObject set(Object[] array);

    public void setattr(PyObject object, String name, Object value);

    public PyObject tuple();

    public PyObject tuple(Iterable<?> iterable);

    public PyObject tuple(Object[] array);

    public PyCallable type(PyObject object);

    public static PyBuiltins get(Interpreter interpreter) {
        return interpreter.getValue("__import__('builtins')", PyObject.class).proxy(PyBuiltins.class);
    }
}
