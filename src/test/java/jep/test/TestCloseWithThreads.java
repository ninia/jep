package jep.test;

import java.util.concurrent.CountDownLatch;

import jep.Jep;
import jep.JepConfig;
import jep.JepException;

/**
 * A test to verify calling close with active python threads does not crash.
 * 
 * @author bsteffen
 */
public class TestCloseWithThreads {

    public static void main(String[] args) throws Exception {
        boolean closeExceptionCaught = false;
        try (Jep jep = new Jep(new JepConfig())){
            CountDownLatch start = new CountDownLatch(1);
            CountDownLatch done = new CountDownLatch(1);
            jep.set("start", start);
            jep.set("done", done);
            jep.eval("def run(start, done):\n  start.countDown()\n  done.await()");
            jep.eval("import threading");
            jep.eval("t = threading.Thread(target=run, args=(start, done))");
            jep.eval("t.start()");
	    start.await();
            try{
                jep.close();
            }catch (JepException e){
                closeExceptionCaught = true;
            }
	    done.countDown();
            if (!closeExceptionCaught){
                throw new Exception("Close worked.");
            }
        }
    }
}
