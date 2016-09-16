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

import java.lang.reflect.InvocationHandler;

/**
 * Extends java.lang.reflect.Proxy for callbacks.
 * 
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 */
public class Proxy extends java.lang.reflect.Proxy {

    private static final long serialVersionUID = 1L;

    /**
     * Constructs a new Proxy instance from a subclass (typically, a dynamic
     * proxy class) with the specified value for its invocation handler.
     * 
     * @param h
     *            an <code>InvocationHandler</code> value
     */
    protected Proxy(InvocationHandler h) {
        super(h);
    }

    /**
     * <pre>
     * Returns an instance of a proxy class for the specified
     * interfaces that dispatches method invocations to the specified
     * invocation handler. This method is equivalent to:
     * 
     * Proxy.getProxyClass(loader, interfaces).
     *     getConstructor(new Class[] { InvocationHandler.class }).
     *     newInstance(new Object[] { handler });
     * 
     * 
     * Proxy.newProxyInstance throws IllegalArgumentException for the
     * same reasons that Proxy.getProxyClass does.
     * </pre>
     * 
     * @param tstate
     *            a <code>long</code> value
     * @param ltarget
     *            a <code>long</code> value
     * @param jep
     *            a <code>Jep</code> value
     * @param loader
     *            the class loader to define the proxy class
     * @param interfaces
     *            the list of interfaces to implement
     * @return an <code>Object</code> value
     * @exception IllegalArgumentException
     *                if an error occurs
     */
    public static Object newProxyInstance(long tstate, long ltarget, Jep jep,
            ClassLoader loader, String[] interfaces)
            throws IllegalArgumentException {

        InvocationHandler ih = null;
        try {
            ih = new jep.InvocationHandler(tstate, ltarget, jep);
        } catch (JepException e) {
            throw new IllegalArgumentException(e);
        }

        Class classes[] = new Class[interfaces.length];
        try {
            for (int i = 0; i < interfaces.length; i++)
                classes[i] = loader.loadClass(interfaces[i]);
        } catch (ClassNotFoundException e) {
            throw new IllegalArgumentException(e);
        }

        return Proxy.newProxyInstance(loader, classes, ih);
    }
}
