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

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import jep.python.PyModule;
import jep.python.PyObject;

/**
 * <p>
 * Embeds CPython in Java. Each Jep instance can be considered a Python
 * sub-intepreter, mostly sandboxed from other Jep instances. However, it is not
 * guaranteed to be completely sandboxed, as one sub-interpreter may be able to
 * affect another when using CPython extensions or referencing the same Java
 * objects in different sub-interpreters.
 * </p>
 * 
 * <p>
 * In general, methods called on a Jep instance must be called from the same
 * thread that created the instance. To maintain stability, avoid having two Jep
 * instances running on the same thread at the same time. Instead provide
 * different threads or close() one before instantiating another on the same
 * thread. Jep instances should always be closed when no longer needed to
 * prevent memory leaks.
 * </p>
 * 
 * @author Mike Johnson
 */
public final class Jep implements AutoCloseable {

    private static final String THREAD_WARN = "JEP THREAD WARNING: ";

    private boolean closed = false;

    private long tstate = 0;

    // all calls must originate from same thread
    private Thread thread = null;

    // used by default if not passed/set by caller
    private ClassLoader classLoader = null;

    // eval() storage.
    private StringBuilder evalLines = null;

    private boolean interactive = false;

    // windows requires this as unix newline...
    private static final String LINE_SEP = "\n";

    /*
     * keep track of objects that we create. do this to prevent crashes in
     * userland if jep is closed.
     */
    private final List<PyObject> pythonObjects = new ArrayList<>();

    /**
     * Tracks if this thread has been used for an interpreter before. Using
     * different interpreter instances on the same thread is iffy at best. If
     * you make use of CPython extensions (e.g. numpy) that use the GIL, then
     * this gets even more risky and can potentially deadlock.
     */
    private final static ThreadLocal<Boolean> threadUsed = new ThreadLocal<Boolean>() {
        @Override
        protected Boolean initialValue() {
            return false;
        }
    };

    /**
     * Sets interpreter settings for the top Python interpreter. This method
     * must be called before the first Jep instance is created in the process.
     * 
     * @param config
     *            the python configuration to use.
     * 
     * @throws JepException
     *             if an error occurs
     * @deprecated Please use {@link #MainInterpreter.setInitParams(PyConfig)}
     *             instead.
     * @since 3.6
     */
    @Deprecated
    public static void setInitParams(PyConfig config) throws JepException {
        MainInterpreter.setInitParams(config);
    }

    /**
     * Sets the sys.argv values on the top interpreter. This is a workaround for
     * issues with shared modules and should be considered experimental.
     * 
     * @param argv
     *            the arguments to be set on Python's sys.argv for the top/main
     *            interpreter
     * @throws JepException
     *             if an error occurs
     * @deprecated Please use
     *             {@link #MainInterpreter.setSharedModulesArgv(String...)}
     *             instead.
     * 
     * @since 3.7
     */
    @Deprecated
    public static void setSharedModulesArgv(String... argv)
            throws JepException {
        MainInterpreter.setSharedModulesArgv(argv);
    }

    // -------------------------------------------------- constructors

    /**
     * Creates a new <code>Jep</code> instance and its associated
     * sub-interpreter.
     * 
     * @exception JepException
     *                if an error occurs
     */
    public Jep() throws JepException {
        this(new JepConfig());
    }

    /**
     * Creates a new <code>Jep</code> instance and its associated
     * sub-interpreter.
     * 
     * @param interactive
     *            whether {@link #eval(String)} should support the slower
     *            behavior of potentially waiting for multiple statements
     * @exception JepException
     *                if an error occurs
     * @deprecated Please use {@link #Jep(JepConfig)} instead.
     */
    @Deprecated
    public Jep(boolean interactive) throws JepException {
        this(interactive, null, null);
    }

    /**
     * Creates a new <code>Jep</code> instance and its associated
     * sub-interpreter.
     * 
     * @param interactive
     *            whether {@link #eval(String)} should support the slower
     *            behavior of potentially waiting for multiple statements
     * @param includePath
     *            a path of directories separated by File.pathSeparator that
     *            will be appended to the sub-intepreter's <code>sys.path</code>
     * @exception JepException
     *                if an error occurs
     * @deprecated Please use {@link #Jep(JepConfig)} instead.
     */
    @Deprecated
    public Jep(boolean interactive, String includePath) throws JepException {
        this(interactive, includePath, null);
    }

