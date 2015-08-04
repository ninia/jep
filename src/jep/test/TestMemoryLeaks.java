package jep.test;

import jep.Jep;
import jep.JepException;

/**
 * A test class that runs for a long time doing tiny actions that can be used to
 * narrow down memory leaks or verify there are no leaks.
 * 
 * When running this, from a terminal do 'ps aux | grep TestMemoryUsage' and
 * repeatedly and watch the memory usage. It will rise for a while and then
 * plateau. If it never plateaus but continues rising, that is indicative of a
 * leak.
 * 
 * 
 * Created: Wed Jul 15 2015
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 * @version $Id$
 */
public class TestMemoryLeaks {

    protected static int REPEAT = 1000000;

    /**
     * @param args
     * @throws JepException
     */
    public static void main(String[] args) throws JepException {
        // testStartStopSubInterpreter();
        testSimpleImport();
        // testObjectCreation();
        // testCompare();
    }

    public static void testStartStopSubInterpreter() throws JepException {
        for (int i = 0; i < REPEAT; i++) {
            Jep jep = new Jep(false);
            jep.close();
        }
    }

    public static void testSimpleImport() throws JepException {
        for (int i = 0; i < REPEAT; i++) {
            Jep jep = new Jep(false);
            jep.eval("import java.util");
            jep.close();
        }
    }

    public static void testObjectCreation() throws JepException {
        for (int i = 0; i < REPEAT; i++) {
            Jep jep = new Jep(false);
            jep.eval("from java.util import ArrayList");
            jep.eval("from java.lang import Integer");
            jep.eval("x = ArrayList()");
            jep.eval("x.add(Integer(5))");
            jep.eval("x.add(Integer(-9))");
            jep.close();
        }
    }

    public static void testCompare() throws JepException {
        for (int i = 0; i < REPEAT; i++) {
            Jep jep = new Jep(false);
            jep.eval("from java.lang import Integer");
            jep.eval("x = Integer(5)");
            jep.eval("y = Integer(6)");
            jep.eval("b = (x < y)");
            Boolean b = (Boolean) jep.getValue("b");
            jep.close();
        }
    }

    // TODO more simplistic tests

}
