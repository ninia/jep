package jep.test;

import jep.Jep;
import jep.JepConfig;
import jep.JepException;

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
public class TestNoSubInterpreter extends Thread{

    public static void main(String[] args) throws Throwable{
        TestNoSubInterpreter[] t = new TestNoSubInterpreter[4];
        try(Jep jep = new Jep(new JepConfig().setSubInterpreter(false))){
            jep.eval("import sys");
            jep.set("n", t.length);
            jep.eval("sys.sharedTestThing = [None] * n");
        }

        for(int i = 0 ; i < t.length ; i += 1){
            t[i] = new TestNoSubInterpreter(i);
            t[i].start();
        }
        for(int i = 0 ; i < t.length ; i += 1){
            t[i].join();
            if( t[i].e != null ){
                throw t[i].e;
            }
        }
        try(Jep jep = new Jep(new JepConfig().setSubInterpreter(false))){
            for(int i = 0 ; i < t.length ; i += 1){
                jep.eval("import sys");
                jep.set("i", i);
                Boolean b = (Boolean) jep.getValue("sys.sharedTestThing[i]");
                if(b.booleanValue() == false){
                    throw new IllegalStateException(i + " failed");
                }
            }
        }
        try(Jep jep = new Jep(new JepConfig().setSubInterpreter(false).addIncludePaths("."))){
            throw new IllegalStateException("Include Path was supposed to fail");
        }catch(JepException e){
            // This is what we want
        }
        try(Jep jep = new Jep(new JepConfig().setSubInterpreter(false).addSharedModules("datetime"))){
            throw new IllegalStateException("Shared Module was supposed to fail");
        }catch(JepException e){
            // This is what we want
        }
    }

    public Exception e = null;

    public final int index;

    public TestNoSubInterpreter(int index){
        this.index = index;
    }

    @Override
    public void run() {
        try(Jep jep = new Jep(new JepConfig().setSubInterpreter(false))){
            jep.eval("import sys");
            jep.set("index", index);
            jep.eval("sys.sharedTestThing[index] = True");
        } catch (Exception e){
            this.e = e;
        }
    }

}
