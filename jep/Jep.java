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
    
    private String hash = null;
    
    // used by default if not passed/set by caller
    private ClassLoader classLoader = null;
    
    // eval() storage.
    private StringBuffer evalLines = null;
    
    private boolean interactive = false;
    
    /**
     * Creates a new <code>Jep</code> instance.
     *
     * @exception JepException if an error occurs
     */
    public Jep() throws JepException {
        super();
        this.hash = String.valueOf(this.hashCode());
        this.classLoader = this.getClass().getClassLoader();
        init(hash);
    }

    
    /**
     * Creates a new <code>Jep</code> instance.
     *
     * @param interactive a <code>boolean</code> value
     * @exception JepException if an error occurs
     */
    public Jep(boolean interactive) throws JepException {
        super();
        this.hash = String.valueOf(this.hashCode());
        this.classLoader = this.getClass().getClassLoader();
        this.interactive = interactive;
        init(hash);
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
        this.hash = String.valueOf(this.hashCode());
        this.classLoader = this.getClass().getClassLoader();
        this.interactive = interactive;
        init(hash);
        
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


    private static synchronized native void init(String hash) throws JepException;


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
        
        run(this.hash, cl, script);
    }
    

    private native void run(String hash,
                            ClassLoader cl,
                            String script) throws JepException;


    /**
     * evaluate python statements
     *
     * @param str a <code>String</code> value
     * @return true if statement complete and was executed.
     * @exception JepException if an error occurs
     */
    public boolean eval(String str) throws JepException {
        boolean finished = true;
        
        try {
            if(str == null || str.equals("")) {
                str = null;
                if(!this.interactive)
                    return false;
                else {
                    if(this.evalLines == null)
                        return true; // nothing to eval
                }
            }
            else {
                char last = str.charAt(str.length() - 1);
            
                // trim comments
                int pos = str.indexOf("#");
                if(pos > -1)
                    str = str.substring(0, pos);
            
                if(last == '\\' || last == ':')
                    finished = false;
                else if(Character.isWhitespace(str.charAt(0)))
                    finished = false;
            }

            if(!finished) {
                if(this.evalLines == null)
                    this.evalLines = new StringBuffer();
                evalLines.append(str + "\n");
                return false;
            }
        
            // may need to run previous lines
            if(this.evalLines != null) {
                eval(this.hash,
                     this.classLoader,
                     this.evalLines.toString());
                this.evalLines = null;
            
                if(str == null)
                    return true;
            }
        
            if(str != null) {
                eval(this.hash, this.classLoader, str);
                return true;
            }
        }
        catch(JepException e) {
            this.evalLines = null;
            throw new JepException(e);
        }
        
        return false;
    }

    
    private native void eval(String hash,
                             ClassLoader cl,
                             String str) throws JepException;


    /**
     * Retrieves a value from python. If the result is not a java object,
     * the implementation currently returns a String
     *
     * @param str a <code>String</code> value
     * @return an <code>Object</code> value
     * @exception JepException if an error occurs
     */
    public Object getValue(String str) throws JepException {
        return getValue(this.hash, this.classLoader, str);
    }

    
    private native Object getValue(String hash,
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
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v an <code>Object</code> value
     * @exception JepException if an error occurs
     */
    public void set(String name, Object v)
        throws JepException {
        set(hash, name, v);
    }

    private native void set(String hash, String name, Object v)
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
        set(hash, name, v);
    }

    private native void set(String hash, String name, String v)
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
            set(hash, name, 1);
        else
            set(hash, name, 0);
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
        set(hash, name, v);
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
        set(hash, name, (int) v);
    }
    
    private native void set(String hash, String name, int v)
        throws JepException;

    
    /**
     * Describe <code>set</code> method here.
     *
     * @param name a <code>String</code> value
     * @param v a <code>char[]</code> value
     * @exception JepException if an error occurs
     */
    private void set(String hash, String name, char[] v)
        throws JepException {
        set(hash, name, new String(v));
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
        set(hash, name, v);
    }
    
    private native void set(String hash, String name, long v)
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
        set(hash, name, v);
    }
    
    private native void set(String hash, String name, double v)
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
        set(hash, name, v);
    }
    
    private native void set(String hash, String name, float v)
        throws JepException;

    // -------------------------------------------------- close me

    /**
     * Shutdown python interpreter. Make sure you call this.
     *
     */
    public void close() {
        if(this.hash == null)
            return;
        
        this.close(hash);
        this.hash = null;
    }

    private native void close(String hash);
    
    
    /**
     * Describe <code>finalize</code> method here.
     *
     */
    protected void finalize() {
        this.close();
    }
}
