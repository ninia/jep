/**
 * Copyright (c) 2004-2019 JEP AUTHORS.
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
import java.util.Map;

import jep.python.MemoryManager;

/**
 * As of Jep 3.9, you should strive to instantiate either a SubInterpreter or a
 * SharedInterpreter instance and for interacting with the interpreter use the
 * interface Interpreter. If you previously used Jep instances, use
 * SubInterpreter instances to retain the same behavior.
 * 
 * <p>
 * Embeds CPython in Java. Each Jep provides access to a Python interpreter and
 * maintains an independent global namespace for Python variables. Values can be
 * passed from Java to Python using the various set() methods. Various methods,
 * such as {@link #eval(String)} and {@link #invoke(String, Object...)} can be
 * used to execute Python code. Python variables can be accessed using
 * {@link #getValue(String)}.
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
 */
public class Jep implements Interpreter {

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

    private final MemoryManager memoryManager = new MemoryManager();

    private final boolean isSubInterpreter;

    // windows requires this as unix newline...
    private static final String LINE_SEP = "\n";

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
     * @deprecated Please use {@link MainInterpreter#setInitParams(PyConfig)}
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
     *             {@link MainInterpreter#setSharedModulesArgv(String...)}
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
     * Creates a new <code>Jep</code> instance and its associated interpreter.
     * 
     * @deprecated Deprecated in 3.9. Use SubInterpreter or SharedInterpreter
     *             instead. If you used Jep objects in previous releases, use
     *             SubIntepreter for the same behavior.
     * 
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @throws JepException
     *             if an error occurs
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
     * @throws JepException
     *             if an error occurs
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
     * @throws JepException
     *             if an error occurs
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
     * @throws JepException
     *             if an error occurs
     * @deprecated Please use {@link #Jep(JepConfig)} instead.
     */
    @Deprecated
    public Jep(boolean interactive, String includePath, ClassLoader cl,
            ClassEnquirer ce) throws JepException {
        this(new JepConfig().setInteractive(interactive)
                .setIncludePath(includePath).setClassLoader(cl)
                .setClassEnquirer(ce));
    }

    /**
     * Creates a new <code>Jep</code> instance and its associated interpreter.
     * 
     * @param config
     *            the configuration for the Jep instance
     * 
     * @deprecated Deprecated in 3.9. Use SubInterpreter or SharedInterpreter
     *             instead. If you used Jep objects in previous releases, use
     *             SubIntepreter for the same behavior.
     * 
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
    public Jep(JepConfig config) throws JepException {
        this(config, true);
    }

    protected Jep(JepConfig config, boolean useSubInterpreter)
            throws JepException {
        MainInterpreter mainInterpreter = MainInterpreter.getMainInterpreter();
        if (threadUsed.get()) {
            Thread current = Thread.currentThread();
            StringBuilder warning = new StringBuilder(THREAD_WARN)
                    .append("Unsafe reuse of thread ").append(current.getName())
                    .append(" for another Python sub-interpreter.\n")
                    .append("Please close() the previous Jep instance to ensure stability.");
            throw new JepException(warning.toString());
        }

        this.isSubInterpreter = useSubInterpreter;

        if (config.classLoader == null) {
            this.classLoader = this.getClass().getClassLoader();
        } else {
            this.classLoader = config.classLoader;
        }

        boolean hasSharedModules = config.sharedModules != null
                && !config.sharedModules.isEmpty();

        this.interactive = config.interactive;
        this.tstate = init(this.classLoader, hasSharedModules,
                useSubInterpreter);
        threadUsed.set(true);
        this.thread = Thread.currentThread();
        configureInterpreter(config);
    }

    protected void configureInterpreter(JepConfig config) throws JepException {
        // why write C code if you don't have to? :-)
        if (config.includePath != null) {
            String includePath = config.includePath.toString();

            // Added for compatibility with Windows file system
            if (includePath.contains("\\")) {
                includePath = includePath.replace("\\", "\\\\");
            }

            exec("import sys");
            exec("sys.path += '" + includePath + "'.split('"
                    + File.pathSeparator + "')");
        }
        boolean hasSharedModules = config.sharedModules != null
                && !config.sharedModules.isEmpty();

        eval("import jep");
        if (hasSharedModules) {
            set("sharedModules", config.sharedModules);
            set("sharedImporter", MainInterpreter.getMainInterpreter());
            exec("jep.shared_modules_hook.setupImporter(sharedModules,sharedImporter)");
            exec("del sharedModules");
            exec("del sharedImporter");
        }
        setupJavaImportHook(config.classEnquirer);
        if (config.redirectOutputStreams) {
            exec("from jep import redirect_streams");
            exec("redirect_streams.setup()");
        }
    }

    protected void setupJavaImportHook(ClassEnquirer enquirer)
            throws JepException {
        if (enquirer == null) {
            enquirer = ClassList.getInstance();
        }
        set("classlist", enquirer);
        exec("from jep import java_import_hook");
        exec("java_import_hook.setupImporter(classlist)");
        exec("del classlist");
        exec("del java_import_hook");
    }

    private native long init(ClassLoader classloader, boolean hasSharedModules,
            boolean useSubinterpreter) throws JepException;

    /**
     * Checks if the current thread is valid for the method call. All calls must
     * check the thread.
     * 
     * @deprecated For internal usage only.
     * 
     *             <b>Internal Only</b>
     * 
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
    public void isValidThread() throws JepException {
        if (this.thread != Thread.currentThread())
            throw new JepException("Invalid thread access.");
        if (this.closed)
            throw new JepException("Jep instance has been closed.");
        if (this.tstate == 0)
            throw new JepException("Initialization failed.");
    }

    @Override
    public void runScript(String script) throws JepException {
        runScript(script, null);
    }

    /**
     * Runs a Python script.
     * 
     * @deprecated This may be removed in a future version of Jep, as Jep does
     *             not fully support changing the ClassLoader after
     *             construction.
     * 
     * @param script
     *            a <code>String</code> absolute path to script file.
     * @param cl
     *            a <code>ClassLoader</code> value, may be null.
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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

    @Override
    public Object invoke(String name, Object... args) throws JepException {
        isValidThread();
        if (name == null || name.trim().equals("")) {
            throw new JepException("Invalid function name.");
        }

        return invoke(this.tstate, name, args, null);
    }

    @Override
    public Object invoke(String name, Map<String, Object> kwargs)
            throws JepException {
        isValidThread();
        if (name == null || name.trim().equals("")) {
            throw new JepException("Invalid function name.");
        }

        return invoke(this.tstate, name, null, kwargs);
    }

    @Override
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

    @Override
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

    @Override
    public void exec(String str) throws JepException {
        isValidThread();
        exec(this.tstate, str);
    }

    private native void exec(long tstate, String str) throws JepException;

    @Override
    public Object getValue(String str) throws JepException {
        isValidThread();

        return getValue(this.tstate, str, Object.class);
    }

    @Override
    public <T> T getValue(String str, Class<T> clazz) throws JepException {
        isValidThread();

        return clazz.cast(getValue(this.tstate, str, clazz));
    }

    private native Object getValue(long tstate, String str, Class<?> clazz)
            throws JepException;

    /**
     * @deprecated use Python 3 bytes object instead and
     *             {@link #getValue(String,Class)} with byte[].class
     *
     *             Retrieves a Python string object as a Java byte[].
     * 
     * @param str
     *            the name of the Python variable to get from the
     *            sub-interpreter's global scope
     * @return an <code>Object</code> array
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
    public byte[] getValue_bytearray(String str) throws JepException {
        isValidThread();

        return getValue_bytearray(this.tstate, str);
    }

    private native byte[] getValue_bytearray(long tstate, String str)
            throws JepException;

    private native long createModule(long tstate, String name)
            throws JepException;

    // -------------------------------------------------- set things

    /**
     * Sets the default classloader.
     * 
     * @deprecated This may be removed in a future version of Jep. Jep does not
     *             fully support changing the ClassLoader after construction.
     * 
     * @param cl
     *            a <code>ClassLoader</code> value
     */
    @Deprecated
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
     * @deprecated This may be removed in a future version of Jep.
     * 
     * @param v
     *            if the sub-interpreter should run in interactive mode
     */
    @Deprecated
    public void setInteractive(boolean v) {
        this.interactive = v;
    }

    /**
     * Gets whether or not this sub-interpreter is interactive.
     * 
     * @deprecated This may be removed in a future version of Jep.
     * 
     * @return whether or not the sub-interpreter is interactive
     */
    @Deprecated
    public boolean isInteractive() {
        return this.interactive;
    }

    @Override
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>String</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>boolean</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            an <code>int</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
    public void set(String name, int v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    /**
     * Sets the Java short into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            an <code>int</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>char[]</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
    public void set(String name, char[] v) throws JepException {
        isValidThread();

        set(tstate, name, new String(v));
    }

    /**
     * Sets the Java char into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>char</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
    public void set(String name, char v) throws JepException {
        isValidThread();

        set(tstate, name, new String(new char[] { v }));
    }

    /**
     * Sets the Java byte into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param b
     *            a <code>byte</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
    public void set(String name, byte b) throws JepException {
        isValidThread();

        set(tstate, name, b);
    }

    /**
     * Sets the Java long into the sub-interpreter's global scope with the
     * specified variable name.
     * 
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>long</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>double</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>float</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>boolean[]</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            an <code>int[]</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>short[]</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>byte[]</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>long[]</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>double[]</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
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
     * @deprecated Use {@link #set(String, Object)} instead.
     * 
     * @param name
     *            the Python name for the variable
     * @param v
     *            a <code>float[]</code> value
     * @throws JepException
     *             if an error occurs
     */
    @Deprecated
    public void set(String name, float[] v) throws JepException {
        isValidThread();

        set(tstate, name, v);
    }

    private native void set(long tstate, String name, float[] v)
            throws JepException;

    // -------------------------------------------------- close me

    /**
     * Gets the memory manager associated with this Jep instance. The memory
     * manager attempts to track native memory usage of PyObjects.
     * 
     * @return the memory manager
     */
    protected MemoryManager getMemoryManager() {
        return memoryManager;
    }

    protected long getThreadState() {
        return tstate;
    }

    /**
     * Gets the class loader associated with this Jep instance.
     * 
     * @return the class loader
     */
    protected ClassLoader getClassLoader() {
        return classLoader;
    }

    /**
     * Shuts down the Python interpreter. Make sure you call this to prevent
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
            StringBuilder warning = new StringBuilder(THREAD_WARN).append(
                    "Unsafe close() of Python sub-interpreter by thread ")
                    .append(current.getName())
                    .append(".\nPlease close() from the creating thread to ensure stability.");
            throw new JepException(warning.toString());
        }

        if (isSubInterpreter) {
            exec("import sys");
            Boolean hasThreads = getValue("'threading' in sys.modules",
                    Boolean.class);
            if (hasThreads.booleanValue()) {
                Integer count = getValue(
                        "sys.modules['threading'].active_count()",
                        Integer.class);
                if (count.intValue() > 1) {
                    throw new JepException(
                            "All threads must be stopped before closing Jep.");
                }
            }
        }

        getMemoryManager().cleanupReferences();

        // don't attempt close twice if something goes wrong
        this.closed = true;

        if (isSubInterpreter) {
            exec(this.tstate, "import jep");
            exec(this.tstate, "jep.shared_modules_hook.teardownImporter()");
        }

        this.close(tstate);
        this.tstate = 0;

        threadUsed.set(false);
    }

    private native void close(long tstate);

}
