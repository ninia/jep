package jep.test.numpy;

import jep.Jep;
import jep.JepException;

/**
 * Tests closing a sub-interpreter with numpy and then trying to use a new
 * sub-interpreter with numpy. Illustrates a problem where the reference to the
 * any() method is lost.
 * 
 * Created: October 2015
 * 
 * @author Ben Steffensmeier
 * @version $Id$
 */
public class TestNumpyAny {

    public static void main(String[] args) {
        Jep jep = null;
        try {
            jep = new Jep();
            jep.eval("import numpy");
            jep.eval("numpy.ndarray([1]).any()");
            jep.close();

            jep = new Jep();
            jep.eval("import numpy");

            // this line will fail and throw an exception
            jep.eval("numpy.ndarray([1]).any()");
            jep.close();
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (jep != null) {
                jep.close();
            }
        }
    }

}
