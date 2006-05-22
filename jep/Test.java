package jep;

import java.util.ArrayList;
import java.io.File;

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

    public Test() {
    } // Test constructor

    
    public void run() {

        for(int i = 0; i < 1; i++) {
            System.out.println("running i: " + i);
            Jep jep = null;
            
            try {
                File pwd = new File(".");

                jep = new Jep(false, pwd.getAbsolutePath());
                jep.set("testo", new Test());
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

                jep.runScript("test.py");

                System.out.print("get unknown val:  ");
                System.out.println(jep.getValue("_asdf"));
            }
            catch(Throwable t) {
                System.out.println("Java caught error:");
                t.printStackTrace();
                break;
            }
            finally {
                System.out.println("**** close me");
                jep.close();
            }
        }
    }

    protected void finalize() {
        System.out.println("test instance finalized, you should see this " +
                           "if the reference counting worked...");
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
        ArrayList ret = new ArrayList();
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
