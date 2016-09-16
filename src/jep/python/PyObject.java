package jep.python;

import jep.Jep;
import jep.JepException;


/**
 * <pre>
 * PyObject.java - encapsulates a pointer to a PyObject
 *
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
 *
 * Created: Thu Sep 7 11:52:03 2006
 *
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 */
public class PyObject {
    /**
     * the jep that created this object
     *
     */
    protected Jep  jep = null;

    /**
     * the pointer to the PyObject
     *
     */
    protected long obj = 0;

    /**
     * thread state pointer in python
     *
     */
    protected long tstate = 0;


    /**
     * Make a new PyObject
     * 
     * @param tstate a <code>long</code> value
     * @param obj
     * @param jep
     * @exception JepException if an error occurs
     */
    public PyObject(long tstate, long obj, Jep jep) throws JepException {
        this.tstate = tstate;
        this.obj    = obj;
        this.jep    = jep;

        if(obj == 0)
            throw new JepException("Unable to create object, NULL.");
    }


    /**
     * Check if PyObject is valid
     * 
     * @throws JepException
     */
    public void isValid() throws JepException {
        if(obj == 0)
            throw new JepException("Object: Invalid pointer.");
        jep.isValidThread();
    }


    /**
     * internal use only
     *
     * @exception JepException if an error occurs
     */
    public void decref() throws JepException {
        isValid();
        this.decref(this.obj);
    }


    private native void decref(long ptr) throws JepException;


    /**
     * internal use only
     *
     * @exception JepException if an error occurs
     */
    public void incref() throws JepException {
        isValid();
        this.incref(this.obj);
    }


    private native void incref(long ptr) throws JepException;


    /**
     * I will be closed automagically.
     * 
     */
    public void close() {
        try {
            if(this.obj == 0)
                return;

            this.decref();
        }
        catch(JepException e) {
            // shouldn't happen?
            ;
        }

        this.obj = 0;
    }


    // ------------------------------ set things

    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v an <code>Object</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, Object v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }

    private native void set(long tstate, long module, String name, Object v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>String</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, String v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }

    private native void set(long tstate, long module, String name, String v)
        throws JepException;



    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>boolean</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, boolean v) throws JepException {
        // there's essentially no difference between int and bool...
        if(v)
            set(name, 1);
        else
            set(name, 0);
    }


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v an <code>int</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, int v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }
    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v an <code>int</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, short v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }
    
    private native void set(long tstate, long module, String name, int v)
        throws JepException;

    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>char[]</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, char[] v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, new String(v));
    }


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>char</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, char v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, new String(new char[] { v }));
    }


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param b a <code>byte</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, byte b) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, b);
    }

    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>long</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, long v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }
    
    private native void set(long tstate, long module, String name, long v)
        throws JepException;
    
    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>double</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, double v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }
    
    private native void set(long tstate, long module, String name, double v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>float</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, float v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }
    
    private native void set(long tstate, long module, String name, float v)
        throws JepException;


    // -------------------------------------------------- set arrays

    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>boolean[]</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, boolean[] v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }

    private native void set(long tstate, long module, String name, boolean[] v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v an <code>int[]</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, int[] v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }

    private native void set(long tstate, long module, String name, int[] v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>short[]</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, short[] v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }

    private native void set(long tstate, long module, String name, short[] v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>byte[]</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, byte[] v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }

    private native void set(long tstate, long module, String name, byte[] v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>long[]</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, long[] v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }

    private native void set(long tstate, long module, String name, long[] v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>double[]</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, double[] v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }

    private native void set(long tstate, long module, String name, double[] v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>float[]</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, float[] v) throws JepException {
        isValid();
        set(this.tstate, this.obj, name, v);
    }

    private native void set(long tstate, long module, String name, float[] v)
        throws JepException;


    /**
     * Create a module.
     *
     * <b>Internal use only.</b>
     *
     * @param tstate a <code>long</code> value
     * @param onModule a <code>long</code> value
     * @param name a <code>String</code> value
     * @return a <code>long</code> value
     * @exception JepException if an error occurs
     */
    protected native long createModule(long tstate,
                                       long onModule,
                                       String name) throws JepException;


    /**
     * Get a string value from a module.
     *
     * <b>Internal use only.</b>
     *
     * @param tstate a <code>long</code> value
     * @param onModule a <code>long</code> value
     * @param str a <code>String</code> value
     * @return an <code>Object</code> value
     * @exception JepException if an error occurs
     */
    protected native Object getValue(long tstate,
                                     long onModule,
                                     String str) throws JepException;
}
