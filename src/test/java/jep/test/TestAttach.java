package jep.test;

import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.function.Supplier;

import jep.Interpreter;
import jep.SharedInterpreter;
import jep.SubInterpreter;
import jep.python.PyCallable;

/**
 * Test the functionality of Interreter.useThread().
 */
public class TestAttach {

    /**
     * On construction this creates a new Interpreter on a background thread and
     * sets up some state that can be shared with other threads. Each of the
     * test methods should be called from other threads to use the shared state
     * to increment a counter in the main thread. When the test is done calling
     * finish() will stop the background interpreter and verify that the test
     * methods on other threads have incremented the counter the correct number
     * of times.
     */
    private static class BackgroundInterpreter extends Thread {

        private final Supplier<Interpreter> interpFactory;

        private final CyclicBarrier sync;

        private Interpreter interp;

        private PyCallable increment;

        private int finalCount;

        public BackgroundInterpreter(Supplier<Interpreter> interpFactory)
                throws InterruptedException, BrokenBarrierException,
                TimeoutException {
            this.interpFactory = interpFactory;
            this.sync = new CyclicBarrier(2);
            this.start();
            this.sync.await(5, TimeUnit.SECONDS);
        }

        @Override
        public void run() {
            this.interp = interpFactory.get();
            interp.exec("from types import ModuleType");
            interp.exec("import sys");
            interp.exec("testModule = ModuleType('use_thread_test')");
            interp.exec("sys.modules['use_thread_test'] = testModule");
            interp.exec("testModule.count = 0");
            interp.exec("def increment():\n  testModule.count += 1");
            interp.exec("testModule.increment = increment");
            this.increment = interp.getValue("increment", PyCallable.class);
            try {
                /* signal to the constructor that initialization is done. */
                this.sync.await(5, TimeUnit.SECONDS);
                /* wait for finish() */
                this.sync.await(5, TimeUnit.SECONDS);
            } catch (TimeoutException | InterruptedException
                    | BrokenBarrierException e) {
                throw new RuntimeException(e);
            }
            this.finalCount = interp.getValue("testModule.count",
                    Integer.class);
            this.interp.close();
        }

        /*
         * Test that a PyCallable created from the main interpreter is usable in
         * a new thread.
         */
        public void testSharedPyCallable() {
            try (Interpreter interp = this.interp.attach(false)) {
                increment.call();
            }
        }

        /*
         * Test that when globals are shared a function in globals can be called
         * in a new thread.
         */
        public void testSharedGlobals() {
            try (Interpreter interp = this.interp.attach(true)) {
                interp.exec("increment()");
            }
        }

        /*
         * Test that modules can be used from a different thread.
         */
        public void testSharedModules() {
            try (Interpreter interp = this.interp.attach(false)) {
                boolean incrementDefined = interp
                        .getValue("'increment' in globals()", Boolean.class);
                if (incrementDefined) {
                    throw new IllegalStateException(
                            "increment should not be defined.");
                }
                interp.exec("from use_thread_test import increment");
                interp.exec("increment()");
            }
        }

        /**
         * Close the interpreter and verify the correct number of tests ran.
         * This verifies the count from the background thread and also attaches
         * the current thread to the background interpreter and re-verifies the
         * count after the background interpreter has closed to ensure that
         * attached interpreters are functional after the original interpreter
         * closed.
         * 
         * @param expectedCount
         * @throws InterruptedException
         * @throws BrokenBarrierException
         * @throws TimeoutException
         */
        public void finish(int expectedCount) throws InterruptedException,
                BrokenBarrierException, TimeoutException {
            try (Interpreter interp = this.interp.attach(true)) {
                this.sync.await(5, TimeUnit.SECONDS);
                this.join();
                if (finalCount != expectedCount) {
                    throw new IllegalStateException(
                            "Count does not match. Expected: " + expectedCount
                                    + " Actual: " + finalCount);
                }
                int finalCount = interp.getValue("testModule.count",
                        Integer.class);
                if (finalCount != expectedCount) {
                    throw new IllegalStateException(
                            "Count after close does not match. Expected: "
                                    + expectedCount + " Actual: " + finalCount);
                }
            }
        }

    }

    public static void main(String[] args) throws InterruptedException,
            BrokenBarrierException, TimeoutException {
        BackgroundInterpreter shared = new BackgroundInterpreter(
                SharedInterpreter::new);
        BackgroundInterpreter sub1 = new BackgroundInterpreter(
                SubInterpreter::new);
        BackgroundInterpreter sub2 = new BackgroundInterpreter(
                SubInterpreter::new);
        /*
         * Use an executor to test that there are no concurrency issues with
         * multiple threads.
         */
        ExecutorService executor = Executors.newFixedThreadPool(4);
        /* Test all the interpreters using all 3 test methods. */
        executor.execute(() -> shared.testSharedPyCallable());
        executor.execute(() -> sub1.testSharedPyCallable());
        executor.execute(() -> sub2.testSharedPyCallable());
        executor.execute(() -> shared.testSharedGlobals());
        executor.execute(() -> sub1.testSharedGlobals());
        executor.execute(() -> sub2.testSharedGlobals());
        executor.execute(() -> shared.testSharedModules());
        executor.execute(() -> sub1.testSharedModules());
        executor.execute(() -> sub2.testSharedModules());
        executor.execute(() -> shared.testSharedPyCallable());
        /*
         * Repeat the test on 2 interpreters to get a different expected number
         * and increase concurrency.
         */
        executor.execute(() -> sub1.testSharedPyCallable());
        executor.execute(() -> shared.testSharedGlobals());
        executor.execute(() -> sub1.testSharedGlobals());
        executor.execute(() -> shared.testSharedModules());
        executor.execute(() -> sub1.testSharedModules());
        /* Repeat one more time on only one interpreter. */
        executor.execute(() -> sub1.testSharedPyCallable());
        executor.execute(() -> sub1.testSharedGlobals());
        executor.execute(() -> sub1.testSharedModules());
        executor.shutdown();
        executor.awaitTermination(5, TimeUnit.SECONDS);
        shared.finish(6);
        sub1.finish(9);
        sub2.finish(3);
    }

}
