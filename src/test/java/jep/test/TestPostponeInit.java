package jep.test;

import jep.Jep;
import jep.JepConfig;

import java.lang.Boolean;
import java.lang.Exception;
import java.lang.StringBuilder;
import java.lang.System;
import java.lang.Thread;

/**
 * Test that we can postpone the initialization of the Jep instance
 *
 * Created: June 2019
 *
 * @author Aitor Hernandez
 */
public class TestPostponeInit extends Thread{

    public static void main(String[] args) throws Throwable{
        TestPostponeInit[] t = new TestPostponeInit[16];
        for(int i = 0 ; i < t.length ; i += 1){
            t[i] = new TestPostponeInit();
            t[i].start();
        }
        for(int i = 0 ; i < t.length ; i += 1){
            t[i].join();
            if( t[i].e != null ){
                throw t[i].e;
            }
        }
        JepConfig config = new JepConfig();
        boolean useSubinterpreter = true;
        boolean postponeInit = true;

        try(Jep jep = new Jep(config, postponeInit)){
            if (jep.isInitialized() == true){
                System.exit(1);
            }
            jep.init();
            jep.eval("success = True");
            Thread.sleep(10);
            Object success = jep.getValue("success");
            if(!Boolean.TRUE.equals(success)){
                System.exit(1);
            }
        }

    }

    public Exception e = null;

    @Override
    public void run() {
        try(Jep jep = new Jep(new JepConfig(), true)){
            if (jep.isInitialized() == true){
                throw new Exception("Jep is already initialized!");
            }
            jep.eval("success = True");
            Object success = jep.getValue("success");
            if(!Boolean.TRUE.equals(success)){
                throw new Exception("'success' variable not set in python");
            }
        } catch (Exception e){
            this.e = e;
        }
    }

}
