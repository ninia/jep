package jep;

import java.io.Reader;
import java.io.File;
import java.io.IOException;
import java.io.FileWriter;

import javax.script.*;


/**
 * <pre>
 * JepScriptEngine.java - Embeds CPython in Java.
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
 * Created: Tue Sep 5 18:35:03 2006
 *
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$
 */
public class JepScriptEngine implements ScriptEngine {
    private Jep inter = null;

    private Bindings bindings       = null;
    private Bindings globalBindings = null;

    private ScriptContext context = null;

    private ScriptEngineFactory factory = null;


    protected JepScriptEngine() throws ScriptException {
        try {
            this.inter = new Jep(true); // make interactive because javax.script sucks
            this.inter.setClassLoader(Thread.currentThread().getContextClassLoader());
        }
        catch(JepException e) {
            throw new ScriptException(e.getMessage());
        }
    }


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#createBindings()
	 */
	public Bindings createBindings() {
		return new SimpleBindings();
	}


	/*
     * <pre>
     * Run script from reader.
     *
     * Performance of this method will suck compared to using
     * Jep.runScript(). If you care, read the rant in the source of
     * this method. Or, just don't use javax.script.
     *
     * </pre>
     *
	 * @see javax.script.ScriptEngine#eval(java.io.Reader)
	 */
	public Object eval(Reader reader) throws ScriptException {

        /**
         * thanks to javax.script's fucking retard stupidity, i don't
         * have a goddamn file name. every fucking script engine works
         * off a filename, so why i have to read the file my own self
         * when python could do it (oh, and byte compile the damn
         * thing) i don't fucking know. not that the jsr people would
         * know how much this sucks for me, because it's too damned
         * complicated to send a simple "you're making my life
         * difficult, here's why..." comment. comments are closed.
         *
         * so yeah, performance with this will suck compared to just
         * sending a damned file name. maybe you should just use
         * Jep.runScript and forget about this bullshit.
         *
         */

        // turn off interactive mode
        this.inter.setInteractive(false);

        FileWriter fout = null;
        File       temp = null;

        try {
            temp = File.createTempFile("jep", ".py");
            fout = new FileWriter(temp);
        }
        catch(IOException e) {
            throw new ScriptException("Couldn't create temp file: " +
                                      e.getMessage());
        }

        try {
            char[] buf = new char[1024];
            int count;

            while((count = reader.read(buf)) > 0)
                fout.write(buf, 0, count);

            fout.close();

            // okay sic jep on it
            this.inter.runScript(temp.getAbsolutePath());
        }
        catch(IOException e) {
            throw new ScriptException("Error writing to file: " +
                                      e.getMessage());
        }
        catch(JepException e) {
            throw new ScriptException(e.getMessage());
        }
        finally {
            try {
                temp.delete();
            }
            catch(SecurityException e) {
                ;                   // jre will delete it, anyhow
            }
        }

        return null;
	}


	/**
     * Note: always returns null due to Python limitations.
     *
     * (non-Javadoc)
	 * @see javax.script.ScriptEngine#eval(java.io.Reader, javax.script.ScriptContext)
	 */
	public Object eval(Reader reader,
                       ScriptContext context) throws ScriptException {
        this.context = context;
		return eval(reader);
	}


	/**
     * Note: always returns null due to Python limitations.
     *
     * (non-Javadoc)
	 * @see javax.script.ScriptEngine#eval(java.io.Reader, javax.script.Bindings)
	 */
	public Object eval(Reader reader,
                       Bindings bindings) throws ScriptException {
        this.bindings = bindings;
		return eval(reader);
	}


	/**
     * Note: always returns null due to Python limitations.
     *
     * (non-Javadoc)
	 * @see javax.script.ScriptEngine#eval(java.lang.String)
	 */
	public Object eval(String line) throws ScriptException {
        this.inter.setInteractive(true);

        try {
            this.inter.eval(line);
        }
        catch(JepException e) {
            throw new ScriptException(e.getMessage());
        }

		return null;
	}


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#eval(java.lang.String, javax.script.ScriptContext)
	 */
	public Object eval(String line,
                       ScriptContext context) throws ScriptException {
        this.context = context;
		return eval(line);
	}


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#eval(java.lang.String, javax.script.Bindings)
	 */
	public Object eval(String line, Bindings b) throws ScriptException {
        this.bindings = b;
		return eval(line);
	}


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#getFactory()
	 */
	public ScriptEngineFactory getFactory() {
		return this.factory;
	}


    protected void setFactory(ScriptEngineFactory fact) {
        this.factory = fact;
    }


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#get(java.lang.String)
	 */
	public Object get(String name) {
        try {
            return this.inter.getValue(name);
        }
        catch(JepException e) {
            // can't throw exception. that's real smart of javax.script
            e.printStackTrace();
            return null;
        }
	}


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#put(java.lang.String, java.lang.Object)
	 */
	public void put(String name,
                    Object val) throws IllegalArgumentException {
        try {
            this.inter.set(name, val);
        }
        catch(JepException e) {
            throw new IllegalArgumentException(e);
        }
	}


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#getBindings(int)
	 */
	public Bindings getBindings(int scope) {
        if(scope == ScriptContext.ENGINE_SCOPE)
            return this.bindings;
        else
            return this.globalBindings;
	}


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#setBindings(javax.script.Bindings, int)
	 */
	public void setBindings(Bindings bindings, int scope) {
        if(scope == ScriptContext.ENGINE_SCOPE)
            this.bindings = bindings;
        else
            this.globalBindings = bindings;
	}


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#getContext()
	 */
	public ScriptContext getContext() {
		return this.context;
	}


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#setContext(javax.script.ScriptContext)
	 */
	public void setContext(ScriptContext c) {
        this.context = c;
	}
}
