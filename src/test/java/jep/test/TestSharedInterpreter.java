package jep.test;

import jep.Interpreter;
import jep.SharedInterpreter;

/**
 * Tests that jep works without using sub-interpreters and verified that all
 * modules are shared between interpreters on different threads.
 *
 * Created: Jan 2018
 *
 * @author Ben Steffensmeier
 */
public class TestSharedInterpreter extends Thread {

    public static void main(String[] args) throws Throwable {
        testSharedModule();
        testSharedJep();
        testSharedTypes();
    }

    public static void testSharedModule() throws Throwable{
        TestSharedInterpreter[] t = new TestSharedInterpreter[4];
        try (Interpreter interp = new SharedInterpreter()) {
            interp.eval("import sys");
            interp.set("n", t.length);
            interp.eval("sys.sharedTestThing = [None] * n");
        }

        for (int i = 0; i < t.length; i += 1) {
            t[i] = new TestSharedInterpreter(i);
            t[i].start();
        }
        for (int i = 0; i < t.length; i += 1) {
            t[i].join();
            if (t[i].e != null) {
                throw t[i].e;
            }
        }
        try (Interpreter interp = new SharedInterpreter()) {
            for (int i = 0; i < t.length; i += 1) {
                interp.eval("import sys");
                interp.set("i", i);
                Boolean b = (Boolean) interp.getValue("sys.sharedTestThing[i]");
                if (b.booleanValue() == false) {
                    throw new IllegalStateException(i + " failed");
                }
            }
        }
    }

    /**
     * In the original implementation the _jep module would be recreated for
     * each SharedInterpreter. It shouldn't do that.
     */
    public static void testSharedJep() throws Throwable{
        try (Interpreter interp = new SharedInterpreter()) {
            interp.exec("import _jep");
            interp.exec("import sys");
            interp.exec("sys.testJep = _jep");
        }
        try (Interpreter interp = new SharedInterpreter()) {
            interp.exec("import _jep");
            interp.exec("import sys");
            boolean pass = interp.getValue("sys.testJep is _jep", Boolean.class);
            if (!pass) {
                throw new IllegalStateException("_jep module changed.");
            }
        }
    }

    public static void testSharedTypes() throws Throwable{
        try (Interpreter interp = new SharedInterpreter()) {
            interp.exec("import sys");
            interp.exec("from java.util import ArrayList");
            interp.exec("sys.sampleArrayList = ArrayList()");
        }
        try (Interpreter interp = new SharedInterpreter()) {
            interp.exec("import sys");
            interp.exec("from java.util import ArrayList");
            interp.exec("sampleArrayList = ArrayList()");
            boolean pass = interp.getValue("id(type(sampleArrayList)) == id(type(sys.sampleArrayList))", Boolean.class);
            if (!pass) {
                throw new IllegalStateException("types are different");
            }
        }
    }

    public Exception e = null;

    public final int index;

    public TestSharedInterpreter(int index) {
        this.index = index;
    }

    @Override
    public void run() {
        try (Interpreter interp = new SharedInterpreter()) {
            interp.exec("import sys");
            interp.set("index", index);
            interp.exec("sys.sharedTestThing[index] = True");
        } catch (Exception e) {
            this.e = e;
        }
    }

}
