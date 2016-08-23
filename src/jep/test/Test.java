package jep.test;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;

import jep.Jep;
import jep.JepException;
import jep.python.PyModule;

/**
 * Test.java
 * 
 * 
 * Created: Fri Apr 30 12:42:58 2004
 * 
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 */
public class Test implements Runnable {

    private Jep jep = null;

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

                this.jep = new Jep(this.testEval, pwd.getAbsolutePath());
                jep.set("testo", this);
                jep.set("test", "value from java.");
                jep.set("testi", i);
                jep.set("testb", true);
                jep.set("testl", 123123122112L);
                jep.set("testd", 123.123D);
                jep.set("testf", 12312.123123F);
                jep.set("testy", 127);
                jep.set("testc", 't');
                jep.set("testn", (String) null);
                jep.set("testn", (Object) null);
                jep.set("testz", this.getClass());

                // arrays
                int[] ia = new int[] { 3 };
                double[] da = new double[] { 2.0 };
                String[] sa = new String[] { "0" };

                jep.eval("def manip(li, val):\n\tli[0]=val\n\tli.commit()");
                jep.invoke("manip", ia, 1);
                jep.invoke("manip", da, 1.0);
                jep.invoke("manip", sa, "1");

                System.out.println(ia[0]);
                System.out.println(da[0]);
                System.out.println(sa[0]);

                jep.set("x", da);
                assert ((double[]) jep.getValue("x"))[0] == 1.0;

                boolean[] ab = new boolean[10];
                ab[1] = true;
                jep.set("testab", ab);

                double[] ad = new double[10];
                ad[1] = 1.7976931348623157E308D;
                jep.set("testad", ad);

                PyModule amod = jep.createModule("amod");
                amod.set("testab", ab);
                amod.set("testad", ad);

                if (!this.testEval)
                    jep.runScript("test.py");
                else {
                    BufferedReader buf = new BufferedReader(new FileReader(
                            "test.py"));

                    String line = null;
                    while ((line = buf.readLine()) != null) {
                        if (line.trim().startsWith("#"))
                            continue;

                        System.out.println("Running line: " + line);
                        jep.eval(line);
                    }

                    buf.close();
                }

                jep.invoke("testMethod", true);
                jep.invoke("testMethod", 123);
                jep.invoke("testMethod", 112L);
                jep.invoke("testMethod", 112.23D);
                jep.invoke("testMethod", 112.2312331F);
                jep.invoke("testMethod", (byte) 211);
                jep.invoke("testMethod", 't');

                Object ret = jep
                        .invoke("testMethod", "method called from Java");
                System.out.println("testMethod ret:   " + ret);

                System.out.println("Test get object: " + jep.getValue("testo"));
                System.out.println("Test get string: " + jep.getValue("test"));
                System.out.println("Test get int: "
                        + ((Integer) jep.getValue("testi")).intValue());
                System.out
                        .println("Test get boolean: " + jep.getValue("testb"));
                System.out.println("Test get long: " + jep.getValue("testl"));
                System.out.println("Test get double: " + jep.getValue("testd"));
                System.out.println("Test get float: " + jep.getValue("testf"));
                System.out.println("Test get short: " + jep.getValue("testy"));
                System.out.println("Test get null: " + jep.getValue("testn"));
                System.out.println("Test get class: " + jep.getValue("testz"));

                jep.eval("testmap = {'blah': 'har'}");
                System.out.println("Test get Python object: "
                        + jep.getValue("testmap"));

                System.out.print("get unknown val:  ");

                try {
                    System.out.println(jep.getValue("_asdf"));
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
                if (jep != null)
                    jep.close();
            }
        }
    }

    // get the jep used for this class
    public Jep getJep() {
        return this.jep;
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
            System.out.println("                  " + "array[" + i + "] = "
                    + p[i]);
    }

    public void sendIntArray(int p[]) {
        if (p == null)
            throw new NullPointerException("p is null?");
        for (int i = 0; i < p.length; i++)
            System.out.println("                  " + "array[" + i + "] = "
                    + p[i]);
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

    public static Object[] test20Args(Object arg1, Object arg2, Object arg3,
            Object arg4, Object arg5, Object arg6, Object arg7, Object arg8,
            Object arg9, Object arg10, Object arg11, Object arg12,
            Object arg13, Object arg14, Object arg15, Object arg16,
            Object arg17, Object arg18, Object arg19, Object arg20) {
        return new Object[] { arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,
                arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17,
                arg18, arg19, arg20 };
    }

    public static void testRestrictedClassLoader() throws Throwable {
        final Throwable[] t = new Throwable[1];
        Thread thread = new Thread(new Runnable() {

            @Override
            public void run() {
                Jep jep = null;
                try {
                    jep = new Jep(true, "", restrictedClassLoader);
                    jep.eval("from java.io import File");
                } catch (Throwable th) {
                    t[0] = th;
                } finally {
                    if (jep != null) {
                        jep.close();
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
        Jep jep = new Jep();
        try {
            jep.runScript("runtests.py");
        } finally {
            jep.close();
        }
    }
} // Test
