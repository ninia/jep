/**
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
 */
package jep;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Reader;

import javax.script.Bindings;
import javax.script.ScriptContext;
import javax.script.ScriptEngine;
import javax.script.ScriptEngineFactory;
import javax.script.ScriptException;
import javax.script.SimpleBindings;

import jep.python.PyModule;

/**
 * Implements {@link javax.script.ScriptEngine}
 * 
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 */
public class JepScriptEngine implements ScriptEngine {
    private Jep jep = null;

    private Bindings bindings = new SimpleBindings();

    private Bindings globalBindings = new SimpleBindings();

    private ScriptContext context = null;

    private ScriptEngineFactory factory = null;

    /**
     * Make a new JepScriptEngine
     * 
     * @throws ScriptException
     *             if an error occurs
     */
    public JepScriptEngine() throws ScriptException {
        try {
            this.jep = new Jep(true); // make interactive because javax.script
                                      // sucks
            this.jep.setClassLoader(Thread.currentThread()
                    .getContextClassLoader());
        } catch (JepException e) {
            throw (ScriptException) new ScriptException(e.getMessage())
                    .initCause(e);
        }
    }

    /**
     * Describe <code>createBindings</code> method here.
     * 
     * @see javax.script.ScriptEngine#createBindings()
     * 
     * @return a <code>Bindings</code> value
     */
    @Override
    public Bindings createBindings() {
        return new SimpleBindings();
    }

    // me: lazy
    private void _setContext(ScriptContext c) throws ScriptException {
        try {
            this.jep.set("context", c);
        } catch (JepException e) {
            throw (ScriptException) new ScriptException(e.getMessage())
                    .initCause(e);
        }
    }

    // write temp file from Reader
    private File writeTemp(Reader reader) throws IOException {
        File temp = File.createTempFile("jep", ".py");
        FileWriter fout = new FileWriter(temp);

        char[] buf = new char[1024];
        int count;

        while ((count = reader.read(buf)) > 0)
            fout.write(buf, 0, count);

        fout.close();
        return temp;
    }

    private Object eval(Reader reader, ScriptContext context, Bindings bindings)
            throws ScriptException {
        try {
            // make sure to always set a context, even if null (None)
            _setContext(context);

            // turn off interactive mode
            this.jep.setInteractive(false);

            File temp = writeTemp(reader);

            // okay sic jep on it
            this.jep.runScript(temp.getAbsolutePath());
        } catch (IOException e) {
            throw new ScriptException("Error writing to file: "
                    + e.getMessage());
        } catch (JepException e) {
            throw (ScriptException) new ScriptException(e.getMessage())
                    .initCause(e);
        }

        return null;
    }

    /**
     * <pre>
     * Run script from reader.
     * 
     * Performance of this method will suck compared to using
     * Jep.runScript(). Use the compiled interface or something.
     * 
     * </pre>
     * 
     * @param reader
     *            a <code>Reader</code> value
     * @return an <code>Object</code> value
     * @exception ScriptException
     *                if an error occurs
     */
    @Override
    public Object eval(Reader reader) throws ScriptException {
        return eval(reader, this.context, this.bindings);
    }

    /**
     * Note: always returns null due to Python limitations.
     * 
     * (non-Javadoc)
     * 
     * @param reader
     *            a <code>Reader</code> value
     * @param context
     *            a <code>ScriptContext</code> value
     * @return an <code>Object</code> value
     * @exception ScriptException
     *                if an error occurs
     * @see javax.script.ScriptEngine#eval(java.io.Reader,
     *      javax.script.ScriptContext)
     */
    @Override
    public Object eval(Reader reader, ScriptContext context)
            throws ScriptException {
        // the spec says don't do this:
        // this.context = context;
        return eval(reader, context, this.bindings);
    }

    /**
     * Note: always returns null due to Python limitations.
     * 
     * (non-Javadoc)
     * 
     * @param reader
     *            a <code>Reader</code> value
     * @param bindings
     *            a <code>Bindings</code> value
     * @return an <code>Object</code> value
     * @exception ScriptException
     *                if an error occurs
     * @see javax.script.ScriptEngine#eval(java.io.Reader,
     *      javax.script.Bindings)
     */
    @Override
    public Object eval(Reader reader, Bindings bindings) throws ScriptException {
        // spec says don't do this:
        // this.bindings = bindings;
        return eval(reader, this.context, bindings);
    }

    // -------------------------------------------------- string evals

    /**
     * Note: always returns null due to Python limitations.
     * 
     * (non-Javadoc)
     * 
     * @param line
     *            a <code>String</code> value
     * @return an <code>Object</code> value
     * @exception ScriptException
     *                if an error occurs
     * @see javax.script.ScriptEngine#eval(java.lang.String)
     */
    @Override
    public Object eval(String line) throws ScriptException {
        return eval(line, this.context, this.bindings);
    }

