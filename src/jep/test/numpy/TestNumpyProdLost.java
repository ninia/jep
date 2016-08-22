package jep.test.numpy;

import jep.Jep;

/**
 * Tests closing a sub-interpreter with numpy and then trying to use a new
 * sub-interpreter with numpy. Illustrates a problem where the reference to the
 * prod() method is lost.
 * 
 * In Python 2 this test results in a 'NoneType' is not callable error.
 * 
 * In Python 3 this test results in a JVM crash.
 * 
 * This does NOT make use of the shared modules feature.
 * 
 * 
 * Created: October 2015
 * 
 * @author Ben Steffensmeier
 */
public class TestNumpyProdLost {

    public static void main(String[] args) {
        Jep jep = null;
        try {
            jep = new Jep(false, ".");
            jep.eval("import numpy");
            jep.eval("numpy.ndarray([1]).prod()");
            jep.close();

            jep = new Jep(false, ".");
            jep.eval("import numpy");

            // this line will fail and throw an exception
            jep.eval("numpy.ndarray([1]).prod()");
            jep.close();
        } catch (Throwable e) {
            /*
             * we expected a failure, usually it is 'NoneType' object is not
             * callable
             */
            jep.close();
            System.exit(0);
        } finally {
            if (jep != null) {
                jep.close();
            }
        }
        System.err.println("numpy mysteriously worked with sub-interpreters");
        System.exit(1);
    }

}
