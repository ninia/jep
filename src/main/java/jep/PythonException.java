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
 * A {@link JepException} caused by a native Python exception in Python code.
 * Carries the Python exception instance as a {@link PyObject} proxy, allowing
 * callers to inspect attributes (e.g. {@code .args}, custom fields) on the
 * original exception.
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
     * Called from native code for pure-Python exceptions.
     *
     * @param s
     *            error message
     * @param pythonType
     *            address of the Python exception type
     * @param pythonTraceback
     *            formatted traceback string, or {@code null}
     * @param pythonException
     *            the Python exception instance, or {@code null}
     */
    protected PythonException(String s, long pythonType, String pythonTraceback,
            PyObject pythonException) {
        super(s, pythonType, pythonTraceback);
        this.pythonException = pythonException;
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
}
