package jep;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

/**
 * <pre>
 * NamingConventionClassEnquirer.java - A simple enquirer to see
 * if the package/class name starts following one of the normal
 * Java conventions of package/classnames.
 * 
 * This class can be used in situations where you want to avoid
 * the overhead of initializing ClassList and/or importing all
 * classes in a package.
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
public class NamingConventionClassEnquirer implements ClassEnquirer {

	private static final List<String> TOP_LEVEL = Arrays.asList("java",
			"javax", "com", "gov", "org", "edu", "mil", "net");
	
	private List<String> javaNames;
	
	private List<String> namesWithDot;
	
	/**
	 * Constructor
	 */
	public NamingConventionClassEnquirer() {
		this(false);
	}
	
	/**
	 * Constructor
	 * @param includeCountryCodes whether or not a name starting with a
	 * 2-letter country code such a uk, de, fr, us, ch should be considered as
	 * a Java package. 
	 */
	public NamingConventionClassEnquirer(boolean includeCountryCodes) {
		if(includeCountryCodes) {
			String[] codes = Locale.getISOCountries();
			javaNames = new ArrayList<String>(TOP_LEVEL.size() + codes.length);
			javaNames.addAll(TOP_LEVEL);
			for(String country: codes) {
				javaNames.add(country.toLowerCase());
			}			
		} else {
			javaNames = new ArrayList<String>(TOP_LEVEL.size());
			javaNames.addAll(TOP_LEVEL);
		}
		
		namesWithDot = new ArrayList<String>(javaNames.size());
		for(String javaStart: javaNames) {
			namesWithDot.add(javaStart.concat("."));
		}
	}
	
	/**
	 * Adds a package name to the list of names that should be considered
	 * as Java packages
	 * @param pkgStart
	 */
	public void addJavaPackageStart(String pkgStart) {
		javaNames.add(pkgStart);
		namesWithDot.add(pkgStart.concat("."));
	}

	@Override
	public boolean contains(String name) {
		if(name == null) {
			throw new IllegalArgumentException("name must not be null");
		}
		int size = javaNames.size();
		
		for(int i=0; i < size; i++) {
			if(name.equals(javaNames.get(i)) || name.startsWith(namesWithDot.get(i))) {
				return true;
			}
		}
		
		return false;
	}

	@Override
	public boolean supportsPackageImport() {
		return false;
	}

}
