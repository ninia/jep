package jep.test.numpy;

import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;
import jep.SubInterpreter;

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

    public static void main(String[] args) throws JepException {
        Interpreter interp = null;
        try {
            JepConfig config = new JepConfig().addIncludePaths(".")
                    .addSharedModules("numpy");
            interp = new SubInterpreter(config);
            interp.eval("import numpy");
            interp.eval("numpy.ndarray([1]).prod()");
            interp.close();

            for (int i = 0; i < N_JEPS; i++) {
                interp = new SubInterpreter(config);
                interp.eval("import numpy");

                /*
                 * this line no longer fails because numpy was shared the first
                 * time
                 */
                interp.eval("numpy.ndarray([1]).prod()");
                interp.close();
            }
        } catch (Throwable e) {
            e.printStackTrace();
            System.exit(1);
        } finally {
            if (interp != null) {
                interp.close();
            }
        }
        System.exit(0);
    }

}
