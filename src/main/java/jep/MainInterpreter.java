/**
 * Copyright (c) 2015-2021 JEP AUTHORS.
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

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.SynchronousQueue;

/**
 * The main Python interpreter that all sub-interpreters will be created from.
 * In a simpler embedded Python project, a single Python interpreter would be
 * used and all would be good. However, since Jep supports multithreading with
 * multiple sub-interpreters, we need the MainInterpreter to work around some
 * issues.
 * 
 * The MainInterpreter is used to avoid potential deadlocks. Python can deadlock
 * when trying to acquire the GIL through methods such as <a href=
 * "https://docs.python.org/3/c-api/init.html#c.PyGILState_Ensure">PyGILState_*</a>.
 * While Jep does not use those methods, CPython extensions such as numpy do.
 * The deadlock can occur if there is more than one PyThreadState per thread. To
 * get around this, the MainInterpreter creates a unique thread that initializes
 * Python and keeps this thread around forever. This ensures that any new
 * sub-interpreters cannot be created on the same thread as the main Python
 * interpreter.
 * 
 * The MainInterpreter is also used to support shared modules. While each
 * sub-interpreter is fairly sandboxed, in practice this does not always work
 * well with CPython extensions. In particular, disposing of a sub-interpreter
 * that has imported a CPython extension may cause some of the CPython
 * extension's objects to be garbage collected. To get around this, shared
 * modules import on the main interpreter's thread so they can be shared amongst
 * sub-interpreters and will never be garbage collected.
 * 
 * For more information about why the MainInterpreter class exists, see <a href=
 * "https://docs.python.org/3/c-api/init.html#bugs-and-caveats">Sub-interpreter
 * bugs and caveats</a>.
 * 
 * 
 * @author Nate Jensen
 * @author Ben Steffensmeier
 * @since 3.8
 */
public final class MainInterpreter implements AutoCloseable {

    private static MainInterpreter instance = null;

    private static PyConfig pyConfig = null;

    private static String[] sharedModulesArgv = null;

    private static String jepLibraryPath = null;

    private Thread thread;

    private BlockingQueue<String> importQueue = new SynchronousQueue<>();

    private BlockingQueue<Object> importResults = new SynchronousQueue<>();

    private Throwable error;

    private MainInterpreter() {
        // only this class can instantiate it
    }

    /**
     * Creates the MainInterpreter instance that will be used by Jep. This
     * should be called from all Jep constructors to ensure the native module
     * has been loaded and initialized before a valid Interpreter instance is
     * produced.
     * 
     * @return the main interpreter
     * @throws Error
     *             if an error occurs
     */
    protected static synchronized MainInterpreter getMainInterpreter()
            throws Error {
        if (null == instance) {
            try {
                instance = new MainInterpreter();
                instance.initialize();
            } catch (Error e) {
                instance.close();
                instance.error = e;
                throw e;
            }
        } else if (null != instance.error) {
            throw new Error(
                    "The main Python interpreter previously failed to initialize.",
                    instance.error);
        }
        return instance;
    }

    /**
     * Initializes CPython, by calling a native function in the jep module, and
     * ultimately Py_Initialize(). We load on a separate thread to try and avoid
     * GIL issues that come about from a sub-interpreter being on the same
     * thread as the main interpreter.
     * 
     * @throws Error
     *             if an error occurs
     */
    protected void initialize() throws Error {
        if (jepLibraryPath != null) {
            System.load(jepLibraryPath);
        } else {
            try {
                System.loadLibrary("jep");
            } catch (UnsatisfiedLinkError e) {
                if (!LibraryLocator.findJepLibrary(pyConfig)) {
                    throw e;
                }
            }
        }

        if (pyConfig != null) {
            setInitParams(pyConfig.noSiteFlag, pyConfig.noUserSiteDirectory,
                    pyConfig.ignoreEnvironmentFlag, pyConfig.verboseFlag,
                    pyConfig.optimizeFlag, pyConfig.dontWriteBytecodeFlag,
                    pyConfig.hashRandomizationFlag, pyConfig.pythonHome);
        }

        thread = new Thread("JepMainInterpreter") {

            @Override
            public void run() {
                try {
                    initializePython(sharedModulesArgv);
                } catch (Throwable t) {
                    error = t;
                } finally {
                    synchronized (MainInterpreter.this) {
                        MainInterpreter.this.notify();
                    }
                }
                /*
                 * We need to keep this main interpreter thread around. It might
                 * be used for importing shared modules. Even if it is not used
                 * it must remain running because if its thread shuts down while
                 * another thread is in Python, then the thread state can get
                 * messed up leading to stability/GIL issues.
                 */
                try {
                    while (true) {
                        String nextImport = importQueue.take();
                        Object result = nextImport;
                        try {
                            MainInterpreter.sharedImportInternal(nextImport);
                        } catch (JepException e) {
                            result = e;
                        }
                        importResults.put(result);
                    }
                } catch (InterruptedException e) {
                    // ignore
                }
            }
        };
        thread.setDaemon(true);
        synchronized (this) {
            thread.start();
            try {
                this.wait();
            } catch (InterruptedException e) {
                if (error != null) {
                    error = e;
                }
            }
        }
        if (error != null) {
            throw new Error(error);
        }
    }

