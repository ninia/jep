/**
 * Copyright (c) 2018 JEP AUTHORS.
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
 * Class for creating instances of Interpreters which share all imported
 * modules. In this case each SharedInterpreter still maintains distinct global
 * variables but some interpreter state will be shared. This is equivalent to
 * using shared modules to share every Python package in Jep. Anything that
 * changes the way a module behaves will impact all SharedInterpreters so care
 * must be taken to ensure that different SharedInterpreters aren't conflicting.
 * For example sys.path, time.tzset(), and numpy.seterr() will change the
 * behavior of all SharedInterpreters.
 *
 * Within a single Java process it is valid to mix Interpreter instances that
 * use SubInterpreters with SharedInterpreters. The SubInterpreter instances
 * will remain isolated from SubInterpreter instances and from any
 * SharedInterpreters. To maintain stability, it is not possible to have
 * multiple Interpreter instances active on the same Thread at the same time and
 * this limitation includes SharedInterpreters.
 *
 *
 * @author Ben Steffensmeier
 * 
 * @since 3.8
 */
public class SharedInterpreter extends Jep {

    private static JepConfig config = new JepConfig();

    private static boolean initialized = false;

    public SharedInterpreter() throws JepException {
        super(config, false);
        exec("__name__ = '__main__'");
    }

    @Override
    protected void configureInterpreter(JepConfig config) throws JepException {
        synchronized (SharedInterpreter.class) {
            if (!initialized) {
                super.configureInterpreter(config);
                initialized = true;
            }
        }
    }

    /**
     * Sets interpreter settings for the all SharedInterpreters. This method
     * must be called before the first SharedInterpreter is created in the
     * process.
     * 
     * @param config
     *            the jep configuration to use.
     * 
     * @throws JepException
     *             if an error occurs
     * @since 3.9
     */
    public static void setConfig(JepConfig config) throws JepException {
        if (initialized) {
            throw new JepException(
                    "JepConfig must be set before creating any SharedInterpreters");
        } else if (config.sharedModules != null) {
            throw new JepException(
                    "sharedModules cannot be used with SharedInterpreters");
        }
        SharedInterpreter.config = config;
    }

}
