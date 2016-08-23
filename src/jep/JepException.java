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

/**
 * JepException - it happens.
 * 
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 */
public class JepException extends Exception {

    private static final long serialVersionUID = 1L;

    /**
     * Creates a new <code>JepException</code> instance.
     * 
     */
    public JepException() {
        super();
    }

    /**
     * Creates a new <code>JepException</code> instance.
     * 
     * @param s
     *            a <code>String</code> value
     */
    public JepException(String s) {
        super(s);
    }

    /**
     * Creates a new <code>JepException</code> instance.
     * 
     * @param t
     *            a <code>Throwable</code> value
     */
    public JepException(Throwable t) {
        super(t);
    }

    /**
     * Creates a new <code>JepException</code> instance.
     * 
     * @param s
     *            a <code>String</code> value
     * @param t
     *            a <code>Throwable</code> value
     */
    public JepException(String s, Throwable t) {
        super(s, t);
    }
}
