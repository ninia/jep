/**
 * Copyright (c) 2006-2018 JEP AUTHORS.
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

import jep.Jep;
import jep.JepException;

/**
 * A Java object that wraps a pointer to a Python object.
 * 
 * This class is not thread safe and PyObjects can only be used on the Thread
 * where they were created. When a Jep instance is closed all PyObjects from
 * that instance will be invalid and can no longer be used.
 *
 * This class is in the process of a redesign so methods may be added, removed,
 * or changed in ways that are not backwards compatible in the future. When
 * using this class it may require extra effort to move to a new version of Jep.
 */
public class PyObject implements AutoCloseable {

    protected final PyPointer pointer;

    protected final Jep jep;

    /**
     * Make a new PyObject
     * 
     * @param tstate
     *            a <code>long</code> value
     * @param pyObject
     *            the address of the python object
     * @param jep
     *            the instance of jep that created this object
     * @exception JepException
     *                if an error occurs
     */
    public PyObject(long tstate, long pyObject, Jep jep) throws JepException {
        this.jep = jep;
        this.pointer = new PyPointer(this, jep, tstate, pyObject);
    }

    /**
     * Called from native code
     * 
     * @return the address of the native PyObject
     */
    protected long getPyObject() {
        return pointer.pyObject;
    }

    /**
     * Check if PyObject is valid.
     * 
     * @deprecated In a future release this method will not be public and/or its
     *             method signature may change.
     * 
     * @throws JepException
     *             if it is not safe to use this python object
     */
    @Deprecated
    public void isValid() throws JepException {
        checkValid();
    }

    /**
     * Check if PyObject is valid.
     * 
     * @throws JepException
     *             if it is not safe to use this python object
     */
    protected void checkValid() throws JepException {
        jep.isValidThread();
        if (this.pointer.isDisposed()) {
            throw new JepException(
                    getClass().getSimpleName() + " has been closed.");
        }
    }

    /**
     * Check if PyObject is valid. This is an alternative to isValid() if you
     * need a RuntimeException.
     * 
     * @throws IllegalStateException
     *             if it is not safe to use this python object
     */
    protected void checkValidRuntime() throws IllegalStateException {
        try {
            checkValid();
        } catch (JepException e) {
            throw new IllegalStateException(e);
        }
    }

    /**
     * @deprecated internal use only
     *
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void decref() throws JepException {
        isValid();
        this.decref(pointer.tstate, pointer.pyObject);
    }

    private native void decref(long tstate, long ptr) throws JepException;

    /**
     * @deprecated internal use only
     *
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void incref() throws JepException {
        isValid();
        this.incref(pointer.tstate, pointer.pyObject);
    }

    private native void incref(long tstate, long ptr) throws JepException;

    @Override
    public void close() throws JepException {
        /*
         * Do not use isValid() because there should be no exception if this is
         * already closed.
         */
        jep.isValidThread();
        this.pointer.dispose();
    }

    // ------------------------------ set things

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            an <code>Object</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, Object v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, Object v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>String</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, String v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, String v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>boolean</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, boolean v) throws JepException {
        // there's essentially no difference between int and bool...
        if (v) {
            set(name, 1);
        } else {
            set(name, 0);
        }
    }

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            an <code>int</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, int v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            an <code>int</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, short v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, int v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>char[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, char[] v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, new String(v));
    }

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>char</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, char v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name,
                new String(new char[] { v }));
    }

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param b
     *            a <code>byte</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, byte b) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, b);
    }

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>long</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, long v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, long v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>double</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, double v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, double v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>float</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, float v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, float v)
            throws JepException;

    // -------------------------------------------------- set arrays

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>boolean[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, boolean[] v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, boolean[] v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            an <code>int[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, int[] v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, int[] v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>short[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, short[] v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, short[] v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>byte[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, byte[] v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, byte[] v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>long[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, long[] v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, long[] v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>double[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, double[] v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, double[] v)
            throws JepException;

    /**
     * Describe <code>set</code> method here.
     * 
     * @deprecated Use {@link #setAttr(String, Object)} instead.
     *
     * @param name
     *            a <code>String</code> value
     * @param v
     *            a <code>float[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public void set(String name, float[] v) throws JepException {
        isValid();
        set(pointer.tstate, pointer.pyObject, name, v);
    }

    private native void set(long tstate, long module, String name, float[] v)
            throws JepException;

    /**
     * Access an attribute of the wrapped Python Object, similar to the Python
     * built-in function getattr. This is equivalent to the Python statement
     * <code>this.attr_name</code>.
     * 
     * @param attr_name
     *            the attribute name
     * @return a Java version of the attribute
     * @exception JepException
     *                if an error occurs
     * @since 3.8
     */
    public Object getAttr(String attr_name) throws JepException {
        checkValid();
        return getAttr(pointer.tstate, pointer.pyObject, attr_name,
                Object.class);
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
     * @exception JepException
     *                if an error occurs
     * @since 3.8
     */
    public <T> T getAttr(String attr_name, Class<T> clazz) throws JepException {
        checkValid();
        return clazz.cast(
                getAttr(pointer.tstate, pointer.pyObject, attr_name, clazz));
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
        checkValid();
        setAttr(pointer.tstate, pointer.pyObject, attr_name, o);
    }

    private native void setAttr(long tstate, long pyObject, String attr_name,
            Object o);

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
        checkValid();
        delAttr(pointer.tstate, pointer.pyObject, attr_name);
    }

    private native void delAttr(long tstate, long pyObject, String attr_name);

    /**
     * Create a module.
     *
     * <b>Internal use only.</b>
     *
     * @param tstate
     *            a <code>long</code> value
     * @param onModule
     *            a <code>long</code> value
     * @param name
     *            a <code>String</code> value
     * @return a <code>long</code> value
     * @exception JepException
     *                if an error occurs
     */
    protected native long createModule(long tstate, long onModule, String name)
            throws JepException;

    /**
     * Get a string value from a module.
     *
     * <b>Internal use only.</b>
     *
     * @param tstate
     *            a <code>long</code> value
     * @param onModule
     *            a <code>long</code> value
     * @param str
     *            a <code>String</code> value
     * @return an <code>Object</code> value
     * @exception JepException
     *                if an error occurs
     */
    protected native Object getValue(long tstate, long onModule, String str)
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
        checkValidRuntime();
        return equals(pointer.tstate, pointer.pyObject, obj);
    }

    /**
     * Produces the string representation of the wrapped Python object by using
     * the Python built-in method str.
     */
    @Override
    public String toString() {
        checkValidRuntime();
        return toString(pointer.tstate, pointer.pyObject);
    }

    /**
     * Produces the hash code of the wrapped Python object by using the Python
     * built-in method hash. Hash codes are not consistent between languages.
     * For example the hash code of the string "hello" will be different in
     * Python than in Java, even if this PyObject wrapped the string "hello".
     */
    @Override
    public int hashCode() {
        checkValidRuntime();
        Long value = hashCode(pointer.tstate, pointer.pyObject);
        return value.hashCode();
    }

    private native boolean equals(long tstate, long pyObject, Object obj);

    private native String toString(long tstate, long pyObject);

    private native long hashCode(long tstate, long pyObject);

}
