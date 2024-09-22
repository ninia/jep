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

    /**
     * We can't actually verify the correct malloc is used but we do ned to
     * verify shared modules can't be mixed with isolated malloc.
     */
    public boolean testUseMainObmalloc() {
        SubInterpreterOptions interpOptions = SubInterpreterOptions.legacy().setUseMainObmalloc(false);
        JepConfig config = new JepConfig().setSubInterpreterOptions(interpOptions).addSharedModules("os");
        try (Interpreter interp = new SubInterpreter(config)) {
            failure = "Shared modules can only be used with a shared allocator.";
            return false;
        } catch (JepException e) {
            if (e.getMessage().equals("Shared modules can only be used with a shared allocator.")) {
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
            return false;
        }
    }

    public boolean testIsolatedWithSharedModule() {
        SubInterpreterOptions interpOptions = SubInterpreterOptions.isolated();
        JepConfig config = new JepConfig().setSubInterpreterOptions(interpOptions).addSharedModules("os");
        try (Interpreter interp = new SubInterpreter(config)) {
            failure = "Shared modules cannot be used in isolated interpreters.";
            return false;
        } catch (JepException e) {
            if (e.getMessage().equals("Shared modules can only be used with a shared allocator.")) {
                return true;
            }
            failure = e.getMessage();
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
        if (!testUseMainObmalloc()) {
            return;
        }
        if (!testIsolated()) {
            return;
        }
        if (!testIsolatedWithSharedModule()) {
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
