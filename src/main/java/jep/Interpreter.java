/**
 * Copyright (c) 2019 JEP AUTHORS.
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

import java.util.List;
import java.util.Map;

import jep.python.PyObject;

/**
 * An interface for using a Python Interpreter.
 * 
 * <p>
 * Embeds CPython in Java. Each Interpreter instance provides access to a Python
 * interpreter and maintains an independent global namespace for Python
 * variables. Values can be passed from Java to Python using the
 * {@link #set(String, Object)} method. Various methods, such as
 * {@link #exec(String)} and {@link #invoke(String, Object...)} can be used to
 * execute Python code. Python variables can be accessed using
 * {@link #getValue(String)} and {@link #getValue(String, Class)}.
 * </p>
 * 
 * <p>
 * Methods called on an Interpreter must be called from the same thread that
 * created the instance. To maintain stability, avoid having two Interpreter
 * instances running on the same thread at the same time. Instead provide
 * different threads or close() one before instantiating another on the same
 * thread. Interpreter instances should always be closed when no longer needed
 * to prevent memory leaks.
 * </p>
 * 
 * @since 3.9
 */
public interface Interpreter extends AutoCloseable {

    /**
     * Invokes a Python function.
     * 
     * @param name
     *            a Python function name in globals dict or the name of a global
     *            object and method using dot notation
     * @param args
     *            args to pass to the function in order
     * @return an <code>Object</code> value
     * @throws JepException
     *             if an error occurs
     */
    public Object invoke(String name, Object... args) throws JepException;

    /**
     * Invokes a Python function.
     * 
     * @param name
     *            a Python function name in globals dict or the name of a global
     *            object and method using dot notation
     * @param kwargs
     *            a Map of keyword args
     * @return an {@link Object} value
     * @throws JepException
     *             if an error occurs
     */
    public Object invoke(String name, Map<String, Object> kwargs)
            throws JepException;

    /**
     * Invokes a Python function.
     * 
     * @param name
     *            a Python function name in globals dict or the name of a global
     *            object and method using dot notation
     * @param args
     *            args to pass to the function in order
     * @param kwargs
     *            a Map of keyword args
     * @return an {@link Object} value
     * @throws JepException
     *             if an error occurs
     */
    public Object invoke(String name, Object[] args, Map<String, Object> kwargs)
            throws JepException;

    /**
     * <p>
     * Evaluate Python statements.
     * </p>
     * 
     * <p>
     * In interactive mode, Jep may not immediately execute the given lines of
     * code. In that case, eval() returns false and the statement is stored and
     * is appended to the next incoming string.
     * </p>
     * 
     * <p>
     * If you're running an unknown number of statements, finish with
     * <code>eval(null)</code> to flush the statement buffer.
     * </p>
     * 
     * <p>
     * Interactive mode is slower than a straight eval call since it has to
     * compile the code strings to detect the end of the block. Non-interactive
     * mode is faster, but code blocks must be complete. For example:
     * </p>
     * 
     * <pre>
     * interactive mode == false
     * <code>jep.eval("if(Test):\n    print('Hello world')");</code>
     * </pre>
     * 
     * <pre>
     * interactive mode == true
     * <code>jep.eval("if(Test):");
     * jep.eval("    print('Hello world')");
     * jep.eval(null);
     * </code>
     * </pre>
     * 
     * <p>
     * Also, Python does not readily return object values from eval(). Use
     * {@link #getValue(String)} instead.
     * </p>
     * 
     * <p>
     * Note: Interactive mode will be removed in a future release. This method
     * may still be used for executing individual statements. See console.py for
     * an example of how to interactively execute Python using the builtin
     * compile() and exec() functions.
     * </p>
     *
     * @param str
     *            a <code>String</code> statement to eval
     * @return true if statement complete and was executed.
     * @throws JepException
     *             if an error occurs
     */
    public boolean eval(String str) throws JepException;

    /**
     * Execute an arbitrary number of Python statements in this interpreter.
     * Similar to the Python builtin exec function.
     *
     * @param str
     *            Python code to exececute
     * @throws JepException
     *             if an error occurs
     */
    public void exec(String str) throws JepException;

    /**
     * Runs a Python script.
     * 
     * @param script
     *            a <code>String</code> absolute path to script file.
     * @throws JepException
     *             if an error occurs
     */
    public void runScript(String script) throws JepException;

