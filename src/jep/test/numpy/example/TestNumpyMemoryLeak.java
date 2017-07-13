package jep.test.numpy.example;

import jep.Jep;
import jep.JepException;

/**
 * Illustrates a tiny leak somewhere in numpy. This is really hard to spot but
 * if you have a long running process (e.g. server process with 99.999% uptime)
 * then it will eventually show up.
 * 
 * To see it with this test, use ps aux | grep TestNumpyMemoryLeak repeatedly
 * and watch the memory usage. If you comment out the import numpy and instead
 * import something else (in the example below the os module), you can watch the
 * memory rise but eventually it will plateau. If you have an interpreter with
 * only import numpy, you can watch the memory rise and continue to rise
 * indefinitely, albeit rather slowly.
 * 
 * Note if you use the shared modules feature of Jep and make numpy a shared
 * module, then the problem doesn't exist as numpy will not reinitialize.
 * 
 * This test is NOT run by the unittests.
 * 
 * Created: April 2015
 * 
 * @author Nate Jensen
 * @see "https://github.com/numpy/numpy/issues/5857"
 */
public class TestNumpyMemoryLeak {

    private static final int REPEAT = 100000;

    public static void main(String[] args) throws Exception {
        Thread t = new Thread() {
            @Override
            public void run() {
                /*
                 * do not close jep0, otherwise the test may not work with some
                 * versions of numpy
                 */
                Jep jep0 = null;
                try {
                    jep0 = new Jep();
                    jep0.eval("import numpy");
                } catch (JepException e) {
                    e.printStackTrace();
                }
            }
        };
        t.start();

        // give the first thread time to start
        Thread.currentThread().sleep(2000);

        for (int i = 0; i < REPEAT; i++) {
            try (Jep jep = new Jep()) {
                jep.eval("import numpy");
                // jep.eval("import os");
            }
        }
    }

}
