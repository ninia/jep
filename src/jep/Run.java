/**
 * Copyright (c) 2015 JEP AUTHORS.
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
 */
package jep;

import java.io.File;

/**
 * Run.java - Execute a Python script in the Jep interpreter.
 * 
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$
 */
public class Run {
    private static boolean interactive = false;

    private static boolean swingApp = false;

    private static String file = null;

    private static String scriptArgv = null;

    private final static String USAGE = "  Usage: jep.Run [OPTIONS]...  [FILE].. [SCRIPT ARGS]\n"
            + "Options:\n"
            + "  -i                         Run script interactively.\n"
            + "  -s                         Run script in event dispatching thread (for use with Swing)\n";

    public static int run(boolean eventDispatch) {
        Jep jep = null;

        try {
            jep = new Jep(false, ".");

            // "set" by eval'ing it
            jep.eval("import sys; sys.argv = argv = " + scriptArgv);
            if (!file.endsWith("jep" + File.separator + "console.py")) {
                jep.runScript(file);
            } else {
                // don't use console's __main__ so we can reuse the interpreter
                jep.setInteractive(true);
                jep.set("jepInstance", jep);
                jep.eval("from jep import console");
                jep.eval("console.prompt(jepInstance)");
            }
        } catch (Throwable t) {
            t.printStackTrace();
            if (jep != null)
                jep.close();

            return 1;
        }

        try {
            if (interactive) {
                jep.set("jep", jep);
                jep.eval("from jep import *");
                jep.eval("import console");
                jep.setInteractive(true);
                jep.eval("console.prompt(jep)");
            }
        } catch (Throwable t) {
            t.printStackTrace();
            jep.close();

            return 1;
        }

        // if we're the event dispatch thread, we should quit now.
        // don't close jep.
        if (eventDispatch)
            return 0;

        jep.close();
        return 0;
    }

    /**
     * Describe <code>main</code> method here.
     * 
     * @param args
     *            a <code>String</code> value
     * @exception Throwable
     *                if an error occurs
     * @exception Exception
     *                if an error occurs
     */
    public static void main(String args[]) throws Throwable {
        String scriptArgs[] = new String[args.length];
        int argsi = 0;

        for (int i = 0; i < args.length; i++) {
            if (file != null)
                scriptArgs[argsi++] = args[i];
            else if (args[i].equals("-i"))
                interactive = true;
            else if (args[i].equals("-s"))
                swingApp = true;
            else if (args[i].equals("-h")) {
                System.out.println(USAGE);
                System.exit(1);
            } else if (args[i].startsWith("-")) {
                System.out.println("Run: Unknown option: " + args[i]);
                System.out.println(USAGE);
                System.exit(1);
            } else if (!args[i].startsWith("-"))
                file = args[i];
        }

        if (file == null) {
            System.out.println("Run: Invalid file, null");
            System.out.println(USAGE);
            System.exit(1);
        }

        // setup argv
        StringBuffer b = new StringBuffer("[");
        // always the first arg
        b.append("'" + file + "',");
        // trailing comma is okay
        for (int i = 0; i < argsi; i++)
            b.append("'" + scriptArgs[i] + "',");
        b.append("]");
        scriptArgv = b.toString();

        int ret = 1;
        if (swingApp) {
            // run in the event-dispatching thread
            javax.swing.SwingUtilities.invokeAndWait(new Runnable() {
                @Override
                public void run() {
                    Run.run(swingApp);
                }
            });
            ret = 0;
        } else
            ret = run(swingApp);

        // in case we're run with -Xrs
        if (!swingApp)
            System.exit(ret);
    }

    private Run() {
    }

} // Run
