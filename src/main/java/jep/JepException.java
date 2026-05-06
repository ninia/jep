/**
 * Copyright (c) 2004-2022 JEP AUTHORS.
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
public class JepException extends RuntimeException {

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
     * The Python traceback formatted by the <code>traceback</code> module, or
     * <code>null</code> if no traceback was available (e.g. the exception was not
     * thrown as a result of a native Python exception or was not surfaced by a Python
     * call, or formatting itself failed). Formatted like a normal multi-line Python
     * traceback, suitable for direct logging.
     *
     */
    private final String pythonTraceback;

    /**
     * Creates a new <code>JepException</code> instance.
     * 
     */
    public JepException() {
        super();
        this.pythonType = 0;
        this.pythonTraceback = null;
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
        this.pythonTraceback = null;
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
            this.pythonTraceback = j.pythonTraceback;
        } else {
            this.pythonType = 0;
            this.pythonTraceback = null;
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
            this.pythonTraceback = j.pythonTraceback;
        } else {
            this.pythonType = 0;
            this.pythonTraceback = null;
        }
    }

    /**
     * Construct with the address of a python exception type. This is for
     * internal use only.
     *
     * @param s
     *            error message
     * @param pythonType
     *            the address of the type of the python exception that triggered
     *            this exception
     */
    protected JepException(String s, long pythonType) {
        super(s);
        this.pythonType = pythonType;
        this.pythonTraceback = null;
    }

    /**
     * Construct with the address of a Python exception type and a formatted Python
     * traceback. This is for internal use only.
     *
     * @param s
     *            error message
     * @param pythonType
     *            the address of the type of the python exception that triggered
     *            this exception
     * @param pythonTraceback
     *            the python traceback of the python exception that triggered
     *            this exception
     */
    protected JepException(String s, long pythonType, String pythonTraceback) {
        super(s);
        this.pythonType = pythonType;
        this.pythonTraceback = pythonTraceback;
    }

    /**
     * Construct with a Throwable cause and a formatted Python traceback. This
     * is for internal use only.
     * @param s
     *            error message
     * @param t
     *            the <code>Throwable</code> that caused this exception
     * @param pythonTraceback
     *            the python traceback of the python exception that triggered
     *            this exception
     */
    protected JepException(String s, Throwable t, String pythonTraceback) {
        super(s, t);
        if (t instanceof JepException) {
            JepException j = (JepException) t;
            this.pythonType = j.pythonType;
        } else {
            this.pythonType = 0;
        }
        this.pythonTraceback = pythonTraceback;
    }

    /**
     * Returns the Python traceback as formatted by Python's <code>traceback</code>
     * module, or <code>null</code> if no Python traceback is available. The string
     * matches what <code>traceback.print_exc()</code> would print in pure Python,
     * including source line text and PEP 657 carets, where applicable.
     *
     * @return The formatted Python traceback, or {@code null}
     */
    public String getPythonTraceback() {
        return pythonTraceback;
    }

    /**
     * Get the address of the python exception type that triggered this
     * exceptions. This is for internal use only.
     *
     * @return the address of tthe triggering python exception type
     */
    protected long getPythonType() {
        return pythonType;
    }

}