    /**
     * Creates a new <code>Jep</code> instance and its associated
     * sub-interpreter.
     * 
     * @param interactive
     *            whether {@link #eval(String)} should support the slower
     *            behavior of potentially waiting for multiple statements
     * @param includePath
     *            a path of directories separated by File.pathSeparator that
     *            will be appended to the sub-intepreter's <code>sys.path</code>
     * @param cl
     *            the ClassLoader to use when importing Java classes from Python
     * @exception JepException
     *                if an error occurs
     * @deprecated Please use {@link #Jep(JepConfig)} instead.
     */
    @Deprecated
    public Jep(boolean interactive, String includePath, ClassLoader cl)
            throws JepException {
        this(interactive, includePath, cl, null);
    }

    /**
     * Creates a new <code>Jep</code> instance and its associated
     * sub-interpreter.
     * 
     * @param interactive
     *            whether {@link #eval(String)} should support the slower
     *            behavior of potentially waiting for multiple statements
     * @param includePath
     *            a path of directories separated by File.pathSeparator that
     *            will be appended to the sub-intepreter's <code>sys.path</code>
     * @param cl
     *            the ClassLoader to use when importing Java classes from Python
     * @param ce
     *            a <code>ClassEnquirer</code> to determine which imports are
     *            Python vs Java, or null for the default {@link ClassList}
     * @exception JepException
     *                if an error occurs
     * @deprecated Please use {@link #Jep(JepConfig)} instead.
     */
    @Deprecated
    public Jep(boolean interactive, String includePath, ClassLoader cl,
            ClassEnquirer ce) throws JepException {
        this(new JepConfig().setInteractive(interactive)
                .setIncludePath(includePath).setClassLoader(cl)
                .setClassEnquirer(ce));
    }

    public Jep(JepConfig config) throws JepException {
        MainInterpreter mainInterpreter = MainInterpreter.getMainInterpreter();
        if (threadUsed.get()) {
            Thread current = Thread.currentThread();
            StringBuilder warning = new StringBuilder(THREAD_WARN)
                    .append("Unsafe reuse of thread ").append(current.getName())
                    .append(" for another Python sub-interpreter.\n")
                    .append("Please close() the previous Jep instance to ensure stability.");
            throw new JepException(warning.toString());
        }

        if (config.classLoader == null) {
            this.classLoader = this.getClass().getClassLoader();
        } else {
            this.classLoader = config.classLoader;
        }

        boolean hasSharedModules = config.sharedModules != null
                && !config.sharedModules.isEmpty();

        this.interactive = config.interactive;
        this.tstate = init(this.classLoader, hasSharedModules);
        threadUsed.set(true);
        this.thread = Thread.currentThread();

        // why write C code if you don't have to? :-)
        if (config.includePath != null) {
            String includePath = config.includePath.toString();

            // Added for compatibility with Windows file system
            if (includePath.contains("\\")) {
                includePath = includePath.replace("\\", "\\\\");
            }

            eval("import sys");
            eval("sys.path += '" + includePath + "'.split('"
                    + File.pathSeparator + "')");
        }

        eval("import jep");
        if (hasSharedModules) {
            set("sharedModules", config.sharedModules);
            set("sharedImporter", mainInterpreter);
            eval("jep.shared_modules_hook.setupImporter(sharedModules,sharedImporter)");
            eval("del sharedModules");
            eval("del sharedImporter");
            eval(null); // flush
        }
        ClassEnquirer ce = config.classEnquirer;
        if (ce == null) {
            ce = ClassList.getInstance();
        }
        set("classlist", ce);
        eval("jep.java_import_hook.setupImporter(classlist)");
        eval("del classlist");
        eval(null); // flush

        if (config.redirectOutputStreams) {
            eval("from jep import redirect_streams");
            eval("redirect_streams.setup()");
            eval(null); // flush
        }
    }

    private native long init(ClassLoader classloader, boolean hasSharedModules)
            throws JepException;

