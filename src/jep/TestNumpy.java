package jep;

import java.io.File;

/**
 * TestNumpy.java.  Runs a variety of simple tests to verify numpy interactions
 * are working correctly.  run() is called by setup.py test, or you can call the
 * java main() method directly for other tests.
 * 
 * 
 * Created: Wed Apr 08 2015
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 * @version $Id$
 */
public class TestNumpy implements Runnable {

    protected Jep jep = null;

    // set to a high number to test for memory leaks
    private static final int REPEAT = 1; // 0000000;

    private static boolean PRINT = false;

    @Override
    public void run() {
        try {
            File pwd = new File(".");
            /*
             * Anytime you start a new Jep interpreter and import numpy within
             * it, even if you call close() on the interpreter you will leak
             * some native memory. Therefore, for now, do NOT new up the
             * interpreter and close it within the for loop.
             */
            jep = new Jep(false, pwd.getAbsolutePath());
            for (int i = 0; i < REPEAT; i++) {
                testSetAndGet();
            }
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (jep != null)
                jep.close();
        }
    }

    public void testSetAndGet() throws JepException {
        int[] dimensions = new int[] { 4 };

        // test boolean[]
        NDArray<boolean[]> zarray = new NDArray<>(new boolean[] { true, false,
                true, true }, dimensions);
        jep.set("zarray", zarray);
        String z_dtype = (String) jep.getValue("zarray.dtype");
        if (!"bool".equals(z_dtype)) {
            throw new RuntimeException("boolean ndarray set failed, dtype = "
                    + z_dtype);
        }
        NDArray<?> retZ = (NDArray<?>) jep.getValue("zarray");
        if (!zarray.equals(retZ)) {
            throw new RuntimeException("boolean[] before != boolean[] after");
        }
        if (zarray.hashCode() != retZ.hashCode()) {
            throw new RuntimeException(
                    "boolean[].hashCode() before != boolean[].hasCode() after");
        }

        // test byte[]
        NDArray<byte[]> barray = new NDArray<>(new byte[] { 0x10, 0x00, 0x54,
                032 }, dimensions);
        jep.set("barray", barray);
        String b_dtype = (String) jep.getValue("barray.dtype");
        if (!"int8".equals(b_dtype)) {
            throw new RuntimeException("byte ndarray set failed, dtype = "
                    + b_dtype);
        }
        NDArray<?> retB = (NDArray<?>) jep.getValue("barray");
        if (!barray.equals(retB)) {
            throw new RuntimeException("byte[] before != byte[] after");
        }
        if (barray.hashCode() != retB.hashCode()) {
            throw new RuntimeException(
                    "byte[].hashCode() before != byte[].hasCode() after");
        }

        // test short[]
        NDArray<short[]> sarray = new NDArray<>(new short[] { 5, 3, 1, 8 },
                dimensions);
        jep.set("sarray", sarray);
        String s_dtype = (String) jep.getValue("sarray.dtype");
        if (!"int16".equals(s_dtype)) {
            throw new RuntimeException("short ndarray set failed, dtype = "
                    + s_dtype);
        }
        NDArray<?> retS = (NDArray<?>) jep.getValue("sarray");
        if (!sarray.equals(retS)) {
            throw new RuntimeException("short[] before != short[] after");
        }
        if (sarray.hashCode() != retS.hashCode()) {
            throw new RuntimeException(
                    "short[].hashCode() before != short[].hasCode() after");
        }

        // test int[]
        NDArray<int[]> iarray = new NDArray<>(
                new int[] { 547, 232, -675, 101 }, dimensions);
        jep.set("iarray", iarray);
        String i_dtype = (String) jep.getValue("iarray.dtype");
        if (!"int32".equals(i_dtype)) {
            throw new RuntimeException("int ndarray set failed, dtype = "
                    + i_dtype);
        }
        NDArray<?> retI = (NDArray<?>) jep.getValue("iarray");
        if (!iarray.equals(retI)) {
            throw new RuntimeException("int[] before != int[] after");
        }
        if (iarray.hashCode() != retI.hashCode()) {
            throw new RuntimeException(
                    "int[].hashCode() before != int[].hasCode() after");
        }

        // test long[]
        NDArray<long[]> larray = new NDArray<>(new long[] { 62724764L,
                3424637L, 3426734242L, -3429234L }, dimensions);
        jep.set("larray", larray);
        String l_dtype = (String) jep.getValue("larray.dtype");
        if (!"int64".equals(l_dtype)) {
            throw new RuntimeException("long ndarray set failed, dtype = "
                    + l_dtype);
        }
        NDArray<?> retL = (NDArray<?>) jep.getValue("larray");
        if (!larray.equals(retL)) {
            throw new RuntimeException("long[] before != long[] after");
        }
        if (larray.hashCode() != retL.hashCode()) {
            throw new RuntimeException(
                    "long[].hashCode() before != long[].hasCode() after");
        }

        // test float[]
        NDArray<float[]> farray = new NDArray<>(new float[] { 4.32f, -0.0001f,
                349.285f, 3201.0f }, dimensions);
        jep.set("farray", farray);
        String f_dtype = (String) jep.getValue("farray.dtype");
        if (!"float32".equals(f_dtype)) {
            throw new RuntimeException("float ndarray set failed, dtype = "
                    + f_dtype);
        }
        NDArray<?> retF = (NDArray<?>) jep.getValue("farray");
        if (!farray.equals(retF)) {
            throw new RuntimeException("float[] before != float[] after");
        }
        if (farray.hashCode() != retF.hashCode()) {
            throw new RuntimeException(
                    "float[].hashCode() before != float[].hasCode() after");
        }

        // test double[]
        NDArray<double[]> darray = new NDArray<>(new double[] { 0.44321,
                0.00015, -9.34278, 235574.53 }, dimensions);
        jep.set("darray", darray);
        String d_dtype = (String) jep.getValue("darray.dtype");
        if (!"float64".equals(d_dtype)) {
            throw new RuntimeException("double ndarray set failed, dtype = "
                    + d_dtype);
        }
        NDArray<?> retD = (NDArray<?>) jep.getValue("darray");
        if (!darray.equals(retD)) {
            throw new RuntimeException("double[] before != double[] after");
        }
        if (darray.hashCode() != retD.hashCode()) {
            throw new RuntimeException(
                    "double[].hashCode() before != double[].hasCode() after");
        }

        if (PRINT) {
            System.out.println("NDArray get/set checked out OK");
        }
    }