    /**
     * Import a module into the main interpreter on the correct thread for that
     * interpreter. This is called from the Python shared modules import hook to
     * create a module needed by a SubInterpreter.
     * 
     * @param module
     *            the name of the module to import
     * @throws JepException
     *             if an error occurs
     */
    public void sharedImport(String module) throws JepException {
        try {
            importQueue.put(module);
            Object result = importResults.take();
            if (result instanceof JepException) {
                throw new JepException(
                        "Error importing shared module " + module,
                        ((JepException) result));
            }
        } catch (InterruptedException e) {
            throw new JepException(e);
        }
    }

    /**
     * Stop the interpreter thread.
     */
    @Override
    public void close() {
        if (thread != null) {
            thread.interrupt();
        }
    }

    /**
     * Sets interpreter settings for the main Python interpreter. This method
     * must be called before the first Interpreter instance is created in the
     * process.
     * 
     * @param config
     *            the python configuration to use.
     * 
     * @throws IllegalStateException
     *             if called after the Python interpreter is initialized
     * @since 3.6
     */
    public static void setInitParams(PyConfig config) throws IllegalStateException {
        if (null != instance) {
            throw new IllegalStateException(
                    "Jep.setInitParams(PyConfig) called after initializing python interpreter.");
        }
        pyConfig = config;
    }

    /**
     * Sets the sys.argv values on the main interpreter. This method must be
     * called before the first Interpreter instance is created in the process.
     * 
     * @param argv
     *            the arguments to be set on Python's sys.argv for the main
     *            interpreter
     * @throws IllegalStateException
     *             if called after the Python interpreter is initialized
     * 
     * @since 3.7
     */
    public static void setSharedModulesArgv(String... argv)
            throws IllegalStateException {
        if (instance != null) {
            throw new IllegalStateException(
                    "Jep.setSharedModulesArgv(...) called after initializing python interpreter.");
        }
        sharedModulesArgv = argv;
    }

    /**
     * Sets the path of the jep native library. The location should be a path
     * that can be passed to {@link java.lang.System#load(String)}. This method
     * must be called before the first Interpreter instance is created in the
     * process.
     * 
     * @param path
     *            the path of the jep native library, an absolute path leading
     *            to a file that is often named libjep.so or libjep.dll.
     * @throws IllegalStateException
     *             if called after the Python interpreter is initialized
     * 
     * @since 3.9
     */
    public static void setJepLibraryPath(String path) throws IllegalStateException {
        if (instance != null) {
            throw new IllegalStateException(
                    "Jep.setJepLibraryPath(...) called after initializing python interpreter.");
        }
        jepLibraryPath = path;
    }

    private static native void setInitParams(int noSiteFlag,
            int noUserSiteDiretory, int ignoreEnvironmentFlag, int verboseFlag,
            int optimizeFlag, int dontWriteBytecodeFlag,
            int hashRandomizationFlag, String pythonHome);

    private static native void initializePython(String[] mainInterpreterArgv);

    private static native void sharedImportInternal(String module)
            throws JepException;

}
