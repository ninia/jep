package jep.test.numpy;

import jep.Jep;
import jep.JepConfig;

/**
 * Tests closing a sub-interpreter with numpy shared and then trying to use a
 * new sub-interpreter with numpy. Illustrates that since each import of numpy
 * is a shared module, then numpy is no longer losing references to methods.
 * 
 * Created: August 2016
 * 
 * @author Nate Jensen
 */
public class TestNumpyProdShared {

    private static final int N_JEPS = 1;

    public static void main(String[] args) {
        Jep jep = null;
        try {
            JepConfig config = new JepConfig().addIncludePaths(".")
                    .addSharedModules("numpy");
            jep = new Jep(config);
            jep.eval("import numpy");
            jep.eval("numpy.ndarray([1]).prod()");
            jep.close();

            for (int i = 0; i < N_JEPS; i++) {
                jep = new Jep(config);
                jep.eval("import numpy");

                /*
                 * this line no longer fails because numpy was shared the first
                 * time
                 */
                jep.eval("numpy.ndarray([1]).prod()");
                jep.close();
            }
        } catch (Throwable e) {
            e.printStackTrace();
            System.exit(1);
        } finally {
            if (jep != null) {
                jep.close();
            }
        }
        System.exit(0);
    }

}
