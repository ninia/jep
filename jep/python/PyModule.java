package jep.python;

import jep.*;


/**
 * <pre>
 * PyModule.java - encapsulates a pointer to a PyModule
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
 * Created: Thu Sep 7 11:52:03 2006
 *
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$
 */
public class PyModule extends PyObject {
    /**
     * Creates a new <code>PyModule</code> instance.
     *
     * @param obj a <code>long</code> value
     * @param jep a <code>Jep</code> value
     */
    public PyModule(long tstate, long obj, Jep jep) {
        super(tstate, obj, jep);
    }
}
