package jep.test.numpy;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import jep.Jep;
import jep.JepException;

/**
 * Tests multiple threads using numpy simultaneously, printing out numpy
 * ndarrays, and closing/disposing interpreters. This demonstrates the issue
 * where numpy's array2string or array_str reference is lost.
 * 
 * Created: Autumn 2013
 * 
 * @author David Gillingham
 * @version $Id$
 */
public class TestNumpyArrayToString {

    private static final int NUM_ITERATIONS = 10;

    public static void main(String[] args) {
        Runnable disposeThread = new Runnable() {

            @Override
            public void run() {
                for (int i = 0; i < NUM_ITERATIONS; i++) {
                    Jep script = null;
                    try {
                        // just give it some work to do before closing it
                        script = new Jep(true);
                        script.eval("import numpy");
                        script.eval("from numpy import *");
                        script.eval("import re, sys, traceback, os");
                        script.eval("a = {'a':1, 'b':2, 'c':3}");
                        Object a = script.getValue("a");
                        a.toString();
                    } catch (Throwable e) {
                        System.out.println("!! Close thread failed!");
                        e.printStackTrace();
                        break;
                    } finally {
                        if (script != null) {
                            script.close();
                        }
                    }
                }

                System.out.println("Close thread complete...");
                return;
            }
        };

        Runnable printThread = new Runnable() {

            @Override
            public void run() {
                Jep script;
                try {
                    script = new Jep(true);
                } catch (JepException e) {
                    e.printStackTrace();
                    return;
                }

                for (int i = 0; i < NUM_ITERATIONS; i++) {
                    try {
                        script.eval("import numpy");
                        script.eval("from numpy import *");
                        script.eval("array = numpy.array([1,2,3], dtype=numpy.float32)");
                        String iterVal = Integer.toString(i + 1);
                        script.eval("print \"iter:\", " + iterVal
                                + ", \"array:\", str(array)");
                        script.eval("print \"iter:\", " + iterVal
                                + ", \"array:\", numpy.array_str(array)");
                        script.eval("reduced = numpy.add.reduce(array)");
                        script.eval("print \"iter:\", " + iterVal
                                + ", \"reduced:\", str(reduced)");
                        script.eval("print \"iter:\", " + iterVal
                                + ", \"reduced:\", numpy.array_str(reduced)");
                    } catch (JepException e) {
                        System.out.println("!! Print thread failed!");
                        e.printStackTrace();
                        break;
                    }
                }

                return;
            }
        };

        ExecutorService execService = Executors.newFixedThreadPool(2);
        execService.submit(disposeThread);
        execService.submit(printThread);
        execService.shutdown();
    }
}