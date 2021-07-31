/**
 * Copyright (c) 2006-2021 JEP AUTHORS.
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

import java.lang.reflect.Method;

import jep.Jep;
import jep.JepException;

/**
 * Handle Proxy method calls.
 */
public class InvocationHandler implements java.lang.reflect.InvocationHandler {

    private final PyObject pyObject;

    private final boolean functionalInterface;

    /**
     * Creates a new <code>InvocationHandler</code> instance.
     *
     * @param jep
     *            the jep interpreter
     * @param ltarget
     *            the python object's id
     * @param functionalInterface
     *            whether the target is a python callable that should be invoked
     *            directly
     * @throws JepException
     *             if an error occurs
     */
    public InvocationHandler(Jep jep, long ltarget,
            final boolean functionalInterface) throws JepException {
        this(new PyObject(jep, ltarget), functionalInterface);
    }

    /**
     * Creates a new <code>InvocationHandler</code> instance.
     *
     * @param pyObject
     *            the pyObject
     * @param functionalInterface
     *            whether the target is a python callable that should be invoked
     *            directly
     * @throws JepException
     *             if an error occurs
     */
    public InvocationHandler(final PyObject pyObject,
            final boolean functionalInterface) throws JepException {
        this.functionalInterface = functionalInterface;
        this.pyObject = pyObject;
    }

    /**
     * Processes a method invocation on a proxy instance and returns the result.
     * This method will be invoked on an invocation handler when a method is
     * invoked on a proxy instance that it is associated with.
     *
     * @param proxy
     *            the proxy instance that the method was invoked on
     * @param method
     *            the Method instance corresponding to the interface method
     *            invoked on the proxy instance. The declaring class of the
     *            Method object will be the interface that the method was
     *            declared in, which may be a superinterface of the proxy
     *            interface that the proxy class inherits the method through.
     * @param args
     *            an array of objects containing the values of the arguments
     *            passed in the method invocation on the proxy instance, or null
     *            if interface method takes no arguments. Arguments of primitive
     *            types are wrapped in instances of the appropriate primitive
     *            wrapper class, such as java.lang.Integer or java.lang.Boolean.
     * @return the value to return from the method invocation on the proxy
     *         instance. If the declared return type of the interface method is
     *         a primitive type, then the value returned by this method must be
     *         an instance of the corresponding primitive wrapper class;
     *         otherwise, it must be a type assignable to the declared return
     *         type. If the value returned by this method is null and the
     *         interface method's return type is primitive, then a
     *         NullPointerException will be thrown by the method invocation on
     *         the proxy instance. If the value returned by this method is
     *         otherwise not compatible with the interface method's declared
     *         return type as described above, a ClassCastException will be
     *         thrown by the method invocation on the proxy instance.
     * @throws Throwable
     *             the exception to throw from the method invocation on the
     *             proxy instance. The exception's type must be assignable
     *             either to any of the exception types declared in the throws
     *             clause of the interface method or to the unchecked exception
     *             types java.lang.RuntimeException or java.lang.Error. If a
     *             checked exception is thrown by this method that is not
     *             assignable to any of the exception types declared in the
     *             throws clause of the interface method, then an
     *             UndeclaredThrowableException containing the exception that
     *             was thrown by this method will be thrown by the method
     *             invocation on the proxy instance.
     */
    @Override
    public Object invoke(Object proxy, Method method, Object[] args)
            throws Throwable {
        pyObject.checkValid();

        return invoke(proxy, pyObject.pointer.tstate, pyObject.pointer.pyObject,
                method, args, this.functionalInterface);
    }

    public PyObject getPyObject() {
        return pyObject;
    }

    private static native Object invoke(Object proxy, long tstate, long target,
            Method method, Object[] args, boolean functionalInterface);
}
