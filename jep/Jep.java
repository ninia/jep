package jep;

import jep.python.*;
import java.io.File;
import java.util.ArrayList;


/**
 * <pre>
 * Jep.java - Embeds CPython in Java.
 *
 * Copyright (c) 2004, 2005 Mike Johnson.
 *
 * This file is licenced under the the zlib/libpng License.
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
 * Created: Fri Apr 30 10:35:03 2004
 *
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$
 */
public final class Jep {
    
    private boolean closed = false;
    private long    tstate = 0;
    // all calls must originate from same thread
    private Thread thread = null;
    
    // used by default if not passed/set by caller
    private ClassLoader classLoader = null;
    
    // eval() storage.
    private StringBuffer evalLines   = null;
    private boolean      interactive = false;
    
    // windows requires this as unix newline...
    private static final String LINE_SEP = "\n";

    /*
     * keep track of objects that we create. do this to prevent
     * crashes in userland if jep is closed.
     *
     */
    private ArrayList<PyObject> pythonObjects = new ArrayList<PyObject>();
    
    
    // -------------------------------------------------- constructors
    
    /**
     * Creates a new <code>Jep</code> instance.
     *
     * @exception JepException if an error occurs
     */
    public Jep() throws JepException {
        this(false, null, null);
    }

    
    /**
     * Creates a new <code>Jep</code> instance.
     *
     * @param interactive a <code>boolean</code> value
     * @exception JepException if an error occurs
     */
    public Jep(boolean interactive) throws JepException {
        this(interactive, null, null);
    }
    
    
    /**
     * Creates a new <code>Jep</code> instance.
     *
     * @param interactive a <code>boolean</code> value
     * @param includePath a ':' delimited <code>String</code> of directories
     * @exception JepException if an error occurs
     */
    public Jep(boolean interactive, String includePath) throws JepException {
        this(interactive, includePath, null);
    }
    
    
    /**
     * Creates a new <code>Jep</code> instance.
     *
     * @param interactive a <code>boolean</code> value
     * @param includePath a <code>String</code> value
     * @param cl a <code>ClassLoader</code> value
     * @exception JepException if an error occurs
     */
    public Jep(boolean interactive,
               String includePath,
               ClassLoader cl) throws JepException {
        
        if(cl == null)
            this.classLoader = this.getClass().getClassLoader();
        else
            this.classLoader = cl;
        
        this.interactive = interactive;
        this.tstate = init(this.classLoader);
        this.thread = Thread.currentThread();
        
        // why write C code if you don't have to? :-)
        if(includePath != null) {
            eval("import sys");
            eval("sys.path += '" + includePath + "'.split(':')");
        }
    }
    
    
    // load shared library
    static {
        System.loadLibrary("jep");
    }
    
    
    private static native long init(ClassLoader classloader) throws JepException;
    
    
    /**
     * all calls must check thread
     *
     * @exception JepException if an error occurs
     */
    public void isValidThread() throws JepException {
        if(this.thread != Thread.currentThread())
            throw new JepException("Invalid thread access.");
        if(this.tstate == 0)
            throw new JepException("Initialization failed.");
    }


    /**
     * run a python script.
     *
     * @param script a <code>String</code> absolute path to script file.
     * @exception JepException if an error occurs
     */
    public void runScript(String script) throws JepException {
        runScript(script, null);
    }


    /**
     * run a python script.
     *
     * @param script a <code>String</code> absolute path to script file.
     * @param cl a <code>ClassLoader</code> value, may be null.
     * @exception JepException if an error occurs
     */
    public void runScript(String script, ClassLoader cl) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        if(script == null)
            throw new JepException("Script filename cannot be null.");
        
        File file = new File(script);
        if(!file.exists() || !file.canRead())
            throw new JepException("Invalid file: " + file.getAbsolutePath());

