package jep.test.numpy;

import jep.Jep;
import jep.JepConfig;

/**
 * Tests closing a sub-interpreter with numpy and then trying to use a new
 * sub-interpreter with numpy. Illustrates a problem where the reference to the
 * any() method is lost, even if shared modules are used.
 * 
 * The shared modules feature is used AFTER numpy has already lost some
 * references, hence the problem.
 * 
 * Created: August 2016
 * 
 * @author Nate Jensen
 */
public class TestNumpyAnyOrdering {

    public static void main(String[] args) {
        Jep jep = null;
        try {
            jep = new Jep(false, ".");
            jep.eval("import numpy");
            jep.eval("numpy.ndarray([1]).any()");
            jep.close();

            JepConfig config = new JepConfig().addIncludePaths(".")
                    .addSharedModule("numpy");
            jep = new Jep(config);
            jep.eval("import numpy");

            /*
             * this line fails because we already closed a Jep with numpy before
             * we made numpy a shared module
             */
            jep.eval("numpy.ndarray([1]).any()");
            jep.close();
        } catch (Throwable e) {
            e.printStackTrace();
            jep.close();
            System.exit(1);
        } finally {
            if (jep != null) {
                jep.close();
            }
        }
        System.exit(0);
    }
}
