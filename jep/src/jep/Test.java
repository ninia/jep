package jep;

import java.io.*;
import java.util.ArrayList;

import jep.python.*;


/**
 * Test.java
 *
 *
 * Created: Fri Apr 30 12:42:58 2004
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$
 */
public class Test implements Runnable {
    private Jep jep = null;
    private boolean testEval = false;

    public Test() {
    } // Test constructor


    public Test(boolean testEval) {
        this.testEval = testEval;
    }

    
    public void run() {

        for(int i = 0; i < 1; i++) {
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

                boolean[] ab = new boolean[10];
                ab[1] = true;
                jep.set("testab", ab);

                double[] ad = new double[10];
                ad[1] = 1.7976931348623157E308D;
                jep.set("testad", ad);

                PyModule amod = jep.createModule("amod");
                amod.set("testab", ab);
                amod.set("testad", ad);

                if(!this.testEval)
                    jep.runScript("test.py");
                else {
                    BufferedReader buf = new BufferedReader(
                        new FileReader("test.py"));

                    String line = null;
                    while((line = buf.readLine()) != null) {
                        if(line.trim().startsWith("#"))
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

                Object ret = jep.invoke("testMethod", "method called from Java");
                System.out.println("testMethod ret:   " + ret);

                System.out.println("Test get object: " + jep.getValue("testo"));
                System.out.println("Test get string: " + jep.getValue("test"));
                System.out.println("Test get int: " +
                                   ((Integer) jep.getValue("testi")).intValue());
                System.out.println("Test get boolean: " + (Boolean) jep.getValue("testb"));
                System.out.println("Test get long: " + (Long) jep.getValue("testl"));
                System.out.println("Test get double: " + (Float) jep.getValue("testd"));
                System.out.println("Test get float: " + (Float) jep.getValue("testf"));
                System.out.println("Test get short: " + (Integer) jep.getValue("testy"));
                System.out.println("Test get null: " + jep.getValue("testn"));
                System.out.println("Test get class: " + (Class) jep.getValue("testz"));

                jep.eval("testmap = {'blah': 'har'}");
                System.out.println("Test get Python object: " + jep.getValue("testmap"));

                System.out.print("get unknown val:  ");

                try {
                    System.out.println(jep.getValue("_asdf"));
                    System.out.println("whoops");
                }
                catch(JepException e) {
                    System.out.println(e.getMessage());
                }
            }
            catch(Throwable t) {
                System.out.println("Java caught error:");
                t.printStackTrace();
                break;
            }
            finally {
                System.out.println("**** close me");
                if(jep != null)
                    jep.close();
            }
        }
    }

    protected void finalize() {
        System.out.println("test instance finalized, you should see this " +
                           "if the reference counting worked...");
    }

    // get the jep used for this class
    public Jep getJep() {
        return this.jep;
    }

    public String toString() {
        return "toString(). Thanks for calling Java(tm).";
    }

    
    public int getInt() {
        return 2147483647;
    }

    public byte getByte() {
        return 123;
    }

    public char getChar() {
        return 'c';
    }

    public short getShort() {
        return 321;
    }

    public long getLong() {
        return 9223372036854775807L;
    }

    public double getDouble() {
        return 1.7976931348623157E308D;
    }

    public float getFloat() {
        return 3.4028235E38F;
    }

    public Integer getInteger() {
        return new Integer(-2147483648);
    }

    public Long getClassLong() {
        return new Long(9223372036854775807L);
    }

    public Double getClassDouble() {
        return new Double(4.9E-324D);
    }

    public Float getClassFloat() {
        return new Float(3.4028235E38F);
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
        return new String[][] {
            new String[] { "one", "two" },
            new String[] { "one", "two" }
        };
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
        if(p == null)
            throw new NullPointerException("p is null?");
        for(int i = 0; i < p.length; i++)
            System.out.println("                  " +
                               "array[" + i + "] = " + p[i]);
    }

    public void sendIntArray(int p[]) {
        if(p == null)
            throw new NullPointerException("p is null?");
        for(int i = 0; i < p.length; i++)
            System.out.println("                  " +
                               "array[" + i + "] = " + p[i]);
    }

    public void sendMeSomeStuff(String v, ArrayList a) {
        System.out.println("got some stuff:   v = " + v + " and a = " + a);
    }

    public String callback() {
        return "Hey, you called a Java(tm) method!";
    }
    

    // -------------------------------------------------- fields

    public String stringField = "a stringField";
    public boolean booleanField = true;
    public short shortField = 321;
    public int intField = 123;
    public long longField = 9223372036854775807L;
    public double doubleField = 123.123D;
    public float floatField = 3.4028235E38F;
    public byte byteField = 43;
    public char charField = 'c';
    public Class classField = this.getClass();
    
    
    // -------------------------------------------------- static fields
    
    public static String staticString = "stringField";
    public static boolean staticBoolean = true;
    public static short staticShort = 321;
    public static int staticInt = 123;
    public static long staticLong = 9223372036854775807L;
    public static double staticDouble = 123.123D;
    public static float staticFloat = 3.4028235E38F;
    public static byte staticByte = 125;
    public static char staticChar = 'j';
    public static Class staticClass = Thread.currentThread().getClass();

    
    // -------------------------------------------------- static methods
    
    public static String getStaticString() {
        return "a static string.";
    }
    
    public static boolean getStaticBoolean() {
        return false;
    }
    
    public static int getStaticInt() {
        return 123;
    }

    public static short getStaticShort() {
        return 321;
    }

    public static long getStaticLong() {
        return 9223372036854775807L;
    }
    
    public static double getStaticDouble() {
        return 123123213.123D;
    }

    public static float getStaticFloat() {
        return 12312.123F;
    }

    public static Object getStaticObject() {
        return new Object();
    }
    
    public static void callStaticVoid() {
        return;
    }

    public static byte getStaticByte() {
        return 23;
    }

    public static char getStaticChar() {
        return 'b';
    }
    
    public static Class getStaticClass() {
        return Thread.currentThread().getClass();
    }
    
    public static void main(String argv[]) throws Throwable {
        
        if(argv.length < 1) {
            new Thread(new Test()).start();
            new Thread(new Test()).start();
        }
        else {
            int count = Integer.parseInt(argv[0]);
            for(int i = 0; i < count; i++)
                new Thread(new Test()).start();
        }

        new Test().run();
        
        System.gc();
    }

} // Test
