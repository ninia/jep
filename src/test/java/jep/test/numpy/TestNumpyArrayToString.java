package jep.test.numpy;

import jep.Interpreter;
import jep.Jep;
import jep.JepConfig;
import jep.JepException;
import jep.SubInterpreter;

/**
 * Tests closing a sub-interpreter with numpy and then trying to use a new
 * sub-interpreter with numpy. Before shared modules, this illustrated a problem
 * where the reference to the array_str() method is lost and a 'NoneType' is not
 * callable error. Now with shared modules the test should pass.
 * 
 * Created: October 2015
 * 
 * @author Ben Steffensmeier
 * @see "https://github.com/numpy/numpy/issues/3961"
 */
public class TestNumpyArrayToString {

    public static void main(String[] args) throws InterruptedException {
        Interpreter interp0 = null;

        try {
            interp0 = new SubInterpreter(new JepConfig().addIncludePaths(".")
                    .addSharedModules("numpy"));
            interp0.eval("import numpy");

            Thread t = new Thread() {
                @Override
                public void run() {
                    Jep interp1 = null;
                    try {
                        interp1 = new SubInterpreter(
                                new JepConfig().addIncludePaths(".")
                                        .addSharedModules("numpy"));
                        interp1.eval("import numpy");
                    } catch (JepException e) {
                        e.printStackTrace();
                        try {
                            interp1.close();
                        } catch (JepException e1) {
                            e1.printStackTrace();
                        }
                        System.exit(1);
                    } finally {
                        if (interp1 != null) {
                            try {
                                interp1.close();
                            } catch (JepException e) {
                                e.printStackTrace();
                                System.exit(1);
                            }
                        }
                    }
                }
            };
            t.start();

            // wait for the other thread to finish and close
            t.join();

            // this line no longer fails due to the usage of shared modules
            interp0.eval("str(numpy.ndarray([1]))");
        } catch (JepException e) {
            e.printStackTrace();
            try {
                interp0.close();
            } catch (JepException e1) {
                e.printStackTrace();
            }
            System.exit(1);
        } finally {
            if (interp0 != null) {
                try {
                    interp0.close();
                } catch (JepException e) {
                    e.printStackTrace();
                    System.exit(1);
                }
            }
        }
        System.exit(0);
    }
}