    /**
     * Checks if the current thread is valid for the method call. All calls must
     * check the thread.
     * 
     * <b>Internal Only</b>
     * 
     * @exception JepException
     *                if an error occurs
     */
    public void isValidThread() throws JepException {
        if (this.thread != Thread.currentThread())
            throw new JepException("Invalid thread access.");
        if (this.closed)
            throw new JepException("Jep instance has been closed.");
        if (this.tstate == 0)
            throw new JepException("Initialization failed.");
    }

    /**
     * Runs a Python script.
     * 
     * @param script
     *            a <code>String</code> absolute path to script file.
     * @exception JepException
     *                if an error occurs
     */
    public void runScript(String script) throws JepException {
        runScript(script, null);
    }

    /**
     * Runs a Python script.
     * 
     * @param script
     *            a <code>String</code> absolute path to script file.
     * @param cl
     *            a <code>ClassLoader</code> value, may be null.
     * @exception JepException
     *                if an error occurs
     */
    public void runScript(String script, ClassLoader cl) throws JepException {
        isValidThread();

        if (script == null)
            throw new JepException("Script filename cannot be null.");

        File file = new File(script);
        if (!file.exists() || !file.canRead())
            throw new JepException("Invalid file: " + file.getAbsolutePath());

        setClassLoader(cl);
        run(this.tstate, script);
    }

    private native void run(long tstate, String script) throws JepException;

    /**
     * Invokes a Python function.
     * 
     * @param name
     *            must be a valid Python function name in globals dict
     * @param args
     *            args to pass to the function in order
     * @return an <code>Object</code> value
     * @exception JepException
     *                if an error occurs
     */
    public Object invoke(String name, Object... args) throws JepException {
        isValidThread();
        if (name == null || name.trim().equals("")) {
            throw new JepException("Invalid function name.");
        }

        return invoke(this.tstate, name, args, null);
    }

    /**
     * Invokes a Python function.
     * 
     * @param name
     *            must be a valid Python function name in globals dict
     * @param kwargs
     *            a Map of keyword args
     * @return an <Object> value
     * @throws JepException
     * @since 3.8
     */
    public Object invoke(String name, Map<String, Object> kwargs)
            throws JepException {
        isValidThread();
        if (name == null || name.trim().equals("")) {
            throw new JepException("Invalid function name.");
        }

        return invoke(this.tstate, name, null, kwargs);
    }

    /**
     * Invokes a Python function.
     * 
     * @param name
     *            must be a valid Python function name in globals dict
     * @param args
     *            args to pass to the function in order
     * @param kwargs
     *            a Map of keyword args
     * @return an <Object> value
     * @throws JepException
     * @since 3.8
     */
    public Object invoke(String name, Object[] args, Map<String, Object> kwargs)
            throws JepException {
        isValidThread();
        if (name == null || name.trim().equals("")) {
            throw new JepException("Invalid function name.");
        }

        return invoke(this.tstate, name, args, kwargs);
    }

    private native Object invoke(long tstate, String name, Object[] args,
            Map<String, Object> kwargs);

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
     * @param str
     *            a <code>String</code> statement to eval
     * @return true if statement complete and was executed.
     * @exception JepException
     *                if an error occurs
     */
    public boolean eval(String str) throws JepException {
        isValidThread();

        try {
            // trim windows \r\n
            if (str != null) {
                str = str.replaceAll("\r", "");
            }

            if (str == null || str.trim().equals("")) {
                if (!this.interactive) {
                    return false;
                }
                if (this.evalLines == null) {
                    return true; // nothing to eval
                }

                // null means we send lines, whether or not it compiles.
                eval(this.tstate, this.evalLines.toString());
                this.evalLines = null;
                return true;
            }

            // first check if it compiles by itself
            if (!this.interactive || (this.evalLines == null
                    && compileString(this.tstate, str) == 1)) {
                eval(this.tstate, str);
                return true;
            }

            // doesn't compile on it's own, append to eval
            if (this.evalLines == null) {
                this.evalLines = new StringBuilder();
            } else {
                evalLines.append(LINE_SEP);
            }
            evalLines.append(str);

            return false;
        } catch (JepException e) {
            this.evalLines = null;
            throw e;
        }
    }

    private native int compileString(long tstate, String str)
            throws JepException;

