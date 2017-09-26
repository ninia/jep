/**
 * Copyright (c) 2017 JEP AUTHORS.
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

/**
 * Utility class for finding the jep native library for JNI. {@link Jep} will
 * try to load the library using a simple {@link System#loadLibrary(String)}
 * however if that fails then this class will be used to try to find the
 * location of the library.
 * 
 * The jep library is typically distributed inside the jep python module, so
 * this class attempts to find a jep module with the python library inside of
 * it. In order to find the module this class will attempt to mimic the process
 * python uses to build the path and it will search through the same
 * directories. Since this is jsut a mirror of what python is doing, if there
 * are changes to python it may require changes here. Python also has alot of
 * specialization for specific operating systems. Rather than try to handle each
 * case the same, this class will check in any location that might be valid on
 * any operating system and assumes that inappropriate locations will simply not
 * exist.
 * 
 * @author Ben Steffensmeier
 * @since 3.8
 */
final class LibraryLocator {

    private final String libraryName;

    private final boolean ignoreEnv;

    private final boolean noSite;

    private final boolean noUserSite;

    private LibraryLocator(PyConfig pyConfig) {
        if (pyConfig != null) {
            ignoreEnv = pyConfig.ignoreEnvironmentFlag != 0
                    && pyConfig.ignoreEnvironmentFlag != -1;
            noSite = pyConfig.noSiteFlag != 0 && pyConfig.noSiteFlag != -1;
            noUserSite = pyConfig.noUserSiteDirectory != 0
                    && pyConfig.noUserSiteDirectory != -1;
        } else {
            ignoreEnv = false;
            noSite = false;
            noUserSite = false;
        }

        String libraryName = System.mapLibraryName("jep");
        if (libraryName.endsWith(".dylib")) {
            /*
             * OS X uses a different extension for System.loadLibrary and
             * System.mapLibraryName
             */
            libraryName = libraryName.replace(".dylib", ".jnilib");
        }
        this.libraryName = libraryName;
    }

    /**
     * Search in every directory in PYTHONPATH if it is defined.
     * 
     * @return true if the library was loaded.
     */
    private boolean searchPythonPath() {
        if (ignoreEnv) {
            return false;
        }
        String pythonPath = System.getenv("PYTHONPATH");
        if (pythonPath != null) {
            String[] pathDirs = pythonPath.split(File.pathSeparator);
            for (String pathDir : pathDirs) {
                if (searchPackageDir(new File(pathDir))) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Search in the site-packages directory under PYTHONHOME. The python
     * inspiration for this code is in the getsitepackages function in site.py
     * 
     * @return true if the library was loaded.
     */
    private boolean searchSitePackages() {
        if (ignoreEnv || noSite) {
            return false;
        }
        String pythonHome = System.getenv("PYTHONHOME");
        if (pythonHome != null) {
            for (String libDirName : new String[] { "lib", "lib64", "Lib" }) {
                File libDir = new File(pythonHome, libDirName);
                if (!libDir.isDirectory()) {
                    continue;
                }
                File packagesDir = new File(libDir, "site-packages");
                if (searchPackageDir(packagesDir)) {
                    return true;
                }
                packagesDir = new File(libDir, "site-python");
                if (searchPackageDir(packagesDir)) {
                    return true;
                }
                for (File pythonDir : libDir.listFiles()) {
                    if (pythonDir.isDirectory()
                            && pythonDir.getName().matches("python\\d\\.\\d")) {
                        packagesDir = new File(pythonDir, "site-packages");
                        if (searchPackageDir(packagesDir)) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    /**
     * Search in the site-packages directory under PYTHONHOME. This is based off
     * the paths defined in
     * <a href="https://www.python.org/dev/peps/pep-0370/">PEP 370</a>
     * 
     * @return true if the library was loaded.
     */
    private boolean searchUserSitePackages() {
        if (noSite || noUserSite) {
            return false;
        }
        String appdata = System.getenv("APPDATA");
        if (appdata != null) {
            File libDir = new File(appdata, "Python");
            if (libDir.isDirectory()) {
                for (File pythonDir : libDir.listFiles()) {
                    if (pythonDir.isDirectory()
                            && pythonDir.getName().matches("python\\d{2}")) {
                        File packagesDir = new File(pythonDir, "site-packages");
                        if (searchPackageDir(packagesDir)) {
                            return true;
                        }
                    }
                }
            }
        }

        String userHome = System.getProperty("user.home");
        if (userHome != null) {
            File localDir = new File(userHome, ".local");
            File libDir = new File(localDir, "lib");
            if (libDir.isDirectory()) {
                for (File pythonDir : libDir.listFiles()) {
                    if (pythonDir.isDirectory()
                            && pythonDir.getName().matches("python\\d\\.\\d")) {
                        File packagesDir = new File(pythonDir, "site-packages");
                        if (searchPackageDir(packagesDir)) {
                            return true;
                        }
                    }
                }
            }

        }
        return false;
    }

    /**
     * Attempt to load the jep native library from a directory that is expected
     * to contain python modules(such as a PYTHONPATH entry or a site-packages
     * directory).
     * 
     * @return true if the library was loaded.
     */
    private boolean searchPackageDir(File pathDir) {
        if (pathDir.isDirectory()) {
            File jepDir = new File(pathDir, "jep");
            if (jepDir.isDirectory()) {
                File libraryFile = new File(jepDir, libraryName);
                if (libraryFile.exists()) {
                    System.load(libraryFile.getAbsolutePath());
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * @return true if the library was loaded.
     * 
     * @param pyConfig
     *            configuration options for python, used to restrict search path
     *            if the config is restricted. This may be null.
     */
    public static boolean findJepLibrary(PyConfig pyConfig) {
        /* An instance is used to hold temporary state. */
        LibraryLocator loc = new LibraryLocator(pyConfig);
        if (loc.searchPythonPath()) {
            return true;
        } else if (loc.searchSitePackages()) {
            return true;
        } else if (loc.searchUserSitePackages()) {
            return true;
        }
        return false;
    }
}
