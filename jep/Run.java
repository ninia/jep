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
    
    /**
     * Describe <code>main</code> method here.
     *
     * @param args[] a <code>String</code> value
     * @exception Throwable if an error occurs
     * @exception Exception if an error occurs
     */
    public static void main(String args[]) throws Throwable {
        Jep jep = null;
        try {
            jep = new Jep();
            jep.runScript(args[0]);
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
