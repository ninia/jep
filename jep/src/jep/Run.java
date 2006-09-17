package jep;


import java.io.*;

/**
 * <pre>
 * Run.java - Execute a Python script.
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
 * Created: Sun Jun  6 15:06:34 2004
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$	
 */
public class Run {
    
    private final static String USAGE =
    "  Run.java [-i] [-x] file [script args]\n" +
    "-i    Run script interactively";
    
    
    /**
     * Describe <code>main</code> method here.
     *
     * @param args[] a <code>String</code> value
     * @exception Throwable if an error occurs
     * @exception Exception if an error occurs
     */
    public static void main(String args[]) throws Throwable {
        Jep jep = null;
        
        boolean interactive  = false;
        String  file         = null;
        String  scriptArgs[] = new String[args.length];
        int     argsi        = 0;
        
        for(int i = 0; i < args.length; i++) {
            if(file != null)
                scriptArgs[argsi++] = args[i];
            else if(args[i].equals("-i"))
                interactive = true;
            else if(args[i].equals("-h")) {
                System.out.println(USAGE);
                System.exit(1);
            }
            else if(args[i].startsWith("-")) {
                System.out.println("Run: Unknown option: " + args[i]);
                System.out.println(USAGE);
                System.exit(1);
            }
            else if(!args[i].startsWith("-"))
                file = args[i];
        }
        
        if(file == null) {
            System.out.println("Run: Invaid file, null");
            System.out.println(USAGE);
            System.exit(1);
        }
        
        try {
            jep = new Jep(false, ".");
            
            // setup argv
            StringBuffer b = new StringBuffer("[");
            // always the first arg
            b.append("'" + file + "',");
            // trailing comma is okay
            for(int i = 0; i < argsi; i++)
                b.append("'" + scriptArgs[i] + "',");
            b.append("]");
            
            // "set" by eval'ing it
            jep.eval("argv = " + b);
            jep.runScript(file);
        }
        catch(Throwable t) {
            t.printStackTrace();
            jep.close();
            
            System.exit(1);
        }
        
        try {
            if(interactive) {
                jep.set("jep", jep);
                jep.eval("from jep import *");
                jep.eval("import console");
                jep.setInteractive(true);
                jep.eval("console.prompt(jep)");
            }
        }
        catch(Throwable t) {
            t.printStackTrace();
            jep.close();
            
            System.exit(1);
        }

        jep.close();
        // in case we're run with -Xrs
        System.exit(0);
    }
    
    
    private Run() {
    }
    
} // Run
