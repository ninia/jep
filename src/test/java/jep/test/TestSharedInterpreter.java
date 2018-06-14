package jep.test;

import jep.Jep;
import jep.JepConfig;
import jep.JepException;
import jep.SharedInterpreter;

import java.lang.Exception;
import java.lang.Thread;
import java.lang.IllegalStateException;

/**
 * Tests that jep works without using sub-interpreters and verified that
 * all modules are shared between interpreters on different threads.
 *
 * Created: Jan 2018
 *
 * @author Ben Steffensmeier
 */
public class TestSharedInterpreter extends Thread{

    public static void main(String[] args) throws Throwable{
        TestSharedInterpreter[] t = new TestSharedInterpreter[4];
        try(Jep jep = new SharedInterpreter()){
            jep.eval("import sys");
            jep.set("n", t.length);
            jep.eval("sys.sharedTestThing = [None] * n");
        }

        for(int i = 0 ; i < t.length ; i += 1){
            t[i] = new TestSharedInterpreter(i);
            t[i].start();
        }
        for(int i = 0 ; i < t.length ; i += 1){
            t[i].join();
            if( t[i].e != null ){
                throw t[i].e;
            }
        }
        try(Jep jep = new  SharedInterpreter()) {
            for(int i = 0 ; i < t.length ; i += 1){
                jep.eval("import sys");
                jep.set("i", i);
                Boolean b = (Boolean) jep.getValue("sys.sharedTestThing[i]");
                if(b.booleanValue() == false){
                    throw new IllegalStateException(i + " failed");
                }
            }
        }
    }

    public Exception e = null;

    public final int index;

    public TestSharedInterpreter(int index){
        this.index = index;
    }

    @Override
    public void run() {
        try(Jep jep = new SharedInterpreter()){
            jep.eval("import sys");
            jep.set("index", index);
            jep.eval("sys.sharedTestThing[index] = True");
        } catch (Exception e){
            this.e = e;
        }
    }

}
