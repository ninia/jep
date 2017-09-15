package jep.test.synchronization;

import java.util.concurrent.atomic.AtomicInteger;

import jep.Jep;
import jep.JepConfig;
import jep.JepException;

/**
 * Tests that you can lock a PyJObject from within Python just like a
 * synchronized(object) {...} block in Java.
 * 
 * The test aggressively uses one lock object and uses multiple threads from
 * both Python and Java synchronizing against the same lock object at the same
 * time. If the underlying Jep code is not locking properly, then the value of
 * the AtomicInteger will not match what is expected and/or the test will
 * deadlock.
 * 
 * @author Nate Jensen
 * @since 3.7
 */
public class TestCrossLangSync {

    private static final int THREADS_PER_LANG = 16;

    protected final AtomicInteger atomicInt = new AtomicInteger(0);

    protected Object lock = new Object();
    // protected Object lock = TestCrossLangSync.class;

    private static final String PY_CODE = new StringBuilder(
            "def doIt(obj, atom, count):\n")
                    .append("    with obj.synchronized():\n")
                    .append("        startValue = atom.get()\n")
                    .append("        for i in range(1, count):\n")
                    .append("            atom.getAndIncrement()\n")
                    .append("            if atom.get() != startValue + i:\n")
                    .append("                raise ValueError(str(atom.get()) + ' != ' + str(startValue + i))\n")
                    .toString();

    private abstract class TestThread extends Thread {

        protected int count;

        protected Exception e;

        protected TestThread(int count) {
            this.count = count;
        }
    }

    private class PyThread extends TestThread {

        protected PyThread(int count) {
            super(count);
        }

        @Override
        public void run() {
            try {
                try (Jep jep = new Jep(new JepConfig().addIncludePaths("."))) {
                    jep.eval(PY_CODE);
                    jep.set("lock", lock);
                    jep.set("atom", atomicInt);
                    jep.eval("doIt(lock, atom, " + count + ")");
                } catch (JepException e) {
                    throw new RuntimeException("Synchronization failed", e);
                }
            } catch (Exception e) {
                this.e = e;
            }
        }
    }

    private class JavaThread extends TestThread {

        protected JavaThread(int count) {
            super(count);
        }

        @Override
        public void run() {
            try {
                synchronized (lock) {
                    int startValue = atomicInt.get();
                    for (int i = 1; i < count; i++) {
                        atomicInt.getAndIncrement();
                        if (atomicInt.get() != (startValue + i)) {
                            throw new RuntimeException(
                                    "Synchronization failed " + atomicInt.get()
                                            + " != " + startValue + i);
                        }
                    }
                }
            } catch (Exception e) {
                this.e = e;
            }
        }
    }

    public JavaThread buildJavaThread(int count) {
        return new JavaThread(count);
    }

    public PyThread buildPythonThread(int count) {
        return new PyThread(count);
    }

    public void runTest() throws Exception {
        TestThread[] tt = new TestThread[THREADS_PER_LANG * 2];

        for (int i = 0; i < THREADS_PER_LANG * 2; i += 2) {
            tt[i] = new PyThread(i * 100);
            tt[i + 1] = new JavaThread(i * 100);
        }

        for (int i = 0; i < THREADS_PER_LANG * 2; i++) {
            tt[i].start();
        }

        for (int i = 0; i < THREADS_PER_LANG * 2; i++) {
            tt[i].join();
            if (tt[i].e != null) {
                throw tt[i].e;
            }
        }
    }

    public static void main(String[] args) throws Exception {
        TestCrossLangSync test = new TestCrossLangSync();
        test.runTest();
    }

}
