package jep.test;

import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;
import jep.SubInterpreter;

/**
 * Tests that a java exception thrown while executing Python is set as the cause
 * of the JepException.
 * 
 * Created: July 2017
 * 
 * @author Ben Steffensmeier
 */
public class TestExceptionCause {

    public static void main(String[] args) throws JepException {
        JepConfig config = new JepConfig().addIncludePaths(".");
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.eval("from java.util import ArrayList");
            try {
                interp.eval("ArrayList().get(0)");
            } catch (JepException e) {
                if (!(e.getCause() instanceof IndexOutOfBoundsException)) {
                    throw e;
                }
            }
            try {
                interp.eval(
                        "try:\n  ArrayList().get(0)\nexcept AttributeError:\n  pass");
            } catch (JepException e) {
                if (!(e.getCause() instanceof IndexOutOfBoundsException)) {
                    throw e;
                }
            }
        }
    }

}