    public NDArray<int[]> testArgAndReturn(NDArray<int[]> array) {
        int[] data = array.getData();
        int[] newData = new int[data.length];
        for (int i = 0; i < data.length; i++) {
            newData[i] = data[i] + 5;
        }

        return new NDArray<int[]>(newData, array.getDimensions());
    }

    // should only be called from main(), not from python unittests
    public void runPythonSide() {
        try {
            File pwd = new File("tests");
            /*
             * Anytime you start a new Jep interpreter and import numpy within
             * it, even if you call close() on the interpreter you will leak
             * some native memory. Therefore, for now, do NOT new up the
             * interpreter and close it within the for loop.
             */
            jep = new Jep(true, pwd.getAbsolutePath());
            jep.eval("import test_numpy");
            jep.eval("v = test_numpy.TestNumpy('testArgReturn')");
            jep.eval("v.setUp()");
            for (int i = 0; i < REPEAT; i++) {
                jep.eval("v.testArgReturn()");
            }
            for (int i = 0; i < REPEAT; i++) {
                jep.eval("v.testMultiDimensional()");
            }
            for (int i = 0; i < REPEAT; i++) {
                jep.eval("v.testArrayParams()");
            }
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (jep != null)
                jep.close();
        }
    }

    public boolean callBooleanMethod(boolean[] array) {
        return array != null;
    }

    public boolean callByteMethod(byte[] array) {
        return array != null;
    }

    public boolean callShortMethod(short[] array) {
        return array != null;
    }

    public boolean callIntMethod(int[] array) {
        return array != null;
    }

    public boolean callLongMethod(long[] array) {
        return array != null;
    }

    public boolean callFloatMethod(float[] array) {
        return array != null;
    }

    public boolean callDoubleMethod(double[] array) {
        return array != null;
    }

    public void testNDArraySafety() {
        float[][] f = new float[15][];
        int[] dims = new int[] { 15, 20 };
        try {
            NDArray<float[][]> ndarray = new NDArray<>(f, dims);
            ndarray.getDimensions();
            throw new RuntimeException(
                    "NDArray should have failed instantiation");
        } catch (IllegalArgumentException e) {
            if (PRINT) {
                e.printStackTrace();
            }
        }

        float[] d = new float[200];
        try {
            NDArray<float[]> ndarray = new NDArray<>(d, dims);
            ndarray.getDimensions();
            throw new RuntimeException(
                    "NDArray should have failed instantiation");
        } catch (IllegalArgumentException e) {
            if (PRINT) {
                e.printStackTrace();
            }
        }
    }

    /**
     * This main() is for running the tests from Java. If running from the tests
     * from python, use python setup.py test.
     * 
     * @param args
     */
    public static void main(String[] args) {        
        TestNumpy test = new TestNumpy();
        test.run();
        test.runPythonSide();
        test.testNDArraySafety();
    }

}
