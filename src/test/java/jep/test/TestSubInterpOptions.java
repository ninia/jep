package jep.test;

import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;
import jep.SubInterpreter;
import jep.SubInterpreterOptions;

public class TestSubInterpOptions {

    /* Set to a non-null value to fail the test */
    private String failure;

    public boolean testForbidFork() {
        SubInterpreterOptions interpOptions = SubInterpreterOptions.legacy().setAllowFork(false);
        JepConfig config = new JepConfig().setSubInterpreterOptions(interpOptions);
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.exec("import os");
            interp.exec("os.fork()");
            failure = "Fork should not be allowed";
            return false;
        } catch (JepException e) {
            if (e.getMessage().contains("fork not supported for isolated subinterpreters")) {
                return true;
            }
            failure = e.getMessage();
            return false;
        }
    }

    public boolean testForbidExec() {
        SubInterpreterOptions interpOptions = SubInterpreterOptions.legacy().setAllowExec(false);
        JepConfig config = new JepConfig().setSubInterpreterOptions(interpOptions);
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.exec("import os");
            interp.exec("os.execv('/bin/ls', [])");
            failure = "Exec should not be allowed";
            return false;
        } catch (JepException e) {
            if (e.getMessage().contains("exec not supported for isolated subinterpreters")) {
                return true;
            }
            failure = e.getMessage();
            return false;
        }
    }

    public boolean testForbidThreads() {
        SubInterpreterOptions interpOptions = SubInterpreterOptions.legacy().setAllowThreads(false);
        JepConfig config = new JepConfig().setSubInterpreterOptions(interpOptions);
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.exec("import threading");
            interp.exec("threading.Thread(target=print).start()");
            failure = "Thread should not be allowed";
            return false;
        } catch (JepException e) {
            if (e.getMessage().contains("thread is not supported for isolated subinterpreters")) {
                return true;
            }
            failure = e.getMessage();
            return false;
        }
    }

    public boolean testForbidDaemonThreads() {
        SubInterpreterOptions interpOptions = SubInterpreterOptions.legacy().setAllowDaemonThreads(false);
        JepConfig config = new JepConfig().setSubInterpreterOptions(interpOptions);
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.exec("import threading");
            interp.exec("threading.Thread(target=print, daemon=True).start()");
            failure = "Daemon Thread should not be allowed";
            return false;
        } catch (JepException e) {
            if (e.getMessage().contains("daemon threads are disabled in this (sub)interpreter")) {
                return true;
            }
            failure = e.getMessage();
            return false;
        }
    }

    public boolean testIsolated() {
        SubInterpreterOptions interpOptions = SubInterpreterOptions.isolated();
        JepConfig config = new JepConfig().setSubInterpreterOptions(interpOptions);
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.exec("from java.lang import Object");
            // I have no way to check if the interpreter is actually isolated so
            // for now just assume it is OK if it is running and trust the API
            return true;
        } catch (JepException e) {
            failure = e.getMessage();
	    System.out.println(failure);
            return false;
        }
    }

    public void runTest() {
        if (!testForbidFork()) {
            return;
        }
        if (!testForbidExec()) {
            return;
        }
        if (!testForbidThreads()) {
            return;
        }
        if (!testForbidDaemonThreads()) {
            return;
        }
        if (!testIsolated()) {
            return;
        }
    }

    public static String test() throws InterruptedException{
        TestSubInterpOptions test = new TestSubInterpOptions();
        Thread t = new Thread(test::runTest);
        t.start();
        t.join();
        return test.failure;
    }


}
