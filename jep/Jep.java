package jep;

/**
 * Jep.java
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
 * @author <a href="mailto:mrjohnson0@sourceforge.net">Mike Johnson</a>
 * @version 1.0
 */

import java.io.File;

public final class Jep {
    
    private String hash = null;
    
    // used by default if not passed/set by caller
    private ClassLoader classLoader = null;
    
    // eval() storage.
    private StringBuffer evalLines = null;
    
    
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

    
    // load C shared library
    static {
        System.loadLibrary("jep");
    }


    private static synchronized native void init(String hash) throws JepException;


    /**
     * run a python script.
     *
     * @param script a <code>String</code> value
     * @exception JepException if an error occurs
     */
    public void runScript(String script) throws JepException {
        runScript(script, null);
    }


    /**
     * run a python script.
     *
     * @param script a <code>String</code> value
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
    

    private synchronized native void run(String hash,
                                         ClassLoader cl,
                                         String script)
        throws JepException;


    /**
     * evaluate python statements
     *
     * @param str a <code>String</code> value
     * @exception JepException if an error occurs
     */
    public void eval(String str) throws JepException {
        char last = str.charAt(str.length() - 1);
        boolean finished = true;
        
        // trim comments
        int pos = str.indexOf("#");
        if(pos > -1)
            str = str.substring(0, pos);
        
        if(last == '\\' || last == ':')
            finished = false;
        else if(Character.isWhitespace(str.charAt(0)))
            finished = false;
        
        if(!finished) {
            if(this.evalLines == null)
                this.evalLines = new StringBuffer();
            evalLines.append(str + "\n");
            return;
        }
        
        // may need to run previous lines
        if(this.evalLines != null) {
            eval(this.hash, this.classLoader, this.evalLines.toString());
            this.evalLines = null;
        }
        
        eval(this.hash, this.classLoader, str);
    }

    
    private synchronized native void eval(String hash,
                                          ClassLoader cl,
                                          String str)
        throws JepException;


    /**
     * retrieve a value from python. if the result is not a java object,
     * implementation currently returns a String
     *
     * @param str a <code>String</code> value
     * @return an <code>Object</code> value
     * @exception JepException if an error occurs
     */
    public Object getValue(String str) throws JepException {
        return getValue(this.hash, this.classLoader, str);
    }

    
    private synchronized native Object getValue(String hash,
                                                ClassLoader cl,
                                                String str)
        throws JepException;
    

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

    private synchronized native void set(String hash, String name, Object v)
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

    private synchronized native void set(String hash, String name, String v)
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
    
    private synchronized native void set(String hash, String name, int v)
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
    
    private synchronized native void set(String hash, String name, long v)
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
    
    private synchronized native void set(String hash, String name, double v)
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
    
    private synchronized native void set(String hash, String name, float v)
        throws JepException;

    // -------------------------------------------------- close me

    /**
     * Shutdown python interpreter. Make sure you call this.
     *
     */
    public synchronized void close() {
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
