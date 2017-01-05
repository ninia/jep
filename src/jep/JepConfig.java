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

import java.io.File;
import java.util.HashSet;
import java.util.Set;

/**
 * <p>
 * A configuration object for constructing a Jep instance, corresponding to the
 * configuration of the particular Python sub-interpreter. This class is
 * intended to make constructing Jep instances easier while maintaining
 * compatible APIs between releases.
 * </p>
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 * 
 * @since 3.5
 */
public class JepConfig {

    protected boolean interactive = false;

    protected StringBuilder includePath = null;

    protected ClassLoader classLoader = null;

    protected ClassEnquirer classEnquirer = null;

    protected boolean redirectOutputStreams = false;

    protected Set<String> sharedModules = null;

    /**
     * Sets whether <code>Jep.eval(String)</code> should support the slower
     * behavior of potentially waiting for multiple statements
     * 
     * @param interactive
     *            whether the Jep instance should be interactive
     * @return a reference to this JepConfig
     */
    public JepConfig setInteractive(boolean interactive) {
        this.interactive = interactive;
        return this;
    }

    /**
     * Sets a path of directories separated by File.pathSeparator that will be
     * appended to the sub-intepreter's <code>sys.path</code>
     * 
     * @param includePath
     *            directory or directories to include on sys.path
     * @return a reference to this JepConfig
     */
    public JepConfig setIncludePath(String includePath) {
        this.includePath = null;
        if (includePath != null) {
            this.includePath = new StringBuilder(includePath);
        }
        return this;
    }

    /**
     * Adds a path of directories separated by File.pathSeparator that will be
     * appended to the sub-intepreter's <code>sys.path</code>
     * 
     * @param includePaths
     *            directories to include on sys.path
     * @return a reference to this JepConfig
     */
    public JepConfig addIncludePaths(String... includePaths) {
        if (this.includePath == null) {
            this.includePath = new StringBuilder();
        }
        for (String path : includePaths) {
            if (this.includePath.length() > 0) {
                this.includePath.append(File.pathSeparator);
            }
            this.includePath.append(path);
        }
        return this;
    }

    /**
     * Sets the ClassLoader to use when importing Java classes from Python
     * 
     * @param classLoader
     *            the initial ClassLoader for the Jep instance
     * @return a reference to this JepConfig
     */
    public JepConfig setClassLoader(ClassLoader classLoader) {
        this.classLoader = classLoader;
        return this;
    }

    /**
     * Sets a ClassEnquirer to determine which imports are Python vs Java, or
     * null for the default {@link ClassList}
     * 
     * @param classEnquirer
     *            the ClassEnquirer for the Jep instance
     * @return a reference to this JepConfig
     */
    public JepConfig setClassEnquirer(ClassEnquirer classEnquirer) {
        this.classEnquirer = classEnquirer;
        return this;
    }

    /**
     * Sets whether to redirect the Python sys.stdout and sys.stderr streams to
     * the Java System.out and System.err streams
     * 
     * @param redirectOutputStreams
     *            whether to redirect Python streams to Java
     * @return a reference to this JepConfig
     */
    public JepConfig setRedirectOutputStreams(boolean redirectOutputStreams) {
        this.redirectOutputStreams = redirectOutputStreams;
        return this;
    }

    /**
     * Sets the names of modules which should be shared with other Jep
     * sub-interpreters. This can make it possible to use modules which are not
     * designed for use from Python sub-interpreters. This should not be
     * necessary for any module written in Python but is intended for extensions
     * that use the c-api. For a complete discussion of the types of problems
     * that can require shared modules see the documentation on
     * shared_modules_hook.py.
     * 
     * @param sharedModules
     *            a set of module names that should be shared
     * @return a reference to this JepConfig
     * 
     * @since 3.6
     */
    public JepConfig setSharedModules(Set<String> sharedModules) {
        this.sharedModules = sharedModules;
        return this;
    }

    /**
     * Add a module name to the set of shared modules
     * 
     * @deprecated This method will be removed in a future release. Use
     *             #addSharedModules(String...) instead.
     * 
     * @param sharedModule
     *            a module name that should be shared
     * @return a reference to this JepConfig
     * @see #setSharedModules(Set)
     * 
     * @since 3.6
     */
    @Deprecated
    public JepConfig addSharedModule(String sharedModule) {
        if (sharedModules == null) {
            sharedModules = new HashSet<String>();
        }
        sharedModules.add(sharedModule);
        return this;
    }
    
    /**
     * Adds module names to the set of shared modules
     * 
     * @param sharedModules
     *            a set of module names that should be shared
     * @return a reference to this JepConfig
     * @see #setSharedModules(Set)
     * 
     * @since 3.6
     */
    public JepConfig addSharedModules(String... sharedModule) {
        if (sharedModules == null) {
            sharedModules = new HashSet<String>();
        }
        for (String sm : sharedModule) {
            sharedModules.add(sm);
        }
        return this;
    }
}
