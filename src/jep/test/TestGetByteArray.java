package jep.test;

import java.io.File;
import java.io.FileOutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.Random;

import jep.Jep;

/**
 * A test class for verifying that Jep.getValue_bytearray() is working
 * correctly. Also tests Jep.getValue_floatarray() just to be complete.
 * 
 * 
 * Created: Tue Jul 14 2015
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 * @version $Id$
 */
public class TestGetByteArray {

    protected static final int SIZE = 1024;

    /**
     * @param args
     * @throws Exception
     */
    public static void main(String[] args) throws Exception {
        testGetByteArray();
        testGetFloatArray();
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
            jep = new Jep(false);
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

        System.out.println("byte[] properly retrieved from Jep");
    }

    @SuppressWarnings("deprecation")
    public static void testGetFloatArray() throws Exception {
        File output = File.createTempFile("testFloatArrayGet", ".bin");
        byte[] b = new byte[SIZE * 4];
        FloatBuffer fb = ByteBuffer.wrap(b).order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        Random r = new Random();
        for (int i = 0; i < SIZE; i++) {
            fb.put(r.nextFloat());
        }
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
        float[] f2 = null;
        try {
            jep = new Jep(false);
            jep.eval("f = open('" + output.getAbsolutePath() + "', 'rb')");
            jep.eval("x = f.read()");
            jep.eval("f.close()");
            f2 = jep.getValue_floatarray("x");
        } finally {
            if (jep != null) {
                jep.close();
            }
        }

        if (f2 == null) {
            throw new AssertionError(
                    "float array retrieved from python is null");
        }

        if (fb.capacity() != f2.length) {
            throw new AssertionError(
                    "java and python float arrays have different lengths");
        }

        fb.position(0);
        for (int i = 0; i < f2.length; i++) {
            float f0 = fb.get();
            if (f0 != f2[i]) {
                throw new AssertionError("values at index " + i + " differ");
            }
        }

        System.out.println("float[] properly retrieved from jep");
    }

}
