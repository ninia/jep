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

/**
 * Class for creating instances of Interpreters which are sandboxed from other
 * Interpreters. Sub-interpreters isolate different SubInterpreter instances,
 * allowing them to be used safely from multiple unrelated pieces of a Java
 * application without any risk that the SubInterpreters will impact each other.
 * An exception to this is when a SubIntepreter uses CPython extensions, there
 * is no guarantee that other Interpreters may not be affected. The level of
 * isolation depends on if the CPython extension was written to support multiple
 * interpreters in the same process. In those cases you may want to consider
 * using the shared modules feature or a SharedInterpreter.
 *
 * Within a single Java process it is valid to mix Interpreter instances that
 * use SubInterpreters with SharedInterpreters. The SubInterpreter instances
 * will remain isolated from other SubInterpreters and from any
 * SharedInterpreters. To maintain stability, it is not possible to have
 * multiple Interpreter instances active on the same Thread at the same time and
 * this limitation includes SubInterpreters.
 * 
 * 
 * @since 3.9
 */
public class SubInterpreter extends Jep {

    /**
     * Creates a new sub interpreter.
     * 
     * @throws JepException
     *             if an error occurs
     */
    @SuppressWarnings("deprecation")
    public SubInterpreter() throws JepException {
        super();
    }

    @SuppressWarnings("deprecation")
    public SubInterpreter(JepConfig config) throws JepException {
        super(config);
    }

}