    private native void eval(long tstate, String str) throws JepException;

    /**
     * 
     * <p>
     * Retrieves a value from this Python sub-interpreter. Supports retrieving:
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
     * <pre>
     * Python is pretty picky about what it accepts here. The general syntax
     * <code>
     * jep.eval("a = 5");
     * String a = (String) jep.getValue("a");
     * </code>
     * will work.
     * </pre>
     * 
     * @param str
     *            the name of the Python variable to get from the
     *            sub-interpreter's global scope
     * @return an <code>Object</code> value
     * @exception JepException
     *                if an error occurs
     */
    public Object getValue(String str) throws JepException {
        isValidThread();

        return getValue(this.tstate, str);
    }

    private native Object getValue(long tstate, String str) throws JepException;

    /**
     * Retrieves a Python string object as a Java byte[].
     * 
     * @param str
     *            the name of the Python variable to get from the
     *            sub-interpreter's global scope
     * @return an <code>Object</code> array
     * @exception JepException
     *                if an error occurs
     */
    public byte[] getValue_bytearray(String str) throws JepException {
        isValidThread();

        return getValue_bytearray(this.tstate, str);
    }

    private native byte[] getValue_bytearray(long tstate, String str)
            throws JepException;

    /**
     * Track Python objects we create so they can be smoothly shutdown with no
     * risk of crashes due to bad reference counting.
     * 
     * <b>Internal use only.</b>
     * 
     * @param obj
     *            a <code>PyObject</code> value
     * @return same object, for inlining stuff
     * @exception JepException
     *                if an error occurs
     */
    public PyObject trackObject(PyObject obj) throws JepException {
        return trackObject(obj, true);
    }

    /**
     * Track Python objects we create so they can be smoothly shutdown with no
     * risk of crashes due to bad reference counting.
     * 
     * <b>Internal use only.</b>
     * 
     * @param obj
     *            a <code>PyObject</code> value
     * @param inc
     *            should trackObject incref()
     * @return same object, for inlining stuff
     * @exception JepException
     *                if an error occurs
     */
    public PyObject trackObject(PyObject obj, boolean inc) throws JepException {
        // make sure python doesn't close it
        if (inc)
            obj.incref();

        this.pythonObjects.add(obj);
        return obj;
    }

    /**
     * Create a Python module on the interpreter. If the given name is valid,
     * imported module, this method will return that module.
     * 
     * @deprecated This will be removed in a future release.
     * @param name
     *            a <code>String</code> value
     * @return a <code>PyModule</code> value
     * @exception JepException
     *                if an error occurs
     */
    @Deprecated
    public PyModule createModule(String name) throws JepException {
        return (PyModule) trackObject(new PyModule(this.tstate,
                createModule(this.tstate, name), this));
    }

    private native long createModule(long tstate, String name)
            throws JepException;

    // -------------------------------------------------- set things

    /**
     * Sets the default classloader.
     * 
     * @param cl
     *            a <code>ClassLoader</code> value
     */
    public void setClassLoader(ClassLoader cl) {
        if (cl != null && cl != this.classLoader) {
            this.classLoader = cl;
            // call native set
            setClassLoader(this.tstate, cl);
        }
    }

    private native void setClassLoader(long tstate, ClassLoader cl);

    /**
     * Changes behavior of {@link #eval(String)}. Interactive mode can wait for
     * further Python statements to be evaled, while non-interactive mode can
     * only execute complete Python statements.
     * 
     * @param v
     *            if the sub-interpreter should run in interactive mode
     */
    public void setInteractive(boolean v) {
        this.interactive = v;
    }

    /**
     * Gets whether or not this sub-interpreter is interactive.
     * 
     * @return whether or not the sub-interpreter is interactive
     */
    public boolean isInteractive() {
        return this.interactive;
    }

