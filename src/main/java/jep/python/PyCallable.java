/**
 * Copyright (c) 2017-2021 JEP AUTHORS.
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
 * These objects can be instance methods, functions, lambdas, or any Python
 * object implementing the __call__ method.
 * <p>
 * Instance Method Example:
 *
 * <pre>
 * {@code
 *     jep.exec("class Example(object):\n" +
 *              "    def __init__(self):\n" +
 *              "        pass\n" +
 *              "    def helloWorld(self):\n" +
 *              "        return 'Hello World'\n");
 *     jep.exec("instance = Example()");
 *     PyObject pyobj = jep.getValue("instance", PyObject.class);
 *     PyCallable pyHelloWorld = PyObject.getAttr("helloWorld", PyCallable.class);
 *     String result = (String) pyHelloWorld.call();
 * }
 * </pre>
 * <p>
 * Function Example:
 *
 * <pre>
 * {@code
 *     jep.exec("def hello(arg):\n" +
 *              "    return 'Hello ' +  str(arg)");
 *     PyCallable pyHello = jep.getValue("hello", PyCallable.class);
 *     String result = (String) pyHello.call("World");
 * }
 * </pre>
 *
 * @see <a href=
 *      "https://docs.python.org/3/reference/expressions.html#calls">Python 3
 *      Call Expression</a>
 * @author Nate Jensen
 * @since 3.8
 */
public class PyCallable extends PyObject {

    protected PyCallable(Jep jep, long pyObject) throws JepException {
        super(jep, pyObject);
    }

    /**
     * Invokes this callable with the args in order.
     * 
     * @param args
     *            args to pass to the function in order
     * @return an {@link Object} value
     * @throws JepException
     *             if an error occurs
     */
    public Object call(Object... args) throws JepException {
        return callAs(Object.class, args);
    }

    /**
     * Invokes this callable with the args in order, converting the return value
     * to the given class.
     *
     * @param <T>
     *            the generic type of the return type
     * @param expectedType
     *            The expected return type of the invocation
     * @param args
     *            args to pass to the function in order
     * @return a value of the given type
     * @throws JepException
     *             if an error occurs
     */
    public <T> T callAs(Class<T> expectedType, Object... args)
            throws JepException {
        checkValid();
        return expectedType.cast(call(pointer.tstate, pointer.pyObject, args,
                null, expectedType));
    }

    /**
     * Invokes this callable with keyword args.
     * 
     * @param kwargs
     *            a Map of keyword args
     * @return an {@link Object} value
     * @throws JepException
     *             if an error occurs
     */
    public Object call(Map<String, Object> kwargs) throws JepException {
        return callAs(Object.class, kwargs);
    }

    /**
     * Invokes this callable with keyword args, converting the return value to
     * the given class.
     * 
     * @param <T>
     *            the generic type of the return type
     * @param expectedType
     *            The expected return type of the invocation
     * @param kwargs
     *            a Map of keyword args
     * @return a value of the given type
     * @throws JepException
     *             if an error occurs
     */
    public <T> T callAs(Class<T> expectedType, Map<String, Object> kwargs)
            throws JepException {
        checkValid();
        return expectedType.cast(call(pointer.tstate, pointer.pyObject, null,
                kwargs, expectedType));
    }

    /**
     * Invokes this callable with positional args and keyword args.
     * 
     * @param args
     *            args to pass to the function in order
     * @param kwargs
     *            a Map of keyword args
     * @return an {@link Object} value
     * @throws JepException
     *             if an error occurs
     */
    public Object call(Object[] args, Map<String, Object> kwargs)
            throws JepException {
        return callAs(Object.class, args, kwargs);
    }

    /**
     * Invokes this callable with positional args and keyword args, converting
     * the return value to the given class.
     * 
     * @param <T>
     *            the generic type of the return type
     * @param expectedType
     *            The expected return type of the invocation
     * @param args
     *            args to pass to the function in order
     * @param kwargs
     *            a Map of keyword args
     * @return a value of the given type
     * @throws JepException
     *             if an error occurs
     */
    public <T> T callAs(Class<T> expectedType, Object[] args,
            Map<String, Object> kwargs) throws JepException {
        checkValid();
        return expectedType.cast(call(pointer.tstate, pointer.pyObject, args,
                kwargs, expectedType));
    }

    private native Object call(long tstate, long pyObject, Object[] args,
            Map<String, Object> kwargs, Class<?> expectedType) throws JepException;

}