    /**
     * 
     * <p>
     * Retrieves a value from this Python interpreter. Supports retrieving:
     * <ul>
     * <li>Java objects</li>
     * <li>Python None (null)</li>
     * <li>Python strings</li>
     * <li>Python True and False</li>
     * <li>Python numbers</li>
     * <li>Python lists</li>
     * <li>Python tuples</li>
     * <li>Python dictionaries</li>
     * </ul>
     * 
     * <p>
     * For Python containers, such as lists and dictionaries, getValue will
     * recursively move through the container and convert each item. If the type
     * of the value retrieved is not supported, Jep will fall back to returning
     * a String representation of the object. This fallback behavior will
     * probably change in the future and should not be relied upon.
     * </p>
     * 
     * @param str
     *            the name of the Python variable to get from the interpreter's
     *            global scope
     * @return an <code>Object</code> value
     * @throws JepException
     *             if an error occurs
     */
    public Object getValue(String str) throws JepException;

    /**
     * Like {@link #getValue(String)} but allows specifying the return type. If
     * Jep cannot convert the variable to the specified type then a JepException
     * is thrown. This can be used to safely ensure that the return value is an
     * expected type. The following table describes what conversions are
     * currently possible.
     *
     * <table border="1">
     * <caption>The valid classes for Python to Java conversions</caption>
     * <tr>
     * <th>Python Class</th>
     * <th>Java Classes</th>
     * <th>Notes</th>
     * </tr>
     * <tr>
     * <td>str/unicode</td>
     * <td>{@link String}, {@link Character}</td>
     * <td>Character conversion will fail if the str is longer than 1.</td>
     * </tr>
     * <tr>
     * <td>bool</td>
     * <td>{@link Boolean}</td>
     * </tr>
     * <tr>
     * <td>int/long</td>
     * <td>{@link Long}, {@link Integer}, {@link Short}, {@link Byte}</td>
     * <td>Conversion fails if the number is outside the valid range for the
     * Java type</td>
     * </tr>
     * <tr>
     * <td>float</td>
     * <td>{@link Double}, {@link Float}</td>
     * </tr>
     * <tr>
     * <td>list, tuple</td>
     * <td>{@link List}, array</td>
     * <td>When a tuple is converted to a List it is unmodifiable.</td>
     * </tr>
     * <tr>
     * <td>dict</td>
     * <td>{@link Map}</td>
     * </tr>
     * <tr>
     * <td>function, method</td>
     * <td>Any FunctionalInterface</td>
     * </tr>
     * <tr>
     * <td>Buffer Protocol</td>
     * <td>array</td>
     * <td>This includes Python classes such as bytes, bytearray and
     * array.array</td>
     * </tr>
     * <tr>
     * <td>numpy.ndarray</td>
     * <td>{@link NDArray}</td>
     * <td>Only if Jep was built with numpy support</td>
     * </tr>
     * <tr>
     * <td>numpy.float64</td>
     * <td>{@link Double}, {@link Float}</td>
     * </tr>
     * <tr>
     * <td>numpy.float32</td>
     * <td>{@link Float}, {@link Double}</td>
     * </tr>
     * <tr>
     * <td>numpy.int64</td>
     * <td>{@link Long}, {@link Integer}, {@link Short}, {@link Byte}</td>
     * <td>Conversion fails if the number is outside the valid range for the
     * Java type</td>
     * </tr>
     * <tr>
     * <td>numpy.int32</td>
     * <td>{@link Integer}, {@link Long}, {@link Short}, {@link Byte}</td>
     * <td>Conversion fails if the number is outside the valid range for the
     * Java type</td>
     * </tr>
     * <tr>
     * <td>numpy.int16</td>
     * <td>{@link Short}, {@link Integer}, {@link Long}, {@link Byte}</td>
     * <td>Conversion fails if the number is outside the valid range for the
     * Java type</td>
     * </tr>
     * <tr>
     * <td>numpy.int8</td>
     * <td>{@link Byte}. {@link Short}, {@link Integer}, {@link Long}</td>
     * </tr>
     * <tr>
     * <td>NoneType</td>
     * <td>Any(null)</td>
     * </tr>
     * <tr>
     * <td colspan="3">Jep objects such as PyJObjects and jarrays will be
     * returned if the Java type of the wrapped object is compatible.</td>
     * <tr>
     * <tr>
     * <td>Anything else</td>
     * <td>{@link PyObject}, {@link String}</td>
     * </tr>
     * </table>
     *
     * @param <T>
     *            the generic type of the return type
     * @param str
     *            the name of the Python variable to get from the interpreter's
     *            global scope
     * @param clazz
     *            the Java class of the return type.
     * @return a Java version of the variable
     * @throws JepException
     *             if an error occurs
     */
    public <T> T getValue(String str, Class<T> clazz) throws JepException;

    /**
     * Sets the Java Object into the interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            an <code>Object</code> value
     * @throws JepException
     *             if an error occurs
     */
    public void set(String name, Object v) throws JepException;

    @Override
    public void close() throws JepException;

}
