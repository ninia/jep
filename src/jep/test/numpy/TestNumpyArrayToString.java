package jep.test.numpy;

import jep.Jep;
import jep.JepException;

/**
 * Tests closing a sub-interpreter with numpy and then trying to use a new
 * sub-interpreter with numpy. Illustrates a problem where the reference to the
 * array_str() method is lost.
 * 
 * Created: October 2015
 * 
 * @author Ben Steffensmeier
 * @version $Id$
 */
public class TestNumpyArrayToString {

    public static void main(String[] args) throws InterruptedException {
        Jep jep0 = null;

        try {
            jep0 = new Jep();
            jep0.eval("import numpy");

            Thread t = new Thread() {
                @Override
                public void run() {
                    Jep jep1 = null;
                    try {
                        jep1 = new Jep();
                        jep1.eval("import numpy");
                    } catch (JepException e) {
                        e.printStackTrace();
                    } finally {
                        if (jep1 != null) {
                            jep1.close();
                        }
                    }
                }
            };
            t.start();

            // wait for the other thread to finish and close
            t.join();

            // this line will fail and throw an exception
            jep0.eval("str(numpy.ndarray([1]))");
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (jep0 != null) {
                jep0.close();
            }
        }
    }
}