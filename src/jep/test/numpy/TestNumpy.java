package jep.test.numpy;

import java.io.File;
import jep.Jep;
import jep.JepException;
import jep.NDArray;

/**
 * TestNumpy.java. Runs a variety of simple tests to verify numpy interactions
 * are working correctly.
 * 
 * 
 * Created: Wed Apr 08 2015
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 * @version $Id$
 */
public class TestNumpy {

    // set to a high number to test for memory leaks
    private static final int REPEAT = 1; // 0000000;

    private static boolean PRINT = true;
    
    /**
     * Calls testSetAndGet(Jep) on a separate thread to avoid threading
     * issues.  Waits for those results before returning.
     * @throws Throwable
     */
    public void testSetAndGet() throws Throwable
    {
        final Throwable[] t = new Throwable[1];
        Thread thread = new Thread(new Runnable() {

            @Override
            public void run() {
                Jep jep = null;
                try {
                    jep = new Jep(true);
                                        
                } catch (Throwable th) {
                    t[0] = th;
                } finally {
                    if(jep != null) {
                        jep.close();
                    }
                    synchronized (TestNumpy.this) {
                        TestNumpy.this.notify();
                    }
                }
            }
        });
        
        synchronized (TestNumpy.this) {
            thread.start();
            try {
                TestNumpy.this.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        
        if(t[0] != null) {
            throw t[0];
        }

    }

    /**
     * Sets NDArrays in a Jep interpreter, then gets them and verifies
     * the conversion in both directions is safe, ie produces a symmetrical
     * object despite a different reference/instance.
     * @param jep
     * @throws JepException
     */
    public void testSetAndGet(Jep jep) throws JepException {
        int[] dimensions = new int[] { 4 };

        // test boolean[]
        NDArray<boolean[]> zarray = new NDArray<boolean[]>(new boolean[] { true, false,
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
        NDArray<byte[]> barray = new NDArray<byte[]>(new byte[] { 0x10, 0x00, 0x54,
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
        NDArray<short[]> sarray = new NDArray<short[]>(new short[] { 5, 3, 1, 8 },
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
        NDArray<int[]> iarray = new NDArray<int[]>(
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
        NDArray<long[]> larray = new NDArray<long[]>(new long[] { 62724764L,
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
        NDArray<float[]> farray = new NDArray<float[]>(new float[] { 4.32f, -0.0001f,
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
        NDArray<double[]> darray = new NDArray<double[]>(new double[] { 0.44321,
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

    /**
     * Called from python to verify that a Java method's return type of NDArray
     * can be auto-converted to a numpy ndarray.
     * @param array
     * @return a copy of the data + 5
     */
    public NDArray<int[]> testArgAndReturn(NDArray<int[]> array) {
        int[] data = array.getData();
        int[] newData = new int[data.length];
        for (int i = 0; i < data.length; i++) {
            newData[i] = data[i] + 5;
        }

        return new NDArray<int[]>(newData, array.getDimensions());
    }


    /**
     * Helper method to support running the main() method to run from Java
     * instead of python
     * @param jep
     */
    public void runFromJava(Jep jep) {
        try {
            jep.eval("import test_numpy");
            jep.eval("v = test_numpy.TestNumpy('testArgReturn')");            
            jep.eval("v.setUp()");            
            for (int i = 0; i < REPEAT; i++) {
                this.testSetAndGet(jep);
            }
            for (int i = 0; i < REPEAT; i++) {
                jep.eval("v.testArgReturn()");
            }
            System.out.println("return NDArray from Java checked out ok");
            for (int i = 0; i < REPEAT; i++) {
                jep.eval("v.testMultiDimensional()");
            }
            System.out.println("multi dimensional arrays checked out ok");
            for (int i = 0; i < REPEAT; i++) {
                jep.eval("v.testArrayParams()");            
            }
            System.out.println("Passing ndarrays to Java method as Java primitive[] checked out ok");
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (jep != null)
                jep.close();
        }
    }

    /**
     * Verifies a numpy.ndarray of bool can automatically convert to a method
     * arg of boolean[]
     * @param array
     * @return true on success
     */
    public boolean callBooleanMethod(boolean[] array) {
        return array != null;
    }

    /**
     * Verifies a numpy.ndarray of byte can automatically convert to a method
     * arg of byte[]
     * @param array
     * @return true on success
     */
    public boolean callByteMethod(byte[] array) {
        return array != null;
    }

    /**
     * Verifies a numpy.ndarray of int16 can automatically convert to a method
     * arg of short[]
     * @param array
     * @return true on success
     */
    public boolean callShortMethod(short[] array) {
        return array != null;
    }

    /**
     * Verifies a numpy.ndarray of int32 can automatically convert to a method
     * arg of int[]
     * @param array
     * @return true on success
     */
    public boolean callIntMethod(int[] array) {
        return array != null;
    }

    /**
     * Verifies a numpy.ndarray of int64 can automatically convert to a method
     * arg of long[]
     * @param array
     * @return true on success
     */
    public boolean callLongMethod(long[] array) {
        return array != null;
    }

    /**
     * Verifies a numpy.ndarray of float32 can automatically convert to a method
     * arg of float[]
     * @param array
     * @return true on success
     */
    public boolean callFloatMethod(float[] array) {
        return array != null;
    }

    /**
     * Verifies a numpy.ndarray of float64 can automatically convert to a method
     * arg of double[]
     * @param array
     * @return true on success
     */
    public boolean callDoubleMethod(double[] array) {
        return array != null;
    }

    /**
     * Verifies that an NDArray will not allow bad/dangerous constructor args.
     */
    public void testNDArraySafety() {
        float[][] f = new float[15][];
        int[] dims = new int[] { 15, 20 };
        try {
            NDArray<float[][]> ndarray = new NDArray<float[][]>(f, dims);
            ndarray.getDimensions();
            throw new RuntimeException(
                    "NDArray should have failed instantiation");
        } catch (IllegalArgumentException e) {
            if (PRINT) {
                System.out.println("NDArray blocked bad type args");
            }
        }

        float[] d = new float[200];
        try {
            NDArray<float[]> ndarray = new NDArray<float[]>(d, dims);
            ndarray.getDimensions();
            throw new RuntimeException(
                    "NDArray should have failed instantiation");
        } catch (IllegalArgumentException e) {
            if (PRINT) {
                System.out.println("NDArray blocked bad dimensions args");
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
        File pwd = new File("tests");
        TestNumpy test = null;
        Jep jep = null;
        try {
            test = new TestNumpy();
            jep = new Jep(false, pwd.getPath());
            test.runFromJava(jep);
            test.testNDArraySafety();
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (jep != null) {
                jep.close();
            }
        }
    }

}
