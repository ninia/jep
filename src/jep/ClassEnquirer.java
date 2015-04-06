package jep;


/**
 * <pre>
 * ClassEnquirer.java - Interface to enquire if a name is available
 * to be imported from Java.  Used by JEP's importer hook (PEP 302) to
 * determine if an attempt to import a module/class should be directed
 * to the Python importer or the Java importer.
 * 
 *
 * Copyright (c) 2004 - 2011 Mike Johnson.
 *
 * This file is licenced under the the zlib/libpng License.
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
 *
 * Created: Thurs Apr 02 2015
 *
 * </pre>
 *
 * @author [ndjensen at gmail.com] Nate Jensen
 * @version $Id$
 */
public interface ClassEnquirer {
	
	/**
	 * Checks if the name is likely available in Java.  A return
	 * value of true implies the name corresponds to a Java package or class, but
	 * does not guarantee that an import will succeed.  A return value of false
	 * implies that an import from Java would fail, but does not guarantee that an
	 * import will fail.
	 *   
	 * @param name the name to check, such as java, java.util, java.util.ArrayList
	 * @return true if it's likely supported by Java, false if it's likely python
	 */
	public boolean contains(String name);
	
	/**
	 * Whether or not this ClassEnquirer supports importing Java classes at the
	 * package level in addition to the class level.  For example, with the
	 * right ClassLoader JEP should always be able to successfully import Java
	 * classes with syntax such as:
	 * 
	 * <p><blockquote><pre>
	 * from java.util import ArrayList
	 * o = ArrayList()
	 * </p></blockquote></pre>
	 * 
	 * However, only in some scenarios can the package be imported separately
	 * without the fully qualified name, such as:
	 * 
	 * <p><blockquote><pre>
	 * import java.util as ju
	 * o = ju.ArrayList()	 
	 * </p></blockquote></pre>
	 * 
	 * This also roughly corresponds to whether or not dir(javaPackage) will 
	 * return a list of available classes or only the classes that have been
	 * explicitly imported.
	 * 
	 * @return true if this ClassEnquirer supports import of packages in addition
	 * to import of classes, false if it only supports importing classes.
	 */
	public boolean supportsPackageImport();

}
