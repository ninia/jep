package jep;

/**
 * <pre>
 * JepException.java
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
 *
 * Created: Fri Apr 30 10:35:03 2004
 *
 * </pre>
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$
 */
public class JepException extends Throwable {

    /**
     * Creates a new <code>JepException</code> instance.
     *
     */
    public JepException() {
        super();
    }

    
    /**
     * Creates a new <code>JepException</code> instance.
     *
     * @param s a <code>String</code> value
     */
    public JepException(String s) {
        super(s);
    }
    
    
    /**
     * Creates a new <code>JepException</code> instance.
     *
     * @param t a <code>Throwable</code> value
     */
    public JepException(Throwable t) {
        super(t);
    }
}
