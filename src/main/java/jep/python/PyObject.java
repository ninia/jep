/**
 * Copyright (c) 2006-2022 JEP AUTHORS.
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

import java.lang.reflect.Proxy;

import jep.Jep;
import jep.JepAccess;
import jep.JepException;

/**
 * A Java object that wraps a pointer to a Python object.
 * 
 * This class is not thread safe and PyObjects can only be used on the Thread
 * where they were created. When an Interpreter instance is closed all PyObjects
 * from that instance will be invalid and can no longer be used.
 */
public class PyObject extends JepAccess implements AutoCloseable {

    protected final PyPointer pointer;

    /**
     * Make a new PyObject
     * 
     * @param jep
     *            the instance of jep that created this object
     * @param pyObject
     *            the address of the python object
     * @throws JepException
     *             if an error occurs
     */
    protected PyObject(Jep jep, long pyObject) throws JepException {
        this.pointer = new PyPointer(this, getMemoryManager(jep), pyObject);
    }

    /**
     * Called from native code
     * 
     * @return the address of the native PyObject
     */
    protected long getPyObject() throws JepException {
        tstate();
        return pointer.pyObject;
    }

    /**
     * Get the valid jep thread state for this object.
     * 
     * @throws JepException
     *             if it is not safe to use this python object
     */
    protected long tstate() throws JepException {
        if (this.pointer.isDisposed()) {
            throw new JepException(
                    getClass().getSimpleName() + " has been closed.");
        }
        return pointer.memoryManager.getThreadState();
    }

    @Override
    public void close() throws JepException {
        this.pointer.dispose();
    }

    /**
     * Access an attribute of the wrapped Python Object, similar to the Python
     * built-in function getattr. This is equivalent to the Python statement
     * <code>this.attr_name</code>.
     * 
     * @param attr_name
     *            the attribute name
     * @return a Java version of the attribute
     * @throws JepException
     *             if an error occurs
     * @since 3.8
     */
    public Object getAttr(String attr_name) throws JepException {
        return getAttr(tstate(), pointer.pyObject, attr_name, Object.class);
    }

    /**
     * Access an attribute of the wrapped Python Object, similar to the Python
     * built-in function getattr. This method allows you to specify the return
     * type, the supported types are the same as
     * {@link Jep#getValue(String, Class)}.
     * 
     * @param <T>
     *            the generic type of the return type
     * @param attr_name
     *            the attribute name
     * @param clazz
     *            the Java class of the return type.
     * @return a Java version of the attribute
     * @throws JepException
     *             if an error occurs
     * @since 3.8
     */
    public <T> T getAttr(String attr_name, Class<T> clazz) throws JepException {
        return clazz.cast(
                getAttr(tstate(), pointer.pyObject, attr_name, clazz));
    }

    private native Object getAttr(long tstate, long pyObject, String attr_name,
            Class<?> clazz) throws JepException;

    /**
     * Sets an attribute on the wrapped Python object, similar to the Python
     * built-in function setattr. This is equivalent to the Python statement
     * <code>this.attr_name = o</code>.
     * 
     * @param attr_name
     *            the attribute name
     * @param o
     *            the object to set as an attribute
     * @throws JepException
     *             if an error occurs
     * 
     * @since 3.8
     */
    public void setAttr(String attr_name, Object o) throws JepException {
        setAttr(tstate(), pointer.pyObject, attr_name, o);
    }

    private native void setAttr(long tstate, long pyObject, String attr_name,
            Object o) throws JepException;

    /**
     * Deletes an attribute on the wrapped Python object, similar to the Python
     * built-in function delattr. This is equivalent to the Python statement
     * <code>del this.attr_name</code>.
     * 
     * @param attr_name
     *            the name of the attribute to be deleted
     * @throws JepException
     *             if an error occurs
     *
     * @since 3.8
     */
    public void delAttr(String attr_name) throws JepException {
        delAttr(tstate(), pointer.pyObject, attr_name);
    }

    private native void delAttr(long tstate, long pyObject, String attr_name)
            throws JepException;

    /**
     * Checks that the Java type matches and if so then uses Python's rich
     * compare with the == operator to check if this wrapped Python object
     * matches the other PyObject.
     * 
     * Equals is not consistent between languages. Java is strict on equality
     * while Python is flexible. For example in Python code: Integer.valueOf(5)
     * == 5 will evaluate to True while in Java code:
     * Integer.valueOf(5).equals(other) where other is a PyObject wrapping 5
     * will evaluate to false.
     */
    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        return equals(tstate(), pointer.pyObject, obj);
    }

    /**
     * Produces the string representation of the wrapped Python object by using
     * the Python built-in method str.
     */
    @Override
    public String toString() {
        return toString(tstate(), pointer.pyObject);
    }

    /**
     * Produces the hash code of the wrapped Python object by using the Python
     * built-in method hash. Hash codes are not consistent between languages.
     * For example the hash code of the string "hello" will be different in
     * Python than in Java, even if this PyObject wrapped the string "hello".
     */
    @Override
    public int hashCode() {
        Long value = hashCode(tstate(), pointer.pyObject);
        return value.hashCode();
    }

    private native boolean equals(long tstate, long pyObject, Object obj);

    private native String toString(long tstate, long pyObject);

    private native long hashCode(long tstate, long pyObject);

    /**
     * Create a dynamic proxy that implements the provided interfaces by calling
     * the corresponding Python methods on this PyObject. This Python object
     * must have methods matching those defined by the Java interfaces. Matching
     * methods must have the same name, same number of arguments, and must
     * return an object that can be converted to the correct return type. This
     * method does not verify that this Python object has methods matching the
     * Java interfaces. If a method is called on the proxy object that does not
     * have a matching Python method a JepException will be thrown. The
     * returned proxy object can only be used when this PyObject is valid. It
     * cannot be used on other threads or after the Interpreter that it
     * originated from is closed.
     *
     * @param <T>
     *            the generic type of the return type
     * @param primaryInterface
     *            A interface the returned object will implement
     * @param extraInterfaces
     *            Optional additional interfaces the returned object will also
     *            implement.
     * @return a Java proxy implementing the provided interfaces.
     * @throws JepException
     *             if an error occurs or the conversion is not possible
     * @since 3.9
     */
    public <T> T proxy(Class<T> primaryInterface, Class<?>... extraInterfaces)
            throws JepException {
        ClassLoader loader = pointer.memoryManager.getClassLoader();
        Class<?>[] interfaces = null;
        if (extraInterfaces == null || extraInterfaces.length == 0) {
            interfaces = new Class<?>[] { primaryInterface };
        } else {
            interfaces = new Class<?>[extraInterfaces.length + 1];
            interfaces[0] = primaryInterface;
            System.arraycopy(extraInterfaces, 0, interfaces, 1,
                    extraInterfaces.length);
        }
        InvocationHandler ih = new InvocationHandler(this, false);
        return primaryInterface
                .cast(Proxy.newProxyInstance(loader, interfaces, ih));
    }

    /**
     * Attempt to convert this object to a Java equivalant using the builtin Jep
     * conversions. The supported conversions are described in
     * {@link jep.Interpreter#getValue(String, Class)}.
     * 
     * @param <T>
     *            the generic type of the return type
     * @param expectedType
     *            the Java class of the return type.
     * @return a Java version of this object
     * @throws JepException
     *             if an error occurs or the conversion is not possible
     * @since 3.9
     */
    public <T> T as(Class<T> expectedType) throws JepException {
        return expectedType.cast(as(tstate(), pointer.pyObject, expectedType));
    }

    private native Object as(long tstate, long pyObject, Class<?> expectedType)
            throws JepException;
}
