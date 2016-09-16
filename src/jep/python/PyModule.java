package jep.python;

import jep.Jep;
import jep.JepException;


/**
 * <pre>
 * PyModule.java - encapsulates a pointer to a PyModule
 *
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
 *
 * Created: Thu Sep 7 11:52:03 2006
 *
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 */
public class PyModule extends PyObject {
    /**
     * Creates a new <code>PyModule</code> instance.
     *
     * @param tstate a <code>long</code> value
     * @param obj a <code>long</code> value
     * @param jep a <code>Jep</code> value
     * @exception JepException if an error occurs
     */
    public PyModule(long tstate, long obj, Jep jep) throws JepException {
        super(tstate, obj, jep);
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
        super.isValid();
        return (PyModule) jep.trackObject(new PyModule(
                                              super.tstate,
                                              super.createModule(
                                                  super.tstate,
                                                  super.obj,
                                                  name),
                                              super.jep));
    }


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
        super.isValid();
        return super.getValue(super.tstate, super.obj, str);
    }
}
