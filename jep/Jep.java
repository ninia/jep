package jep;

import java.io.File;

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
public final class Jep {

    private String hash = null;

    /**
     * Creates a new <code>Jep</code> instance.
     *
     * @exception JepException if an error occurs
     */
    public Jep() throws JepException {
        super();
        this.hash = String.valueOf(this.hashCode());
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
            cl = this.getClass().getClassLoader();
        
        run(hash, cl, script);
    }
    

    private synchronized native void run(String hash,
                                         ClassLoader cl,
                                         String script)
        throws JepException;


    // -------------------------------------------------- set things

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
