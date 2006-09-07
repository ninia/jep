package jep;

import java.io.Reader;
import java.io.File;
import java.io.IOException;
import java.io.FileWriter;
import java.io.StringReader;

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
public class JepScriptEngine implements ScriptEngine, Compilable {
    private Jep inter = null;

    private Bindings bindings       = new SimpleBindings();
    private Bindings globalBindings = new SimpleBindings();

    private ScriptContext context = null;

    private ScriptEngineFactory factory = null;


    /**
     * Make a new JepScriptEngine
     *
     */
    public JepScriptEngine() throws ScriptException {
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


    // me: lazy
    private void _setContext(ScriptContext c) throws ScriptException {
        try {
            this.inter.set("context", c);
        }
        catch(JepException e) {
            throw new ScriptException(e.getMessage());
        }
    }


    // write temp file from Reader
    private File writeTemp(Reader reader) throws IOException {
        File       temp = File.createTempFile("jep", ".py");
        FileWriter fout = new FileWriter(temp);

        char[] buf = new char[1024];
        int count;

        while((count = reader.read(buf)) > 0)
            fout.write(buf, 0, count);

        fout.close();
        return temp;
    }


    public CompiledScript compile(String script) throws ScriptException {
        File f = null;

        try{
            f = writeTemp(new StringReader(script));
        }
        catch(IOException e) {
            throw new ScriptException("IOException: " + e.getMessage());
        }

        return new JepCompiledScript(f);
    }


    public CompiledScript compile(Reader script) throws ScriptException {
        File f = null;

        try{
            f = writeTemp(script);
        }
        catch(IOException e) {
            throw new ScriptException("IOException: " + e.getMessage());
        }

        return new JepCompiledScript(f);
    }


	/*
     * <pre>
     * Run script from reader.
     *
     * Performance of this method will suck compared to using
     * Jep.runScript(). Use the compiled interface or something.
     *
     * </pre>
     *
	 * @see javax.script.ScriptEngine#eval(java.io.Reader)
	 */
	public Object eval(Reader reader) throws ScriptException {
        try {
            // turn off interactive mode
            this.inter.setInteractive(false);

            File temp = writeTemp(reader);

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
        // the spec says don't do this:
        // this.context = context;
        _setContext(context);
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
        // spec says don't do this:
        // this.bindings = bindings;
		return eval(reader);
	}


	/**
     * Note: always returns null due to Python limitations.
     *
     * (non-Javadoc)
	 * @see javax.script.ScriptEngine#eval(java.lang.String)
	 */
	public Object eval(String line) throws ScriptException {
        return eval(line, this.context, this.bindings);
	}


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#eval(java.lang.String, javax.script.ScriptContext)
	 */
	public Object eval(String line,
                       ScriptContext context) throws ScriptException {
        // spec says don't do that
        // this.context = context;

		return eval(line, context, this.bindings);
	}


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#eval(java.lang.String, javax.script.Bindings)
	 */
	public Object eval(String line, Bindings b) throws ScriptException {
		return eval(line, this.context, b);
	}


	private Object eval(String line,
                        ScriptContext context,
                        Bindings b) throws ScriptException {
        this.inter.setInteractive(true);

        try {
            _setContext(context);
            this.inter.eval(line);
        }
        catch(JepException e) {
            throw new ScriptException(e.getMessage());
        }

		return null;
    }


	/* (non-Javadoc)
	 * @see javax.script.ScriptEngine#getFactory()
	 */
	public ScriptEngineFactory getFactory() {
        if(this.factory == null)
            this.factory = new JepScriptEngineFactory();
		return this.factory;
	}


    /**
     * For internal use.
     *
     */
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
            // can't throw ScriptException. that's awesome
            throw new RuntimeException(e.getMessage());
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
