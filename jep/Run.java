package jep;


import java.io.*;

/**
 * <pre>
 * Run.java - Execute a Python script.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Created: Sun Jun  6 15:06:34 2004
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$	
 */
public class Run {
    
    private final static String USAGE =
    "  Run.java [-i] [-x] file\n" +
    "-i    Run script interactively\n" +
    "-d    Echo statements before running them.";
    
    
    /**
     * Describe <code>main</code> method here.
     *
     * @param args[] a <code>String</code> value
     * @exception Throwable if an error occurs
     * @exception Exception if an error occurs
     */
    public static void main(String args[]) throws Throwable {
        Jep jep = null;
        
        boolean interactive = false;
        boolean debug       = false;
        String  file        = null;

        for(int i = 0; i < args.length; i++) {
            if(args[i].equals("-i"))
                interactive = true;
            else if(args[i].equals("-d"))
                debug = true;
            else if(args[i].equals("-h")) {
                System.out.println(USAGE);
                System.exit(1);
            }
            else if(args[i].startsWith("-")) {
                System.out.println("Run: Unknown option: " + args[i]);
                System.out.println(USAGE);
                System.exit(1);
            }
        }
        
        file = args[args.length - 1];

        try {
            if(debug) {
                jep = new Jep(true, ".");
                BufferedReader in = new BufferedReader(new FileReader(file));
                String line = in.readLine();
                
                while(line != null) {
                    System.out.println("# " + line);
                    jep.eval(line + "\n");
                    
                    line = in.readLine();
                }
                
                jep.eval(null);
                in.close();
            }
            else {
                jep = new Jep(false, ".");
                jep.runScript(file);
            }
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
