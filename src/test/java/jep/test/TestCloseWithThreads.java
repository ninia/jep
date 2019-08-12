package jep.test;

import java.util.concurrent.CountDownLatch;

import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;
import jep.SubInterpreter;

/**
 * A test to verify calling close with active python threads does not crash.
 * 
 * @author bsteffen
 */
public class TestCloseWithThreads {

    public static void main(String[] args) throws Exception {
        boolean closeExceptionCaught = false;
        try (Interpreter interp = new SubInterpreter(new JepConfig())) {
            CountDownLatch start = new CountDownLatch(1);
            CountDownLatch done = new CountDownLatch(1);
            interp.set("start", start);
            interp.set("done", done);
            // await is a keyword in python
            interp.eval(
                    "def run(start, done):\n  start.countDown()\n  getattr(done,'await')()");
            interp.eval("import threading");
            interp.eval("t = threading.Thread(target=run, args=(start, done))");
            interp.eval("t.start()");
            start.await();
            try {
                interp.close();
            } catch (JepException e) {
                closeExceptionCaught = true;
            }
            done.countDown();
            interp.eval("t.join()");
            if (!closeExceptionCaught) {
                throw new Exception("Close worked.");
            }
        }
    }
}
