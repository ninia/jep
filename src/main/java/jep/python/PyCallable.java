/**
 * Copyright (c) 2017 JEP AUTHORS.
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

import jep.Jep;
import jep.JepException;

/**
 * A Java object that wraps a pointer to a Python callable.
 * 
 * These objects are frequently instance methods bound to Python's self object
 * where the self object is this Java PyObject. They can also be Python objects
 * that implement the method __call__.
 * 
 * @see "https://docs.python.org/2/reference/expressions.html#calls"
 * @see "https://docs.python.org/3/reference/expressions.html#calls"
 * 
 *      Example: <code>
 *     Jep jep = new Jep();
 *     jep.eval("class example(object):\n" +
 *              "    def __init__(self):\n" +
 *              "        pass\n" +
 *              "    def helloWorld(self):\n" +
 *              "        return 'Hello World'\n");
 *     jep.eval("instance = example()");
 *     PyObject pyobj = jep.getValue("instance", PyObject.class);
 *     PyCallable pyHelloWorld = PyObject.getAttr("helloWorld", PyCallable.class);
 *     String result = (String) pyHelloWorld.call();
 *     System.out.println(result);
 * </code>
 * 
 * @author Nate Jensen
 * @since 3.8
 */
public class PyCallable extends PyObject {

    public PyCallable(long tstate, long pyObject, Jep jep) throws JepException {
        super(tstate, pyObject, jep);
    }

    /**
     * Invokes this callable with the args in order.
     * 
     * @param args
     *            args to pass to the function in order
     * @return an <Object> value
     * @throws JepException
     */
    public Object call(Object... args) throws JepException {
        isValid();
        return call(pointer.tstate, pointer.pyObject, args, null);
    }

    /**
     * Invokes this callable with keyword args.
     * 
     * @param kwargs
     *            a Map of keyword args
     * @return an <Object> value
     * @throws JepException
     */
    public Object call(Map<String, Object> kwargs) throws JepException {
        isValid();
        return call(pointer.tstate, pointer.pyObject, null, kwargs);
    }

    /**
     * Invokes this callable with positional args and keyword args.
     * 
     * @param args
     *            args to pass to the function in order
     * @param kwargs
     *            a Map of keyword args
     * @return an <Object> value
     * @throws JepException
     */
    public Object call(Object[] args, Map<String, Object> kwargs)
            throws JepException {
        isValid();
        return call(pointer.tstate, pointer.pyObject, args, kwargs);
    }

    private native Object call(long tstate, long pyObject, Object[] args,
            Map<String, Object> kwargs) throws JepException;

}
