package jep;

/**
 * A test class that illustrates how numpy's floating point error handling can
 * cause python to deadlock trying to acquire the GIL. More specifically,
 * ufunc_object.c calls NPY_ALLOW_C_API which attempts to acquire the GIL.
 * Python is confused about the thread state and GIL state due to the top level
 * interpreter being initialized on the same thread as the sub-interpreter(s).
 * 
 * Please see the TODO below for how to trigger the freeze.
 * 
 * Created: Thu Apr 09 2015
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 * @version $Id$
 */
public class TestNumpyGILFreeze {

    private static final String UNDERFLOW = 
            "def forceUnderflow():\n" +
            "   a = numpy.array([1, 2], numpy.float32)\n" +
            "   for i in xrange(10):\n" +
            "      a /= 10000\n" +
            "   print 'python method complete'\n";

    /**
     * Main() method to demonstrate the issue.
     * 
     * @param args
     */
    public static void main(String[] args) {                
        /*
         * TODO: To show the freeze, you need to alter Jep.TopInterpreter to
         * NOT use a separate thread for System.loadLibrary("jep");
         */
        createJepAndUseNumpy();
        createJepAndUseNumpy();

        System.out.println("java main() finished");
    }

    public static void createJepAndUseNumpy() {
        Jep jep = null;
        try {
            jep = new Jep(true);
            jep.eval("import numpy");
            /*
             * If error conditions are set to ignore, we will not reach the
             * point in ufunc_object.c where NPY_ALLOW_C_API will cause the GIL
             * to hang.
             */
            jep.eval("numpy.seterr(under='print')");
            jep.eval(UNDERFLOW);
            jep.eval("forceUnderflow()");
            System.out.println("returned from python interpreter");
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (jep != null) {
                System.out.println("closing jep interpreter");
                jep.close();
            }
        }
    }

}
