package jep;

import java.io.File;

/**
 * <pre>
 * Jep.java - Embeds CPython in Java.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 *
 * Created: Fri Apr 30 10:35:03 2004
 *
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$
 */
public final class Jep {
    
    private int tstate = -1;
    
    // used by default if not passed/set by caller
    private ClassLoader classLoader = null;
    
    // eval() storage.
    private StringBuffer evalLines   = null;
    private boolean      interactive = false;
    
    // windows requires this as unix newline...
    private static final String LINE_SEP = "\n";
    
    
    /**
     * Creates a new <code>Jep</code> instance.
     *
     * @exception JepException if an error occurs
     */
    public Jep() throws JepException {
        super();
        this.classLoader = this.getClass().getClassLoader();
        this.tstate = init();
    }

    
    /**
     * Creates a new <code>Jep</code> instance.
     *
     * @param interactive a <code>boolean</code> value
     * @exception JepException if an error occurs
     */
    public Jep(boolean interactive) throws JepException {
        super();
        this.classLoader = this.getClass().getClassLoader();
        this.interactive = interactive;
        this.tstate = init();
    }
    
    
    /**
     * Creates a new <code>Jep</code> instance.
     *
     * @param interactive a <code>boolean</code> value
     * @param includePath a ':' delimited <code>String</code> of directories
     * @exception JepException if an error occurs
     */
    public Jep(boolean interactive, String includePath) throws JepException {
        super();
        this.classLoader = this.getClass().getClassLoader();
        this.interactive = interactive;
        this.tstate = init();
        
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


    private static synchronized native int init() throws JepException;


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
        if(script == null)
            throw new JepException("Script filename cannot be null.");
        
        File file = new File(script);
        if(!file.exists() || !file.canRead())
            throw new JepException("Invalid file: " + file.getAbsolutePath());

        if(cl == null)
            cl = this.classLoader;
        
        run(this.tstate, cl, script);
    }
    

    private native void run(int tstate,
                            ClassLoader cl,
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
        try {
            // trim windows \r\n
            if(str != null) {
                int p = str.indexOf("\r");
                if(p >= 0)
                    str = str.substring(0, p);
            }
            
            if(str == null || str.trim().equals("")) {
                if(!this.interactive)
                    return false;
                if(this.evalLines == null)
                    return true; // nothing to eval

                // null means we send lines, whether or not it compiles.
                eval(this.tstate,
                     this.classLoader,
                     this.evalLines.toString());
                this.evalLines = null;
                return true;
            }
            else {
                // first check if it compiles by itself
                if(this.evalLines == null &&
                   compileString(this.tstate, str) == 1) {
                    
                    eval(this.tstate,
                         this.classLoader,
                         str);
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


    private native int compileString(int tstate,
                                     String str) throws JepException;

    
    private native void eval(int tstate,
                             ClassLoader cl,
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
        return getValue(this.tstate, this.classLoader, str);
    }

    
    private native Object getValue(int tstate,
                                   ClassLoader cl,
                                   String str) throws JepException;
    

    // -------------------------------------------------- set things
    
    /**
     * set default classloader
     *
     * @param cl a <code>ClassLoader</code> value
     */
    public void setClassLoader(ClassLoader cl) {
        if(cl != null)
            this.classLoader = cl;
    }
    
    
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
    public void set(String name, Object v)
        throws JepException {
        set(tstate, name, v);
    }

    private native void set(int tstate, String name, Object v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>String</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, String v)
        throws JepException {
        set(tstate, name, v);
    }

    private native void set(int tstate, String name, String v)
        throws JepException;



    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>boolean</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, boolean v)
        throws JepException {

        // there's essentially no difference between int and bool...
        if(v)
            set(tstate, name, 1);
        else
            set(tstate, name, 0);
    }


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v an <code>int</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, int v)
        throws JepException {
        set(tstate, name, v);
    }
    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v an <code>int</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, short v)
        throws JepException {
        set(tstate, name, (int) v);
    }
    
    private native void set(int tstate, String name, int v)
        throws JepException;

    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>char[]</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, char[] v) throws JepException {
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
        set(tstate, name, (int) b);
    }

    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>long</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, long v)
        throws JepException {
        set(tstate, name, v);
    }
    
    private native void set(int tstate, String name, long v)
        throws JepException;
    
    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>double</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, double v)
        throws JepException {
        set(tstate, name, v);
    }
    
    private native void set(int tstate, String name, double v)
        throws JepException;


    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>float</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, float v)
        throws JepException {
        set(tstate, name, v);
    }
    
    private native void set(int tstate, String name, float v)
        throws JepException;

    // -------------------------------------------------- close me

    /**
     * Shutdown python interpreter. Make sure you call this.
     *
     */
    public void close() {
        if(this.tstate == -1)
            return;
        
        this.close(tstate);
        this.tstate = -1;
    }

    private native void close(int tstate);
    
    
    /**
     * Describe <code>finalize</code> method here.
     *
     */
    protected void finalize() {
        this.close();
    }
}