        setClassLoader(cl);
        run(this.tstate, script);
    }
    

    private native void run(long tstate,
                            String script) throws JepException;
    

    /**
     * <pre>
     * Evaluate python statements.
     *
     * In interactive mode, Jep may not immediately execute the given lines of code.
     * In that case, eval() returns false and the statement is remembered and
     * is appended to the next incoming string.
     *
     * If you're running an unknown number of statements, finish with
     * <code>eval(null)</code> to flush the statement buffer.
     *
     * Interactive mode is slower than a straight eval call since it has to
     * compile the code strings to detect the end of the block.
     * Non-interactive mode is faster, but code blocks must be complete.
     *
     * For Example:
     * <blockquote>eval("if(Test):\n\tprint 'w00t'")</blockquote>
     * This is a limitation on the Python interpreter and unlikely to change.
     *
     * Also, Python does not readly return object values from eval(). Use
     * {@link #getValue(java.lang.String)} instead.
     *
     * @param str a <code>String</code> value
     * @return true if statement complete and was executed.
     * @exception JepException if an error occurs
     */
    public boolean eval(String str) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        try {
            // trim windows \r\n
            if(str != null)
                str = str.replaceAll("\r", "");
            
            if(str == null || str.trim().equals("")) {
                if(!this.interactive)
                    return false;
                if(this.evalLines == null)
                    return true; // nothing to eval

                // null means we send lines, whether or not it compiles.
                eval(this.tstate, this.evalLines.toString());
                this.evalLines = null;
                return true;
            }
            else {
                // first check if it compiles by itself
                if(this.evalLines == null &&
                   compileString(this.tstate, str) == 1) {
                    
                    eval(this.tstate, str);
                    return true;
                }
                
                // doesn't compile on it's own, append to eval
                if(this.evalLines == null)
                    this.evalLines = new StringBuffer();
                else
                    evalLines.append(LINE_SEP);
                evalLines.append(str);
            }
            
            return false;
        }
        catch(JepException e) {
            this.evalLines = null;
            throw new JepException(e);
        }
    }


    private native int compileString(long tstate,
                                     String str) throws JepException;

    
    private native void eval(long tstate,
                             String str) throws JepException;


    /**
     * <pre>
     * Retrieves a value from python. If the result is not a java object,
     * the implementation currently returns a String.
     *
     * Python is pretty picky about what it excepts here. The general syntax:
     * <blockquote>eval("a = 5")
     *String a = (String) getValue("a")</blockquote>
     * will work.
     * </pre>
     *
     * @param str a <code>String</code> value
     * @return an <code>Object</code> value
     * @exception JepException if an error occurs
     */
    public Object getValue(String str) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        return getValue(this.tstate, str);
    }

    
    private native Object getValue(long tstate,
                                   String str) throws JepException;
    

    private PyObject trackObject(PyObject obj) throws JepException {
        // make sure python doesn't close it
        obj.incref();
        this.pythonObjects.add(obj);
        return obj;
    }


    /**
     * Create a python module on the interpreter. If the given name is
     * valid, imported module, this method will return that module.
     *
     * @param name a <code>String</code> value
     * @return a <code>PyModule</code> value
     * @exception JepException if an error occurs
     */
    public PyModule createModule(String name) throws JepException {
        return (PyModule) trackObject(new PyModule(
                                          this.tstate,
                                          createModule(this.tstate,
                                                       name),
                                          this));
    }


    private native long createModule(long tstate,
                                     String name) throws JepException;


    // -------------------------------------------------- set things
    
    /**
     * set default classloader
     *
     * @param cl a <code>ClassLoader</code> value
     */
    public void setClassLoader(ClassLoader cl) {
        if(cl != null && cl != this.classLoader) {
            this.classLoader = cl;
            // call native set
            setClassLoader(this.tstate, cl);
        }
    }
    
    
    private native void setClassLoader(long tstate, ClassLoader cl);
    
    
    /**
     * changes behavior of eval()
     *
     * @param v a <code>boolean</code> value
     */
    public void setInteractive(boolean v) {
        this.interactive = v;
    }
    
    
    /**
     * get interactive
     *
     * @return a <code>boolean</code> value
     */
    public boolean isInteractive() {
        return this.interactive;
    }

    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v an <code>Object</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, Object v) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        set(tstate, name, v);
    }

    private native void set(long tstate, String name, Object v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>String</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, String v) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        set(tstate, name, v);
    }

    private native void set(long tstate, String name, String v)
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
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        set(tstate, name, v);
    }
    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v an <code>int</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, short v) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        set(tstate, name, (int) v);
    }
    
    private native void set(long tstate, String name, int v)
        throws JepException;

    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>char[]</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, char[] v) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        set(tstate, name, new String(v));
    }


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>char</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, char v) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        set(tstate, name, new String(new char[] { v }));
    }


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param b a <code>byte</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, byte b) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        set(tstate, name, (int) b);
    }

    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>long</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, long v) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();

        set(tstate, name, v);
    }
    
    private native void set(long tstate, String name, long v)
        throws JepException;
    
    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>double</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, double v) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();

        set(tstate, name, v);
    }
    
    private native void set(long tstate, String name, double v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>float</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, float v) throws JepException {
        if(this.closed)
            throw new JepException("Jep has been closed.");
        isValidThread();
        
        set(tstate, name, v);
    }
    
    private native void set(long tstate, String name, float v)
        throws JepException;

    // -------------------------------------------------- close me

    /**
     * Shutdown python interpreter. Make sure you call this.
     *
     */
    public synchronized void close() {
        if(this.closed)
            return;
        
        // close all the PyObjects we created
        for(int i = 0; i < this.pythonObjects.size(); i++)
            pythonObjects.get(i).close();

        this.closed = true;
        this.close(tstate);
        this.tstate = 0;
    }

    private native void close(long tstate);
    
    
    /**
     * Describe <code>finalize</code> method here.
     *
     */
    protected void finalize() {
        this.close();
    }
}
