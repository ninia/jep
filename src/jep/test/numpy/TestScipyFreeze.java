package jep.test.numpy;

import jep.Jep;
import jep.JepException;

/**
 * A test class that illustrates scipy locking up the thread when there are two
 * sub-interpreters on the same thread.
 * 
 * 
 * Created: Tue Jul 14 2015
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 * @version $Id$
 */
public class TestScipyFreeze {

    /**
     * @param args
     */
    public static void main(String[] args) {
        Jep jep0 = null;
        Jep jep = null;
        try {
            jep0 = new Jep(true);
            jep = new Jep(true);
            jep0.eval("from scipy.special import erf");
            jep.eval("from scipy.special import erf");
            System.out.println("returned from python interpreters");
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (jep != null) {
                System.out.println("closing jep interpreter");
                jep.close();
            }
            if (jep0 != null) {
                System.out.println("closing jep0 interpreter");
                jep0.close();
            }
        }
        System.out.println("java main() finished");
    }

}
