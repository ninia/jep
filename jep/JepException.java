package jep;

/**
 * JepException.java
 *
 *
 * Created: Fri Apr 30 12:02:32 2004
 *
 * @author <a href="mailto:mrjohnson0@users.sourceforge.net">Mike Johnson</a>
 * @version 1.0
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
