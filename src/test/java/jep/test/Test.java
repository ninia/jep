package jep.test;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;

import jep.Interpreter;
import jep.Jep;
import jep.JepConfig;
import jep.JepException;
import jep.SubInterpreter;

/**
 * Test.java
 * 
 * Created: April 2004
 * 
 * @author Mike Johnson
 */
public class Test implements Runnable {

    private Interpreter interp = null;

    private boolean testEval = false;

    public static ClassLoader restrictedClassLoader = new ClassLoader() {
        @Override
        public Class<?> loadClass(final String name)
                throws ClassNotFoundException {
            if (name.startsWith("java.io.")) {
                throw new ClassNotFoundException("restricted class: " + name);
            }
            return super.loadClass(name);
        }
    };

    public Test() {
    }

    public Test(boolean testEval) {
        this.testEval = testEval;
    }

    public static enum TestEnum {
        One, Two
    }

    @Override
    public void run() {

        for (int i = 0; i < 1; i++) {
            System.out.println("running i: " + i);

            try {
                File pwd = new File(".");

                this.interp = new SubInterpreter(
                        new JepConfig().addIncludePaths(pwd.getAbsolutePath()));
                interp.set("testo", this);
                interp.set("test", "value from java.");
                interp.set("testi", i);
                interp.set("testb", true);
                interp.set("testl", 123123122112L);
                interp.set("testd", 123.123D);
                interp.set("testf", 12312.123123F);
                interp.set("testy", 127);
                interp.set("testc", 't');
                interp.set("testn", (String) null);
                interp.set("testn", (Object) null);
                interp.set("testz", this.getClass());

                // arrays
                int[] ia = new int[] { 3 };
                double[] da = new double[] { 2.0 };
                String[] sa = new String[] { "0" };

                interp.eval("def manip(li, val):\n\tli[0]=val\n\tli.commit()");
                interp.invoke("manip", ia, 1);
                interp.invoke("manip", da, 1.0);
                interp.invoke("manip", sa, "1");

                System.out.println(ia[0]);
                System.out.println(da[0]);
                System.out.println(sa[0]);

                interp.set("x", da);
                assert ((double[]) interp.getValue("x"))[0] == 1.0;

                boolean[] ab = new boolean[10];
                ab[1] = true;
                interp.set("testab", ab);

                double[] ad = new double[10];
                ad[1] = 1.7976931348623157E308D;
                interp.set("testad", ad);

                if (!this.testEval)
                    interp.runScript("test.py");
                else {
                    BufferedReader buf = new BufferedReader(
                            new FileReader("test.py"));

                    String line = null;
                    while ((line = buf.readLine()) != null) {
                        if (line.trim().startsWith("#"))
                            continue;

                        System.out.println("Running line: " + line);
                        interp.eval(line);
                    }

                    buf.close();
                }

                interp.invoke("testMethod", true);
                interp.invoke("testMethod", 123);
                interp.invoke("testMethod", 112L);
                interp.invoke("testMethod", 112.23D);
                interp.invoke("testMethod", 112.2312331F);
                interp.invoke("testMethod", (byte) 211);
                interp.invoke("testMethod", 't');

                Object ret = interp.invoke("testMethod",
                        "method called from Java");
                System.out.println("testMethod ret:   " + ret);

                System.out.println(
                        "Test get object: " + interp.getValue("testo"));
                System.out
                        .println("Test get string: " + interp.getValue("test"));
                System.out.println("Test get int: "
                        + ((Integer) interp.getValue("testi")).intValue());
                System.out.println(
                        "Test get boolean: " + interp.getValue("testb"));
                System.out
                        .println("Test get long: " + interp.getValue("testl"));
                System.out.println(
                        "Test get double: " + interp.getValue("testd"));
                System.out
                        .println("Test get float: " + interp.getValue("testf"));
                System.out
                        .println("Test get short: " + interp.getValue("testy"));
                System.out
                        .println("Test get null: " + interp.getValue("testn"));
                System.out
                        .println("Test get class: " + interp.getValue("testz"));

                interp.eval("testmap = {'blah': 'har'}");
                System.out.println("Test get Python object: "
                        + interp.getValue("testmap"));

                System.out.print("get unknown val:  ");

                try {
                    System.out.println(interp.getValue("_asdf"));
                    System.out.println("whoops");
                } catch (JepException e) {
                    System.out.println(e.getMessage());
                }
            } catch (Throwable t) {
                System.out.println("Java caught error:");
                t.printStackTrace();
                break;
            } finally {
                System.out.println("**** close me");
                if (interp != null) {
                    try {
                        interp.close();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }

    // get the jep used for this class
    public Interpreter getJep() {
        return this.interp;
    }

    @Override
    public String toString() {
        return "toString(). Thanks for calling Java(tm).";
    }

    public TestEnum getEnum() {
        return TestEnum.One;
    }

    public ArrayList getObject() {
        ArrayList<String> ret = new ArrayList<String>();
        ret.add("list 0");
        return ret;
    }

    public String[] getStringArray() {
        return new String[] { "one", "two" };
    }

    public String[][] getStringStringArray() {
        return new String[][] { new String[] { "one", "two" },
                new String[] { "one", "two" } };
    }

    public String[] setStringArray(String[] array) {
        return array;
    }

    public int[] getIntArray() {
        return new int[] { 1, 2 };
    }

    public boolean[] getBooleanArray() {
        return new boolean[] { false, true };
    }

    public short[] getShortArray() {
        return new short[] { 123, 123 };
    }

    public float[] getFloatArray() {
        return new float[] { 123.123F, 123.123F };
    }

    public Test[] getObjectArray() {
        return new Test[] { new Test(), new Test() };
    }

    public void sendObjectArray(Object p[]) {
        if (p == null)
            throw new NullPointerException("p is null?");
        for (int i = 0; i < p.length; i++)
            System.out.println(
                    "                  " + "array[" + i + "] = " + p[i]);
    }

    public void sendIntArray(int p[]) {
        if (p == null)
            throw new NullPointerException("p is null?");
        for (int i = 0; i < p.length; i++)
            System.out.println(
                    "                  " + "array[" + i + "] = " + p[i]);
    }

    public void sendMeSomeStuff(String v, ArrayList a) {
        System.out.println("got some stuff:   v = " + v + " and a = " + a);
    }

    public String callback() {
        return "Hey, you called a Java(tm) method!";
    }

    public Object testObjectPassThrough(Object bool) {
        return bool;
    }

    public static void callStaticVoid() {
        return;
    }

    public String[] testAllVarArgs(String... args) {
        return args;
    }

    public String[] testMixedVarArgs(String regArg1, String regArg2,
            String... args) {
        String[] result = new String[args.length + 2];
        result[0] = regArg1;
        result[1] = regArg2;
        System.arraycopy(args, 0, result, 2, args.length);
        return result;
    }

    public static Object[] test20Args(Object arg1, Object arg2, Object arg3,
            Object arg4, Object arg5, Object arg6, Object arg7, Object arg8,
            Object arg9, Object arg10, Object arg11, Object arg12, Object arg13,
            Object arg14, Object arg15, Object arg16, Object arg17,
            Object arg18, Object arg19, Object arg20) {
        return new Object[] { arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,
                arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17,
                arg18, arg19, arg20 };
    }

    public static void testRestrictedClassLoader() throws Throwable {
        final Throwable[] t = new Throwable[1];
        Thread thread = new Thread(new Runnable() {

            @Override
            public void run() {
                Jep interp = null;
                try {
                    JepConfig cfg = new JepConfig()
                            .setClassLoader(restrictedClassLoader);
                    interp = new SubInterpreter(cfg);
                    interp.eval("from java.io import File");
                } catch (Throwable th) {
                    t[0] = th;
                } finally {
                    if (interp != null) {
                        try {
                            interp.close();
                        } catch (Exception e) {
                            throw new RuntimeException(
                                    "Error closing Jep instance", e);
                        }
                    }
                    synchronized (Test.class) {
                        Test.class.notify();
                    }
                }
            }
        });

        synchronized (Test.class) {
            thread.start();
            try {
                Test.class.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        if (t[0] == null) {
            throw new RuntimeException("Did not throw classloader exception!");
        } else if (!t[0].getMessage().contains("ImportError")) {
            throw t[0];
        }
    }

    public static void main(String argv[]) throws Throwable {
        Interpreter interp = new SubInterpreter();
        try {
            interp.runScript("runtests.py");
        } finally {
            interp.close();
        }
    }
} // Test
