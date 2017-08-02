package jep.test;

import jep.Jep;
import jep.JepConfig;

import java.lang.Boolean;
import java.lang.Exception;
import java.lang.StringBuilder;
import java.lang.System;
import java.lang.Thread;

/**
 * Tests that shared modules can be imported simultaneously on multiple threads.
 * Python 3 requires careful synchronization to ensure that uninitialized
 * modules are not inadvertantly shared.
 * 
 * This test relies on multiple threads importing at the same time. If the
 * OS thread scheduling gets lucky it might pass even if there are problems.
 * We just assume that over the course of a release if it breaks eventually
 * someone will see a failed test.
 *
 * Created: August 2016
 *
 * @author Ben Steffensmeier
 */
public class TestSharedModulesThreads extends Thread{

    public static void main(String[] args) throws Throwable{
        TestSharedModulesThreads[] t = new TestSharedModulesThreads[16];
        for(int i = 0 ; i < t.length ; i += 1){
            t[i] = new TestSharedModulesThreads();
            t[i].start();
        }
        for(int i = 0 ; i < t.length ; i += 1){
            t[i].join();
            if( t[i].e != null ){
                throw t[i].e;
            }
        }
        /* 
         * Ensure that the shared moduler import hooks don't interfere with pure
         * python threading. This use case is not well supported but it mostly
         * works so this just checks for obvious flaws. 
         */
        try(Jep jep = new Jep(new JepConfig().addIncludePaths(".")
                                             .addSharedModules("xml.etree.ElementTree"))){
            jep.eval("import threading");
            jep.eval("success = False");
            StringBuilder testFunction = new StringBuilder();
            testFunction.append("def testFunction():\n");
            testFunction.append("    global success\n");
            testFunction.append("    import struct\n");
            testFunction.append("    success = True");
            jep.eval(testFunction.toString());
            jep.eval("t = threading.Thread(target=testFunction)");
            jep.eval("t.daemon = True");
            jep.eval("t.start()");
            jep.eval("t.join()");
            Object success = jep.getValue("success");
            if(!Boolean.TRUE.equals(success)){
                System.exit(1);
            }
        }

    }

    public Exception e = null;

    @Override
    public void run() {
        try(Jep jep = new Jep(new JepConfig().addIncludePaths(".")
                                             .addSharedModules("xml.etree.ElementTree"))){
            jep.eval("import xml.etree.ElementTree");
            jep.eval("t = xml.etree.ElementTree.ElementTree");
        } catch (Exception e){
            this.e = e;
        }
    }

}
