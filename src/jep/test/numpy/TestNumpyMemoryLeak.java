package jep.test.numpy;

import jep.Jep;
import jep.JepException;

/**
 * Illustrates a tiny leak somewhere in numpy. This is really hard to spot but
 * if you have a long running process (e.g. server process with 99.999% uptime)
 * then it will eventually show up.
 * 
 * To see it, use ps aux | grep TestNumpy repeatedly and watch the memory usage.
 * If you comment out the import numpy and instead import something else (in the
 * example below the os module), you can watch the memory rise but eventually it
 * will plateau. If you have an interpreter with only import numpy, you can
 * watch the memory rise and continue to rise indefinitely, albeit rather
 * slowly. Probably something in numpy needs a Py_DECREF or a free(struct).
 * 
 * Created: Mon Apr 13 2015
 * 
 * @author Nate Jensen
 */
public class TestNumpyMemoryLeak {

    private static final int REPEAT = 10000;

    /**
     * @param args
     */
    public static void main(String[] args) {
        Jep jep = null;
        for (int i = 0; i < REPEAT; i++) {
            try {
                jep = new Jep(true);
                jep.eval("import numpy");
                // jep.eval("import os");
            } catch (JepException e) {
                e.printStackTrace();
            } finally {
                if (jep != null) {
                    jep.close();
                }
            }
        }

    }

}
