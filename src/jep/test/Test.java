package jep.test;

import java.io.*;
import java.util.ArrayList;

import jep.python.*;

import jep.Jep;
import jep.JepException;

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

    
    public static ClassLoader restrictedClassLoader = new ClassLoader() {
            @Override
            public Class<?> loadClass(final String name) throws ClassNotFoundException {
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
        One,
        Two
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
                int[] ia = new int[] { 3 };
                double[] da = new double[] { 2.0 };
                String[] sa  = new String[] { "0" };

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

    // get the jep used for this class
    public Jep getJep() {
        return this.jep;
    }

    public String toString() {
        return "toString(). Thanks for calling Java(tm).";
    }


    public TestEnum getEnum() {
        return TestEnum.One;
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

    public boolean isBooleanField() {
        return booleanField;
    }

    public void setBooleanField(boolean booleanField) {
        this.booleanField = booleanField;
    }

    public byte getByteField() {
        return byteField;
    }

    public void setByteField(byte byteField) {
        this.byteField = byteField;
    }

    public char getCharField() {
        return charField;
    }

    public void setCharField(char charField) {
        this.charField = charField;
    }

    public Class getClassField() {
        return classField;
    }

    public void setClassField(Class classField) {
        this.classField = classField;
    }

    public double getDoubleField() {
        return doubleField;
    }

    public void setDoubleField(double doubleField) {
        this.doubleField = doubleField;
    }

    public float getFloatField() {
        return floatField;
    }

    public void setFloatField(float floatField) {
        this.floatField = floatField;
    }

    public int getIntField() {
        return intField;
    }

    public void setIntField(int intField) {
        this.intField = intField;
    }

    public long getLongField() {
        return longField;
    }

    public void setLongField(long longField) {
        this.longField = longField;
    }

    public short getShortField() {
        return shortField;
    }

    public void setShortField(short shortField) {
        this.shortField = shortField;
    }

    public String getStringField() {
        return stringField;
    }

    public void setStringField(String stringField) {
        this.stringField = stringField;
    }
    
    
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

    public static boolean isStaticBoolean() {
        return staticBoolean;
    }

    public static void setStaticBoolean(boolean staticBoolean) {
        Test.staticBoolean = staticBoolean;
    }

    public static byte getStaticByte() {
        return staticByte;
    }

    public static void setStaticByte(byte staticByte) {
        Test.staticByte = staticByte;
    }

    public static char getStaticChar() {
        return staticChar;
    }

    public static void setStaticChar(char staticChar) {
        Test.staticChar = staticChar;
    }

    public static double getStaticDouble() {
        return staticDouble;
    }

    public static void setStaticDouble(double staticDouble) {
        Test.staticDouble = staticDouble;
    }

    public static float getStaticFloat() {
        return staticFloat;
    }

    public static void setStaticFloat(float staticFloat) {
        Test.staticFloat = staticFloat;
    }

    public static int getStaticInt() {
        return staticInt;
    }

    public static void setStaticInt(int staticInt) {
        Test.staticInt = staticInt;
    }

    public static long getStaticLong() {
        return staticLong;
    }

    public static void setStaticLong(long staticLong) {
        Test.staticLong = staticLong;
    }

    public static short getStaticShort() {
        return staticShort;
    }

    public static void setStaticShort(short staticShort) {
        Test.staticShort = staticShort;
    }

    public static String getStaticString() {
        return staticString;
    }

    public static void setStaticString(String staticString) {
        Test.staticString = staticString;
    }
    
    // -------------------------------------------------- other static methods
    
    public static Object getStaticObject() {
        return new Object();
    }
    
    public static void callStaticVoid() {
        return;
    }

    public static Class getStaticClass() {
        return Thread.currentThread().getClass();
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
                    if(jep != null) {
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
        
        if(t[0] == null) {            
            throw new RuntimeException("Did not throw classloader exception!");
        } else if(!t[0].getMessage().contains("ImportError")) {
            throw t[0];
        }
    }    
    
    public static void main(String argv[]) throws Throwable {
        Jep jep = new Jep();
        try {
            jep.runScript("runtests.py");
        }
        finally {
            jep.close();
        }
    }
} // Test
