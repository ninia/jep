package jep.test;

import jep.Interpreter;
import jep.JepConfig;
import jep.SubInterpreter;

/**
 * Tests that shared modules can be imported simultaneously on multiple threads.
 * Python 3 requires careful synchronization to ensure that uninitialized
 * modules are not inadvertantly shared.
 * 
 * This test relies on multiple threads importing at the same time. If the OS
 * thread scheduling gets lucky it might pass even if there are problems. We
 * just assume that over the course of a release if it breaks eventually someone
 * will see a failed test.
 *
 * Created: August 2016
 *
 * @author Ben Steffensmeier
 */
public class TestSharedModulesThreads extends Thread {

    public static void main(String[] args) throws Throwable {
        TestSharedModulesThreads[] t = new TestSharedModulesThreads[16];
        for (int i = 0; i < t.length; i += 1) {
            t[i] = new TestSharedModulesThreads();
            t[i].start();
        }
        for (int i = 0; i < t.length; i += 1) {
            t[i].join();
            if (t[i].e != null) {
                throw t[i].e;
            }
        }
        /*
         * Ensure that the shared moduler import hooks don't interfere with pure
         * python threading. This use case is not well supported but it mostly
         * works so this just checks for obvious flaws.
         */
        try (Interpreter interp = new SubInterpreter(
                new JepConfig().addIncludePaths(".")
                        .addSharedModules("xml.etree.ElementTree"))) {
            interp.eval("import threading");
            interp.eval("success = False");
            StringBuilder testFunction = new StringBuilder();
            testFunction.append("def testFunction():\n");
            testFunction.append("    global success\n");
            testFunction.append("    import struct\n");
            testFunction.append("    success = True");
            interp.eval(testFunction.toString());
            interp.eval("t = threading.Thread(target=testFunction)");
            interp.eval("t.start()");
            interp.eval("t.join()");
            /*
             * Sleep to workaround a python bug, that occasionally fails the
             * test. Remove the sleep when 2.7 and 3.3 are no longer supported.
             * https://bugs.python.org/issue18808
             */
            Thread.sleep(10);
            Object success = interp.getValue("success");
            if (!Boolean.TRUE.equals(success)) {
                System.exit(1);
            }
        }

    }

    public Exception e = null;

    @Override
    public void run() {
        try (Interpreter interp = new SubInterpreter(
                new JepConfig().addIncludePaths(".")
                        .addSharedModules("xml.etree.ElementTree"))) {
            interp.eval("import xml.etree.ElementTree");
            interp.eval("t = xml.etree.ElementTree.ElementTree");
        } catch (Exception e) {
            this.e = e;
        }
    }

}
