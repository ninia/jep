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
package jep;

import jep.python.InvocationHandler;
import jep.python.PyObject;

/**
 * Uses java.lang.reflect.Proxy to wrap Python objects.
 * 
 * @author Mike Johnson
 */
class Proxy {

    private static final long serialVersionUID = 1L;

    protected static Object newDirectProxyInstance(Jep jep, long ltarget,
            Class<?> targetInterface) {
        ClassLoader loader = jep.getClassLoader();
        InvocationHandler ih = null;
        try {
            ih = new InvocationHandler(jep, ltarget, true);
        } catch (JepException e) {
            throw new IllegalArgumentException(e);
        }
        Class<?> classes[] = { targetInterface };
        return java.lang.reflect.Proxy.newProxyInstance(loader, classes, ih);
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
     * @param jep
     *            a <code>Jep</code> value
     * @param ltarget
     *            a <code>long</code> value
     * @param interfaces
     *            the list of interfaces to implement
     * @return an <code>Object</code> value
     * @throws IllegalArgumentException
     *             if an error occurs
     */
    protected static Object newProxyInstance(Jep jep, long ltarget,
            String[] interfaces) {
        ClassLoader loader = jep.getClassLoader();
        InvocationHandler ih = null;
        try {
            ih = new InvocationHandler(jep, ltarget, false);
        } catch (JepException e) {
            throw new IllegalArgumentException(e);
        }

        Class<?> classes[] = new Class<?>[interfaces.length];
        try {
            for (int i = 0; i < interfaces.length; i++)
                classes[i] = loader.loadClass(interfaces[i]);
        } catch (ClassNotFoundException e) {
            throw new IllegalArgumentException(e);
        }
        return java.lang.reflect.Proxy.newProxyInstance(loader, classes, ih);
    }

    /**
     * If the object passed in is a proxy for a PyObject, then return the
     * wrapped PyObject otherwise return null.
     *
     * @param proxy
     *            the Object that may be a proxy
     * @return the wrapped PyObject or null if there isn't one
     */
    protected static PyObject getPyObject(Object proxy) {
        if (java.lang.reflect.Proxy.isProxyClass(proxy.getClass())) {
            java.lang.reflect.InvocationHandler ih = java.lang.reflect.Proxy
                    .getInvocationHandler(proxy);
            if (ih instanceof InvocationHandler) {
                return ((InvocationHandler) ih).getPyObject();
            }
        }
        return null;
    }
}
