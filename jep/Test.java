package jep;

import java.util.ArrayList;

/**
 * Test.java
 *
 *
 * Created: Fri Apr 30 12:42:58 2004
 *
 * @author <a href="mailto:mrjohnson0@users.sourceforge.net">Mike Johnson</a>
 * @version 1.0
 */
public class Test implements Runnable {

    public Test() {
    } // Test constructor

    
    public void run() {

//         Jep jep = null;
//         try {
//             jep = new Jep();
//         }
//         catch(Exception e) {
//             ;
//         }

        for(int i = 0; i < 1; i++) {
            System.out.println("running i: " + i);
            Jep jep = null;
            try {
                jep = new Jep();
                jep.set("testo", new Test());
                jep.set("test", "value from java.");
                jep.set("testi", i);
                jep.set("testb", true);
                jep.set("testl", 123123122112L);
                jep.set("testd", 123.123D);
                jep.set("testf", 12312.123123F);
                jep.set("testn", (String) null);
                jep.set("testn", (Object) null);

                jep.runScript("test.py");
            }
            catch(Exception e) {
                e.printStackTrace();
                break;
            }
            finally {
                if(jep != null)
                    jep.close();
            }
        }

//         try {
//             while(true)
//                 Thread.sleep(100);
//         }
//         catch(Exception e) {
//         }

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

    public void sendMeSomeStuff(String v, ArrayList a) {
        System.out.println("got some stuff:   v = " + v + " and a = " + a);
    }

    public String callback() {
        return "Hey, you called a Java(tm) method!";
    }
    

    // -------------------------------------------------- fields

    public String stringField = "stringField";
    public boolean booleanField = true;
    public short shortField = 321;
    public int intField = 123;
    public long longField = 9223372036854775807L;
    public double doubleField = 123.123D;
    public float floatField = 3.4028235E38F;
    
    
    // -------------------------------------------------- static fields
    
    public static String staticString = "stringField";
    public static boolean staticBoolean = true;
    public static short staticShort = 321;
    public static int staticInt = 123;
    public static long staticLong = 9223372036854775807L;
    public static double staticDouble = 123.123D;
    public static float staticFloat = 3.4028235E38F;

    
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
    
    
    public static void main(String argv[]) throws Exception {
        
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
