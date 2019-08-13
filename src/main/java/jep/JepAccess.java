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

import jep.python.MemoryManager;

/**
 * <p>
 * Allow access to Jep internal structure in sub-packages. This class should not
 * be used outside of the Jep project.
 * </p>
 * 
 * @author Ben Steffensmeier
 * 
 * @since 3.9
 */
public abstract class JepAccess {

    protected final Jep jep;

    protected JepAccess(Jep jep) {
        this.jep = jep;
    }

    protected long getThreadState() {
        return jep.getThreadState();
    }

    protected ClassLoader getClassLoader() {
        return jep.getClassLoader();
    }

    protected MemoryManager getMemoryManager() {
        return jep.getMemoryManager();
    }
}
