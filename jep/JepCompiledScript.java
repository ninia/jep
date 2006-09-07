package jep;

import javax.script.*;
import java.io.File;


/**
 * <pre>
 * JepCompiledScript.java - Embeds CPython in Java.
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
 * Created: Wed Sep 6 18:35:03 2006
 *
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$
 */
public class JepCompiledScript extends CompiledScript {
    File         script = null;
    ScriptEngine engine = new JepScriptEngineFactory().getScriptEngine();


	/**
     * make a new me
     *
	 * @param <code>File</code> script file
	 */
	public JepCompiledScript(File script) {
        this.script = script;
	}


	/* (non-Javadoc)
	 * @see javax.script.CompiledScript#eval(javax.script.ScriptContext)
	 */
	public Object eval(ScriptContext context) throws ScriptException {
        Jep jep = null;
        try {
            jep = new Jep(false);
            jep.set("context", context);

            jep.runScript(this.script.getAbsolutePath());
        }
        catch(JepException e) {
            throw new ScriptException(e.getMessage());
        }
        finally {
            if(jep != null)
                jep.close();
        }

		return null;
	}


	/* (non-Javadoc)
	 * @see javax.script.CompiledScript#getEngine()
	 */
	public ScriptEngine getEngine() {
		return this.engine;
	}

}
