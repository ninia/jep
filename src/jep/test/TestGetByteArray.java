package jep.test;

import java.io.File;
import java.io.FileOutputStream;
import java.util.Random;

import jep.Jep;

/**
 * A test class for verifying that Jep.getValue_bytearray() is working
 * correctly.
 * 
 * 
 * Created: July 2015
 * 
 * @author Nate Jensen
 */
public class TestGetByteArray {

    protected static final int SIZE = 1024;

    /**
     * @param args
     * @throws Exception
     */
    public static void main(String[] args) throws Exception {
        try {
            testGetByteArray();
        } catch (Throwable t) {
            t.printStackTrace();
            System.exit(1);
        }
        System.exit(0);
    }

    public static void testGetByteArray() throws Exception {
        File output = File.createTempFile("testByteArrayGet", ".bin");
        byte[] b = new byte[SIZE];
        new Random().nextBytes(b);
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(output);
            fos.write(b);
        } finally {
            if (fos != null) {
                fos.close();
            }
        }

        Jep jep = null;
        byte[] b2 = null;
        try {
            jep = new Jep(false, ".");
            jep.eval("f = open('" + output.getAbsolutePath() + "', 'rb')");
            jep.eval("x = f.read()");
            jep.eval("f.close()");
            b2 = jep.getValue_bytearray("x");
        } finally {
            if (jep != null) {
                jep.close();
            }
        }

        if (b2 == null) {
            throw new AssertionError("byte array retrieved from python is null");
        }

        if (b.length != b2.length) {
            throw new AssertionError(
                    "java and python byte arrays have different lengths");
        }

        for (int i = 0; i < b.length; i++) {
            if (b[i] != b2[i]) {
                throw new AssertionError("values at index " + i + " differ");
            }
        }
    }

}