    /**
     * Describe <code>eval</code> method here.
     * 
     * @param line
     *            a <code>String</code> value
     * @param context
     *            a <code>ScriptContext</code> value
     * @return an <code>Object</code> value
     * @exception ScriptException
     *                if an error occurs
     */
    @Override
    public Object eval(String line, ScriptContext context)
            throws ScriptException {
        // spec says don't do that
        // this.context = context;

        return eval(line, context, this.bindings);
    }

    /**
     * Describe <code>eval</code> method here.
     * 
     * @param line
     *            a <code>String</code> value
     * @param b
     *            a <code>Bindings</code> value
     * @return an <code>Object</code> value
     * @exception ScriptException
     *                if an error occurs
     */
    @Override
    public Object eval(String line, Bindings b) throws ScriptException {
        return eval(line, this.context, b);
    }

    private Object eval(String line, ScriptContext context, Bindings b)
            throws ScriptException {
        this.jep.setInteractive(true);

        try {
            _setContext(context);
            this.jep.eval(line);
        } catch (JepException e) {
            throw (ScriptException) new ScriptException(e.getMessage())
                    .initCause(e);
        }

        return null;
    }

    /**
     * Describe <code>getFactory</code> method here.
     * 
     * @return a <code>ScriptEngineFactory</code> value
     */
    @Override
    public ScriptEngineFactory getFactory() {
        if (this.factory == null)
            this.factory = new JepScriptEngineFactory();
        return this.factory;
    }

    /**
     * For internal use.
     * 
     * @param fact
     *            a <code>ScriptEngineFactory</code> value
     */
    protected void setFactory(ScriptEngineFactory fact) {
        this.factory = fact;
    }

    /**
     * Describe <code>get</code> method here.
     * 
     * @param name
     *            a <code>String</code> value
     * @return an <code>Object</code> value
     */
    @Override
    public Object get(String name) {
        try {
            PyModule module = null;
            String[] tokens = null;

            if (name.indexOf('.') > 0) {
                // split package name by '.' and make modules
                tokens = name.split("\\.");
                for (int i = 0; i < tokens.length - 1; i++) {
                    if (module == null)
                        module = jep.createModule(tokens[i]);
                    else
                        module = module.createModule(tokens[i]);
                }
            }

            if (module == null)
                return this.jep.getValue(name);
            else
                return module.getValue(tokens[tokens.length - 1]);
        } catch (JepException e) {
            // probably not found. javax.script wants use to just return null
            return null;
        }
    }

    /**
     * Describe <code>put</code> method here.
     * 
     * @param name
     *            a <code>String</code> value
     * @param val
     *            an <code>Object</code> value
     * @exception IllegalArgumentException
     *                if an error occurs
     */
    @Override
    public void put(String name, Object val) throws IllegalArgumentException {
        try {
            PyModule module = null;
            String[] tokens = null;
            String mname = null;

            if (name.indexOf('.') > 0) {
                // split package name by '.' and make modules
                tokens = name.split("\\.");
                for (int i = 0; i < tokens.length - 1; i++) {
                    mname = tokens[i];
                    if (module == null)
                        module = jep.createModule(mname);
                    else
                        module = module.createModule(mname);
                }
            }

            if (module == null)
                this.jep.set(name, val);
            else
                module.set(tokens[tokens.length - 1], val);
        } catch (JepException e) {
            throw new IllegalArgumentException(e);
        }
    }

    /**
     * Describe <code>getBindings</code> method here.
     * 
     * @param scope
     *            an <code>int</code> value
     * @return a <code>Bindings</code> value
     */
    @Override
    public Bindings getBindings(int scope) {
        if (scope == ScriptContext.ENGINE_SCOPE)
            return this.bindings;
        else
            return this.globalBindings;
    }

    /**
     * Describe <code>setBindings</code> method here.
     * 
     * @param bindings
     *            a <code>Bindings</code> value
     * @param scope
     *            an <code>int</code> value
     */
    @Override
    public void setBindings(Bindings bindings, int scope) {
        if (scope == ScriptContext.ENGINE_SCOPE)
            this.bindings = bindings;
        else
            this.globalBindings = bindings;
    }

    /**
     * Describe <code>getContext</code> method here.
     * 
     * @return a <code>ScriptContext</code> value
     */
    @Override
    public ScriptContext getContext() {
        return this.context;
    }

    /**
     * Describe <code>setContext</code> method here.
     * 
     * @param c
     *            a <code>ScriptContext</code> value
     */
    @Override
    public void setContext(ScriptContext c) {
        this.context = c;
        try {
            _setContext(c);
        } catch (ScriptException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * You *must* close this
     * 
     */
    public void close() {
        this.jep.close();
    }
}
