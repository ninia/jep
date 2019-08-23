package jep.test.numpy.example;

import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;
import jep.SubInterpreter;

/**
 * A test class that illustrates how numpy's floating point error handling can
 * cause python to deadlock trying to acquire the GIL. More specifically,
 * ufunc_object.c calls NPY_ALLOW_C_API which attempts to acquire the GIL.
 * Python is confused about the thread state and GIL state due to more than one
 * Jep interpreter on the same thread.
 * 
 * Note this problem is only recreatable if you intentionally ignore the JEP
 * THREAD WARNINGS and create two interpreters that coexist on the same thread
 * at the same time.
 * 
 * This test is NOT run by the unittests.
 * 
 * Created: April 2015
 * 
 * @author Nate Jensen
 * @see "https://github.com/numpy/numpy/issues/5856"
 */
public class TestNumpyGILFreeze {

    private static final String UNDERFLOW = "def forceUnderflow():\n"
            + "   a = numpy.array([1, 2], numpy.float32)\n"
            + "   for i in xrange(10):\n" + "      a /= 10000\n"
            + "   print 'python method complete'\n";

    public static void main(String[] args) throws JepException {
        Interpreter interp = null;
        try {
            Interpreter interp0 = new SubInterpreter();
            interp = new SubInterpreter();
            interp0.close();
            interp.eval("import numpy");
            /*
             * If error conditions are set to ignore, we will not reach the
             * point in ufunc_object.c where NPY_ALLOW_C_API will cause the GIL
             * to hang.
             */
            interp.eval("numpy.seterr(under='warn')");
            interp.eval(UNDERFLOW);
            interp.eval("forceUnderflow()"); // this line will freeze
            System.out.println("returned from python interpreter");
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (interp != null) {
                System.out.println("closing jep interpreter");
                interp.close();
            }
        }
        System.out.println("java main() finished");
    }

}
