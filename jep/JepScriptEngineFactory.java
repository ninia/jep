package jep;

import java.util.List;
import java.util.ArrayList;

import javax.script.*;


/**
 * <pre>
 * JepScriptEngineFactory.java - Embeds CPython in Java.
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
public class JepScriptEngineFactory implements ScriptEngineFactory {

	/**
     * Describe <code>getEngineName</code> method here.
     *
     * @return a <code>String</code> value
     */
    public String getEngineName() {
		return "jep";
	}


	/**
     * Describe <code>getEngineVersion</code> method here.
     *
     * @return a <code>String</code> value
     */
    public String getEngineVersion() {
		return "2.x";
	}


	/**
     * (non-Javadoc)
	 * @see javax.script.ScriptEngineFactory#getExtensions()
	 */
	public List<String> getExtensions() {
        List<String> l = new ArrayList<String>();
        l.add(".py");
		return l;
	}


	/**
     * Describe <code>getLanguageName</code> method here.
     *
     * @return a <code>String</code> value
     */
    public String getLanguageName() {
		return "CPython";
	}


	/**
     * Describe <code>getLanguageVersion</code> method here.
     *
     * @return a <code>String</code> value
     */
    public String getLanguageVersion() {
		return "Whatever you compiled with";
	}


	/**
     * Describe <code>getMethodCallSyntax</code> method here.
     *
     * @param obj a <code>String</code> value
     * @param m a <code>String</code> value
     * @param args a <code>String[]</code> value
     * @return a <code>String</code> value
     */
    public String getMethodCallSyntax(String obj, String m, String[] args) {
        // copied from javadoc. might be right. *shrugs*

        String ret = obj;
        ret += "." + m + "(";
        for(int i = 0; i < args.length; i++) {
            ret += args[i];
            if (i == args.length - 1)
                ret += ")";
            else
                ret += ",";
        }

        return ret;
	}


	/**
     * (non-Javadoc)
	 * @see javax.script.ScriptEngineFactory#getMimeTypes()
	 */
	public List<String> getMimeTypes() {
        List<String> l = new ArrayList<String>();
        l.add("text/plain");
		return l;
	}


	/**
     * (non-Javadoc)
	 * @see javax.script.ScriptEngineFactory#getNames()
	 */
	public List<String> getNames() {
        List<String> l = new ArrayList<String>();
        l.add("jep");
		return l;
	}


	/**
     * Describe <code>getOutputStatement</code> method here.
     *
     * @param o a <code>String</code> value
     * @return a <code>String</code> value
     */
    public String getOutputStatement(String o) {
        return "print " + o;
	}


	/**
     * Describe <code>getParameter</code> method here.
     *
     * @param p a <code>String</code> value
     * @return an <code>Object</code> value
     */
    public Object getParameter(String p) {
        if(p == null)
            return null;

        // this is fucking retarded
        if(p.equals(ScriptEngine.ENGINE))
            return getEngineName();

        if(p.equals(ScriptEngine.ENGINE_VERSION))
            return getEngineVersion();

        if(p.equals(ScriptEngine.NAME))
            return "jep";

        if(p.equals(ScriptEngine.LANGUAGE))
            return getLanguageName();

        if(p.equals(ScriptEngine.LANGUAGE_VERSION))
            return getLanguageVersion();

		return null;
	}


	/**
     * Describe <code>getProgram</code> method here.
     *
     * @param lines a <code>String[]</code> value
     * @return a <code>String</code> value
     */
    public String getProgram(String[] lines) {
        StringBuffer ret = new StringBuffer();

        for(int i = 0; i < lines.length; i++) {
            ret.append(lines[i]);
            ret.append("\n");
        }

		return ret.toString();
	}


	/**
     * Describe <code>getScriptEngine</code> method here.
     *
     * @return a <code>ScriptEngine</code> value
     */
    public ScriptEngine getScriptEngine() {
        try {
            JepScriptEngine e = new JepScriptEngine();
            e.setFactory(this);
            return e;
        }
        catch(ScriptException e) {
            // aint this grand.
            // we can throw it in the constructor, but not here.
            throw new RuntimeException(e);
        }
	}

}
