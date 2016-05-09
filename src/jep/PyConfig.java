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

/**
 * <p>
 * A configuration object for setting Python pre-initialization parameters.
 * </p>
 *
 * @author Jeff V Stein
 */
public class PyConfig {

    protected int noSiteFlag = -1;
    protected int noUserSiteDirectory = -1;
    protected int ignoreEnvironmentFlag = -1;

    /**
     * Set the Py_NoSiteFlag variable on the python interpreter. This
     * corresponds to the python "-S" flag and will prevent the "site" module
     * from being automatically loaded.
     * 
     * @param noSiteFlag value to pass to Python for Py_NoSiteFlag
     * @return a reference to this PyConfig
     */
    public PyConfig setNoSiteFlag(int noSiteFlag) {
        this.noSiteFlag = noSiteFlag;
        return this;
    }

    /**
     * Set the Py_NoUserSiteDirectory variable on the python interpreter. This
     * corresponds to the python "-s" flag and will prevent the user's local
     * python site directory from being added to sys.path.
     * 
     * @param noUserSiteDirectory value to pass to Python for
     *        Py_NoUserSiteDirectory
     * @return a reference to this PyConfig
     */
    public PyConfig setNoUserSiteDirectory(int noUserSiteDirectory) {
        this.noUserSiteDirectory = noUserSiteDirectory;
        return this;
    }

    /**
     * Set the Py_IgnoreEnvironmentFlag variable on the python interpreter.
     * This corresponds to the python "-E" flag and will instruct python to
     * ignore all PYTHON* environment variables (e.g. PYTHONPATH).
     * 
     * @param ignoreEnvironmentFlag value to pass to Python for
     *        Py_IgnoreEnvironmentFlag
     * @return a reference to this PyConfig
     */
    public PyConfig setIgnoreEnvironmentFlag(int ignoreEnvironmentFlag) {
        this.ignoreEnvironmentFlag = ignoreEnvironmentFlag;
        return this;
    }
}
