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
public class Console {
    
    /**
     * Describe <code>main</code> method here.
     *
     * @param args[] a <code>String</code> value
     * @exception Exception if an error occurs
     */
    public static void main (String args[]) throws Exception {
        Jep jep = null;
        
        try {
            jep = new Jep();
            jep.eval("import jep");
            jep.eval("print 'Hello, world'");
            jep.eval("if(True):\n    print 'true'");
            jep.eval("a = 5");
            System.out.println("a = " + jep.getValue("a"));
            jep.eval("def test():");
            jep.eval("    if(True): # foobar");
            jep.eval("        print 'called test'");
            jep.eval("        print 'still in if block.'");
            jep.eval("    print 'test, last line'");
            jep.eval("test()");
            jep.eval("print 'blah'");
        }
        finally {
            jep.close();
        }
    }
    
    
    /**
     * Creates a new <code>Console</code> instance.
     *
     */
    public Console() {
    } // Console constructor
    
} // Console
