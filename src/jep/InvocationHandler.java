/**
 * Copyright (c) 2016 JEP AUTHORS.
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

import java.lang.reflect.Method;

import jep.python.PyObject;

/**
 * Handle Proxy method calls.
 * 
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 */
public class InvocationHandler implements java.lang.reflect.InvocationHandler {

    private long tstate;

    private long target;

    private Jep jep;

    /**
     * Creates a new <code>InvocationHandler</code> instance.
     * 
     * @param tstate
     *            the thread state id
     * @param ltarget
     *            the python object's id
     * @param jep
     *            the jep interpreter
     * @exception JepException
     *                if an error occurs
     */
    public InvocationHandler(long tstate, long ltarget, Jep jep)
            throws JepException {
        this.tstate = tstate;
        this.target = ltarget;
        this.jep = jep;

        // track target with jep.

        // this ensures that our target doesn't get garbage collected,
        // and since we can't have a close(), it'll get cleaned up.

        // correction. object is now increfed before being returned to
        // Java since in some cases the garbage collection could run
        // before this.
        jep.trackObject(new PyObject(this.tstate, this.target, this.jep), false);
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
     * @exception Throwable
     *                the exception to throw from the method invocation on the
     *                proxy instance. The exception's type must be assignable
     *                either to any of the exception types declared in the
     *                throws clause of the interface method or to the unchecked
     *                exception types java.lang.RuntimeException or
     *                java.lang.Error. If a checked exception is thrown by this
     *                method that is not assignable to any of the exception
     *                types declared in the throws clause of the interface
     *                method, then an UndeclaredThrowableException containing
     *                the exception that was thrown by this method will be
     *                thrown by the method invocation on the proxy instance.
     */
    @Override
    public Object invoke(Object proxy, Method method, Object[] args)
            throws Throwable {
        this.jep.isValidThread();

        // java passes null args sometimes. *shrugs*
        if (args == null)
            args = new Object[0];

        // find typeid for args. it's easier in Java and I'm a lazy
        // bastard.
        int types[] = new int[args.length];
        for (int i = 0; i < args.length; i++)
            types[i] = Util.getTypeId(args[i]);

        return invoke(method.getName(), this.tstate, this.target, args, types,
                Util.getTypeId(method.getReturnType()));
    }

    private static native Object invoke(String name, long tstate, long target,
            Object[] args, int[] types, int returnType);
}
