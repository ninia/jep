/**
 * Copyright (c) 2017 JEP AUTHORS.
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
 * @author Mike Johnson
 */
public class JepException extends Exception {

    private static final long serialVersionUID = 1L;

    /**
     * The address of the Python type which caused this exception. This is used
     * if the JepException is thrown back into Python so that a new Python
     * exception can be thrown with the same type as the original exception.
     * 
     * @since 3.7
     */
    private final long pythonType;

    /**
     * Creates a new <code>JepException</code> instance.
     * 
     */
    public JepException() {
        super();
        this.pythonType = 0;
    }

    /**
     * Creates a new <code>JepException</code> instance.
     * 
     * @param s
     *            a <code>String</code> value
     */
    public JepException(String s) {
        super(s);
        this.pythonType = 0;
    }

    /**
     * Creates a new <code>JepException</code> instance.
     * 
     * @param t
     *            a <code>Throwable</code> value
     */
    public JepException(Throwable t) {
        super(t);
        if (t instanceof JepException) {
            JepException j = (JepException) t;
            this.pythonType = j.pythonType;
        } else {
            this.pythonType = 0;

        }
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
        if (t instanceof JepException) {
            JepException j = (JepException) t;
            this.pythonType = j.pythonType;
        } else {
            this.pythonType = 0;
        }
    }

    /**
     * Construct with the address of a python exception type. This is for
     * internal use only.
     */
    protected JepException(String s, long pythonType) {
        super(s);
        this.pythonType = pythonType;
    }

    /**
     * Get the address of the python exception type that triggered this
     * exceptions. This is for internal use only.
     */
    protected long getPythonType() {
        return pythonType;
    }

}
