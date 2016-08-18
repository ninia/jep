package jep.test.numpy;

import jep.Jep;

/**
 * Tests closing a sub-interpreter with numpy and then trying to use a new
 * sub-interpreter with numpy. Illustrates a problem where the reference to the
 * any() method is lost.
 * 
 * This does NOT make use of the shared modules feature.
 * 
 * Created: October 2015
 * 
 * @author Ben Steffensmeier
 */
public class TestNumpyAnyLost {

    public static void main(String[] args) {
        Jep jep = null;
        try {
            jep = new Jep(false, ".");
            jep.eval("import numpy");
            jep.eval("numpy.ndarray([1]).any()");
            jep.close();

            jep = new Jep(false, ".");
            jep.eval("import numpy");

            // this line will fail and throw an exception
            jep.eval("numpy.ndarray([1]).any()");
            jep.close();
        } catch (Throwable e) {
            // we expected a failure, verify it
            assert e.getMessage().contains("'NoneType' object is not callable");
            jep.close();
            System.exit(0);
        } finally {
            if (jep != null) {
                jep.close();
            }
        }
        System.err.println("numpy mysteriously worked");
        System.exit(1);
    }

}
