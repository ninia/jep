package jep;

import java.io.*;

import java.util.HashMap;
import java.util.ArrayList;
import java.util.StringTokenizer;
import java.util.Enumeration;

import java.util.jar.*;

import java.net.URL;


/**
 * <pre>
 * Jep.java - Embeds CPython in Java.
 *
 * Copyright (c) 2004, 2005 Mike Johnson.
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
 * Created: Sat Sep 2 10:03:21 2006
 *
 * For internal use. Searches Java jvm for loaded classes.
 *
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$
 */
public class ClassList {
    private static ClassList inst;

    // storage for package, members
    private static HashMap<String, ArrayList<String>> packages =
        new HashMap<String, ArrayList<String>>();


    private ClassList() throws JepException {
        loadClassPath();
        loadPackages();
        loadJREClasses();
    }


    /**
     * load jar files from class path
     *
     */
    private void loadClassPath() throws JepException {
        StringTokenizer tok = new StringTokenizer(
            System.getProperty("java.class.path"),
            ";");
        
        while(tok.hasMoreTokens()) {
            String el = tok.nextToken();

            if(!el.toLowerCase().endsWith(".jar"))
                continue;       // ignore filesystem classpath

            try {
                JarFile jfile = new JarFile(el, false);
                Enumeration<JarEntry> entries = jfile.entries();
                while(entries.hasMoreElements()) {
                    JarEntry ent = entries.nextElement();

                    if(!ent.getName().toLowerCase().endsWith(".class"))
                        continue;

                    // ent.getName() looks like:
                    // jep/ClassList.class
                    int    end   = ent.getName().lastIndexOf('/');
                    String pname = ent.getName().substring(
                        0,
                        end).replace('/', '.');
                    String cname = stripClassExt(ent.getName().substring(end + 1));

                    addClass(pname, cname);
                }

                jfile.close();
            }
            catch(IOException e) {
                // debugging only
                e.printStackTrace();
            }
        }
    }


    /**
     * the jre will tell us about what jar files it has open. use that
     * facility to get a list of packages. then read the files
     * ourselves since java won't share.
     *
     */
    private void loadPackages() throws JepException {
        ClassLoader cl = this.getClass().getClassLoader();

        Package[] ps = Package.getPackages();
        for(Package p : ps) {
            String pname = p.getName().replace('.', '/');
            URL url = cl.getResource(pname);

            if(url == null || !url.getProtocol().equals("file"))
                continue;

            File dir = null;
            try {
                dir = new File(url.toURI());
            }
            catch(java.net.URISyntaxException e) {
                throw new JepException(e);
            }

            for(File classfile : dir.listFiles(new ClassFilenameFilter()))
                addClass(p.getName(), stripClassExt(classfile.getName()));
        }
    }


    // don't pass me nulls.
    // strips .class from a file name.
    private String stripClassExt(String name) {
        return name.substring(0, name.length() - 6);
    }


    /**
     * The jre keeps a list of classes in the lib folder. We don't
     * have a better way to figure out what's in the java package, so
     * this is my little hack.
     *
     */
    private void loadJREClasses() throws JepException {
        // load the JRE's classlist file
        String javaHome = System.getProperty("java.home");
        String pathSep  = System.getProperty("file.separator");

        File file = new File(javaHome + pathSep + "lib" + pathSep + "classlist");
        if(!file.exists())
            return;

        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new InputStreamReader(new FileInputStream(file)));

            String line = "";
            while((line = reader.readLine()) != null) {
                // ignore any class with $
                if(line.indexOf('$') > -1)
                    continue;

                // lines in the file look like: java/lang/String
                // split on /
                String[]     parts = line.split("\\/");
                StringBuffer pname = new StringBuffer();
                String       cname = parts[parts.length - 1];

                for(int i = 0; i < parts.length - 1; i++) {
                    pname.append(parts[i]);
                    if(i < parts.length - 2)
                        pname.append(".");
                }

                addClass(pname.toString(), cname);
            }
        }
        catch(IOException e) {
            throw new JepException(e);
        }
        finally {
            try {
                if(reader != null)
                    reader.close();
            }
            catch(IOException ee) {
                ;
            }
        }
    }


    // add a class with given package name
    private void addClass(String pname, String cname) {
        ArrayList<String> el = packages.get(pname.toString());
        if(el == null)
            el = new ArrayList<String>();

        // convert to style we need in C code
        cname = pname + "." + cname;

        // unlikely, but don't add a class twice.
        if(el.indexOf(cname) > -1)
            return;

        el.add(cname);
        packages.put(pname.toString(), el);
    }


    private String[] _get(String p) {
        ArrayList<String> el = packages.get(p);
        if(el == null)
            return new String[0];

        // wtf...
        // return (String[]) el.toArray();
        // fails with classcastexception.
        // bollocks.

        String[] ret = new String[el.size()];
        for(int i = 0; i < el.size(); i++)
            ret[i] = (String) el.get(i);

        return ret;
    }


    /**
     * get classnames in package
     *
     * @param p a <code>String</code> value
     * @return <code>String[]</code> array of class names
     * @exception JepException if an error occurs
     */
    public static String[] get(String p) throws JepException {
        return ClassList.getInstance()._get(p);
    }


    /**
     * get ClassList instance
     *
     * @return <code>ClassList</code> instance
     */
    public static synchronized ClassList getInstance() throws JepException {
        if(ClassList.inst == null)
            ClassList.inst = new ClassList();
        return ClassList.inst;
    }


    /**
     *
     * testing only
     */
    public static void main(String argv[]) throws Throwable {
        ClassList cl = ClassList.getInstance();
        for(String c : cl.get("java.lang"))
            System.out.println(c);

        // test loadPackages
        for(String c : cl.get("jep"))
            System.out.println(c);
    }
}


class ClassFilenameFilter implements java.io.FilenameFilter {
    public boolean accept(File dir, String name) {
        return (name != null && name.toLowerCase().endsWith(".class"));
    }
}
