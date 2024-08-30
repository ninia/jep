/**
 * Copyright (c) 2024 JEP AUTHORS.
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

import java.util.HashSet;
import java.util.Set;

/**
 * <p>
 * A {@link ClassEnquirer} that defines specific packages as Python packages.
 * This implementation takes a delegate ClassEnquirer to delegate all import
 * determinations as Java or Python except in the cases of the specific Python
 * package names provided. Imports of the specified packages in the embedded
 * Python interpreter will use the Python importer and not the Java importer.
 * This is useful when you have package name conflicts between Java and Python.
 * Some examples are projects like py4j and tensorflow which have Python package
 * names matching Java package names. In those cases, using the default
 * ClassEnquirer of {@link ClassList} will lead to the Java packages on the
 * classpath taking precedence on imports instead of the Python package. That
 * can be solved by using this ClassEnquirer with a delegate of ClassList's
 * instance.
 * </p>
 * 
 * @author Nate Jensen
 * 
 * @since 4.2.1
 */
public class AllowPythonClassEnquirer implements ClassEnquirer {

    protected ClassEnquirer delegate;

    protected Set<String> pyPkgNames = new HashSet<>();

    /**
     * Constructor
     * 
     * @param delegate
     *            the ClassEnquirer instance to delegate method calls to except
     *            in the case of the specified Python package names
     * @param pythonPackageNames
     *            the names of Python packages that should not be treated as a
     *            potential import from Java and instead should immediately
     *            default to being imported by the normal Python importer
     */
    public AllowPythonClassEnquirer(ClassEnquirer delegate,
            String... pythonPackageNames) {
        this.delegate = delegate;
        for (String pyPkg : pythonPackageNames) {
            pyPkgNames.add(pyPkg);
        }
    }

    @Override
    public boolean isJavaPackage(String name) {
        for (String pyPkg : pyPkgNames) {
            if (name.equals(pyPkg) || name.startsWith(pyPkg + ".")) {
                return false;
            }
        }

        return delegate.isJavaPackage(name);
    }

    @Override
    public String[] getClassNames(String pkgName) {
        return delegate.getClassNames(pkgName);
    }

    @Override
    public String[] getSubPackages(String pkgName) {
        return delegate.getSubPackages(pkgName);
    }

}
