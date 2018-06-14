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
 * Class for creating instances of Jep which do not use sub-interpreters. 
 * Typically each Jep instance creates a unique Python sub-interpreter.
 * Sub-interpreters isolate different Jep instances, allowing them to be used
 * safely from multiple unrelated pieces of a Java application without any risk
 * that the Jep instances will impact each other.
 *
 * This class allows creation of a Jep instance that does not use
 * sub-interpreters. In this case each SharedInterpreter still maintains
 * distinct global variables but some interpreter state will be shared. The
 * primary impact is that SharedInterpreters will share any imported modules.
 * This is equivalent to using shared modules to share every python package in
 * Jep. Anything that changes the way a module behaves will impact all
 * SharedInterpreters so care must be taken to ensure that different
 * SharedInterpreters aren't conflicting. For example sys.path, time.tzset(),
 * and numpy.seterr() will change the behavior of all SharedInterpreters.
 *
 * Within a single Java process it is valid to mix Jep instances that use
 * sub-interpreters with SharedInterpreters. The sub-interpreter instances of
 * Jep will remain isolated from other Jep instances and from any
 * SharedInterpreters. To maintain stability, it is not possible to have
 * multiple Jep instances active on the same Thread at the same time and this
 * limitation includes SharedInterpreters.
 *
 *
 * @author Ben Steffensmeier
 * 
 * @since 3.8
 */
public class SharedInterpreter extends Jep {

    private static boolean initialized = false;

    public SharedInterpreter() throws JepException{
        super(new JepConfig(), false);
    }

    @Override
    protected void configureInterpreter(JepConfig config) throws JepException {
        synchronized (SharedInterpreter.class) {
            if(!initialized){
                setupJavaImportHook(config.classEnquirer);
                initialized = true;
            }
        }

    }

}
