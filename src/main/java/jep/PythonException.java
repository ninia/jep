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

import jep.python.PyObject;

/**
 * A {@link JepException} thrown whenever a Python exception is active, whether
 * the root cause was Python code or a Java exception that surfaced through
 * Python. Carries the Python exception instance as a {@link PyObject} proxy,
 * allowing callers to inspect attributes (e.g. {@code .args}, {@code __traceback__},
 * custom fields) on the original Python exception object.
 *
 * <p>The wrapped {@link PyObject} is subject to the same constraints as all
 * Jep proxy objects: it is only valid while the originating {@link Jep}
 * interpreter is open and on the thread that owns it. This exception will
 * always be thrown inside the scope of the Interpreter.
 *
 * @author Michael Quindt
 */
public class PythonException extends JepException {

    private static final long serialVersionUID = 1L;

    /**
     * The original Python exception instance, or {@code null} if it could not
     * be captured (e.g. interpreter state was unavailable at throw time).
     */
    private final PyObject pythonException;

    /**
     * The formatted Python traceback as a string, or {@code null} if it could
     * not be computed at throw time.
     */
    private final String pythonTraceback;

    /**
     * Called from native code for pure-Python exceptions (no Java cause).
     *
     * @param s
     *            error message
     * @param pythonType
     *            address of the Python exception type
     * @param pythonException
     *            the Python exception instance, or {@code null}
     * @param pythonTraceback
     *            formatted traceback string, or {@code null}
     */
    protected PythonException(String s, long pythonType, PyObject pythonException,
            String pythonTraceback) {
        super(s, pythonType);
        this.pythonException = pythonException;
        this.pythonTraceback = pythonTraceback;
    }

    /**
     * Called from native code when a Java exception surfaces through Python.
     * Preserves the Java exception as the cause while carrying the Python
     * exception object that wraps it.
     *
     * @param s
     *            error message
     * @param cause
     *            the originating Java exception
     * @param pythonException
     *            the Python exception instance wrapping the Java cause, or {@code null}
     * @param pythonTraceback
     *            formatted traceback string, or {@code null}
     */
    protected PythonException(String s, Throwable cause, PyObject pythonException,
            String pythonTraceback) {
        super(s, cause);
        this.pythonException = pythonException;
        this.pythonTraceback = pythonTraceback;
    }

    /**
     * Returns the original Python exception instance, or {@code null} if it
     * was not available when the exception was thrown.
     *
     * @return the Python exception as a {@link PyObject}, or {@code null}
     */
    public PyObject getPythonException() {
        return pythonException;
    }

    /**
     * Returns the formatted Python traceback as produced by
     * {@code traceback.format_exception()}, or {@code null} if it was not
     * available when the exception was thrown.
     *
     * @return the formatted traceback string, or {@code null}
     */
    public String getPythonTraceback() {
        return pythonTraceback;
    }
}
