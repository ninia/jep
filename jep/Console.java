package jep;

/**
 * Console.java
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
 *
 * @author <a href="mailto:mrjohnson0@users.sourceforge.net">Mike Johnson</a>
 * @version 1.0
 */

import java.io.*;

public class Console {
    
    private static final String PS1 = ">>> ";
    private static final String PS2 = "... ";


    /**
     * Describe <code>main</code> method here.
     *
     * @param args[] a <code>String</code> value
     * @exception Exception if an error occurs
     */
    public static void main(String args[]) throws Exception {

        Jep jep = null;
        BufferedReader in = null;
        
        try {
            PrintStream out = System.out;
            
            in  = new BufferedReader(new InputStreamReader(System.in));
            jep = new Jep(true);

            out.print(PS1);
            String line;
            while((line = in.readLine()) != null) {
                boolean ran = true;
                
                try {
                    ran = jep.eval(line);
                }
                catch(JepException e) {
                    e.printStackTrace();
                }
                
                if(ran)
                    out.print(PS1);
                else
                    out.print(PS2);
            }
        }
        finally {
            if(jep != null)
                jep.close();
            if(in != null)
                in.close();
        }
    }
    
    
    /**
     * Creates a new <code>Console</code> instance.
     *
     */
    public Console() {
    } // Console constructor
    
} // Console
