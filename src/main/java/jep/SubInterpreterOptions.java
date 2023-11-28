/**
 * Copyright (c) 2023 JEP AUTHORS.
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
 * A configuration object containing additional options for constructing a SubInterpreter.
 *
 * These options map directly to the options documented at
 * https://docs.python.org/3/c-api/init.html#c.PyInterpreterConfig.
 * 
 * These options are ignored if a Python version earlier than 3.12 is used with Jep.
 *
 * @since 4.2
 */
public class SubInterpreterOptions {

    protected boolean isolated;

    protected int useMainObmalloc = -1;

    protected int allowFork = -1;

    protected int allowExec = -1;

    protected int allowThreads = -1;

    protected int allowDaemonThreads = -1;

    protected int checkMultiInterpeExtensions = -1;

    protected int ownGIL = -1;

    protected SubInterpreterOptions(boolean isolated){
        this.isolated = isolated;
    }

    /**
     * If this is false then the sub-interpreter will use its own "object"
     * allocator state. Otherwise it will use (share) the main interpreter’s.
     * 
     * If this is false then check_multi_interp_extensions must be true. If this
     * is true then own GIL must not be true.
     *
     * Shared Modules cannot be used when this is set to false.
     *
     * @param useMainObmalloc
     *            whether the sub-interpreter will share the main interpreter's
     *            allocator state.
     * @return a reference to this SubInterpreterOptions
     *
     */
    public SubInterpreterOptions setUseMainObmalloc(boolean useMainObmalloc) {
        this.useMainObmalloc = useMainObmalloc ? 1 : 0;
        return this;
    }

    /**
     * If this is false then the runtime will not support forking the process
     * in any thread where the sub-interpreter is currently active. Otherwise
     * fork is unrestricted.
     *
     * Note that the subprocess module still works when fork is disallowed.
     *
     * @param allowFork
     *            whether the  sub-interpreter will allow fork.
     * @return a reference to this SubInterpreterOptions
     *
     */
    public SubInterpreterOptions setAllowFork(boolean allowFork) {
        this.allowFork = allowFork ? 1 : 0;
        return this;
    }

    /**
     * If this is false then the runtime will not support replacing the current
     * process via exec (e.g. os.execv()) in any thread where the
     * sub-interpreter is currently active. Otherwise exec is unrestricted.
     *
     * Note that the subprocess module still works when exec is disallowed.
     *
     * @param allowExec
     *            whether the  sub-interpreter will allow exec.
     * @return a reference to this SubInterpreterOptions
     *
     */
    public SubInterpreterOptions setAllowExec(boolean allowExec) {
        this.allowExec = allowExec ? 1 : 0;
        return this;
    }

    /**
     * If this is false then the sub-interpreter’s threading module won’t be
     * able to create threads. Otherwise threads are allowed.
     *
     * @param allowThreads
     *            whether the  sub-interpreter will allow threads.
     * @return a reference to this SubInterpreterOptions
     *
     */
    public SubInterpreterOptions setAllowThreads(boolean allowThreads) {
        this.allowThreads = allowThreads ? 1 : 0;
        return this;
    }

    /**
     * If this is false then the sub-interpreter’s threading module won’t be
     * able to create daemon threads. Otherwise daemon threads are allowed 
     * (as long as allowThreads is true).
     *
     * @param allowDaemonThreads
     *            whether the  sub-interpreter will allow daemon threads.
     * @return a reference to this SubInterpreterOptions
     *
     */
    public SubInterpreterOptions setAllowDaemonThreads(boolean allowDaemonThreads) {
        this.allowDaemonThreads = allowDaemonThreads ? 1 : 0;
        return this;
    }

    /**
     * If this is false then all extension modules may be imported, including
     * legacy (single-phase init) modules, in any thread where the
     * sub-interpreter is currently active. Otherwise only multi-phase init
     * extension modules (see PEP 489) may be imported.
     *
     * This must be true if useMainObmalloc is false.
     *
     * @param checkMultiInterpeExtensions
     *            whether the  sub-interpreter will restrict import of legacy modules
     * @return a reference to this SubInterpreterOptions
     *
     */
    public SubInterpreterOptions setCheckMultiInterpeExtensions(boolean checkMultiInterpeExtensions) {
        this.checkMultiInterpeExtensions = checkMultiInterpeExtensions ? 1 : 0;
        return this;
    }

    /**
     * This determines the operation of the GIL for the sub-interpreter. When
     * this is false use (share) the main interpreter’s GIL. When this is true
     * use the sub-interpreter’s own GIL.
     *
     * If this is true then useMainObmalloc must be false.
     *
     * @param ownGIL
     *            whether the sub-interpreter will use its own GIL.
     * @return a reference to this SubInterpreterOptions
     *
     */
    public SubInterpreterOptions setOwnGIL(boolean ownGIL) {
        this.ownGIL = ownGIL ? 1 : 0;
        return this;
    }

    /**
     * Create a new SubInterpreterOptions with the default legacy settings.
     * All settings can be changed using the setters in this class.
     */
    public static SubInterpreterOptions legacy() {
        return new SubInterpreterOptions(false);
    }

    /**
     * Create a new SubInterpreterOptions with the default isolated settings.
     * Using these settings eliminates GIL contention but may not be compatible
     * with all third party modules. These settings are not compatible with
     * shared modules. All settings can be changed using the setters in this class.
     */
    public static SubInterpreterOptions isolated() {
        return new SubInterpreterOptions(true);
    }

}