    /**
     * Sets the Java Object into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            an <code>Object</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, Object v) throws JepException {
        isValidThread();

        if (v instanceof Class) {
            set(tstate, name, (Class<?>) v);
        } else if (v instanceof String) {
            set(name, ((String) v));
        } else if (v instanceof Float) {
            set(name, ((Float) v).floatValue());
        } else if (v instanceof Integer) {
            set(name, ((Integer) v).intValue());
        } else if (v instanceof Double) {
            set(name, ((Double) v).doubleValue());
        } else if (v instanceof Long) {
            set(name, ((Long) v).longValue());
        } else if (v instanceof Byte) {
            set(name, ((Byte) v).byteValue());
        } else if (v instanceof Short) {
            set(name, ((Short) v).shortValue());
        } else if (v instanceof Boolean) {
            set(name, ((Boolean) v).booleanValue());
        } else {
            set(tstate, name, v);
        }
    }

    private native void set(long tstate, String name, Object v)
            throws JepException;

    private native void set(long tstate, String name, Class<?> v)
            throws JepException;

    /**
     * Sets the Java String into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>String</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, String v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, String v)
            throws JepException;

    /**
     * Sets the Java boolean into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>boolean</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, boolean v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, boolean v)
            throws JepException;

    /**
     * Sets the Java int into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            an <code>int</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, int v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    /**
     * Sets the Java short into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            an <code>int</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, short v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, int v)
            throws JepException;

    /**
     * Sets the Java char[] into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>char[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, char[] v) throws JepException {
        isValidThread();

        set(tstate, name, new String(v));
    }

    /**
     * Sets the Java char into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>char</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, char v) throws JepException {
        isValidThread();

        set(tstate, name, new String(new char[] { v }));
    }

    /**
     * Sets the Java byte into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param b
     *            a <code>byte</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, byte b) throws JepException {
        isValidThread();

        set(tstate, name, b);
    }

    /**
     * Sets the Java long into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>long</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, long v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, long v)
            throws JepException;

    /**
     * Sets the Java double into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>double</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, double v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, double v)
            throws JepException;

    /**
     * Sets the Java float into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>float</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, float v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, float v)
            throws JepException;

    // -------------------------------------------------- set arrays

    /**
     * Sets the Java boolean[] into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>boolean[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, boolean[] v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, boolean[] v)
            throws JepException;

    /**
     * Sets the Java int[] into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            an <code>int[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, int[] v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, int[] v)
            throws JepException;

    /**
     * Sets the Java short[] into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>short[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, short[] v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, short[] v)
            throws JepException;

    /**
     * Sets the Java byte[] into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>byte[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, byte[] v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, byte[] v)
            throws JepException;

    /**
     * Sets the Java long[] into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>long[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, long[] v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, long[] v)
            throws JepException;

    /**
     * Sets the Java double[] into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>double[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, double[] v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, double[] v)
            throws JepException;

    /**
     * Sets the Java float[] into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>float[]</code> value
     * @exception JepException
     *                if an error occurs
     */
    public void set(String name, float[] v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, float[] v)
            throws JepException;

    // -------------------------------------------------- close me

    /**
     * Shuts down the Python sub-interpreter. Make sure you call this to prevent
     * memory leaks.
     * 
     */
    @Override
    public synchronized void close() throws JepException {
        if (this.closed)
            return;

        if (!Thread.currentThread().equals(thread)) {
            /*
             * This is inherently unsafe, the thread state information inside
             * Python can get screwed up if a sub-interpreter is closed from a
             * different thread. Py_EndInterpreter is assuming that the
             * interpreter is being ended from the same thread. If close() is
             * called from a different thread, at best you will lose a little
             * bit of memory, at worst you will screw up Python's internal
             * tracking of thread state, which could lead to deadlocks or
             * crashes.
             */
            Thread current = Thread.currentThread();
            StringBuilder warning = new StringBuilder(THREAD_WARN)
                    .append("Unsafe close() of Python sub-interpreter by thread ")
                    .append(current.getName())
                    .append(".\nPlease close() from the creating thread to ensure stability.");
            throw new JepException(warning.toString());
        }

        // close all the PyObjects we created
        for (int i = 0; i < this.pythonObjects.size(); i++) {
            pythonObjects.get(i).close();
        }

        // don't attempt close twice if something goes wrong
        this.closed = true;

        try {
            eval(this.tstate, "import jep");
            eval(this.tstate, "jep.shared_modules_hook.teardownImporter()");
        } catch (JepException e) {
            throw new RuntimeException(e);
        }

        this.close(tstate);
        this.tstate = 0;

        threadUsed.set(false);
    }

    private native void close(long tstate);

}
