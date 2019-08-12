package jep.test.numpy;

import java.nio.IntBuffer;

import jep.DirectNDArray;
import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;
import jep.NDArray;
import jep.SubInterpreter;
import jep.python.PyObject;

/**
 * Runs a variety of simple tests to verify numpy interactions are working
 * correctly.
 * 
 * Created: April 2015
 * 
 * @author Nate Jensen
 */
public class TestNumpy {

    /**
     * Sets NDArrays in a Jep interpreter, then gets them and verifies the
     * conversion in both directions is safe, ie produces a symmetrical object
     * despite a different reference/instance.
     * 
     * @param interp
     * @throws JepException
     */
    public void testSetAndGet(Interpreter interp) throws JepException {
        int[] dimensions = new int[] { 4 };
        PyObject asPyObj;

        // test boolean[]
        NDArray<boolean[]> zarray = new NDArray<>(
                new boolean[] { true, false, true, true }, dimensions);
        interp.set("zarray", zarray);
        String z_dtype = (String) interp.getValue("zarray.dtype");
        if (!"bool".equals(z_dtype)) {
            throw new AssertionError(
                    "boolean ndarray set failed, dtype = " + z_dtype);
        }
        NDArray<?> retZ = (NDArray<?>) interp.getValue("zarray");
        if (!zarray.equals(retZ)) {
            throw new AssertionError("boolean[] before != boolean[] after");
        }
        if (zarray.hashCode() != retZ.hashCode()) {
            throw new AssertionError(
                    "boolean[].hashCode() before != boolean[].hasCode() after");
        }
        asPyObj = interp.getValue("zarray", PyObject.class);

        // test byte[]
        NDArray<byte[]> barray = new NDArray<>(
                new byte[] { 0x10, 0x00, 0x54, 032 }, dimensions);
        interp.set("barray", barray);
        String b_dtype = (String) interp.getValue("barray.dtype");
        if (!"int8".equals(b_dtype)) {
            throw new AssertionError(
                    "byte ndarray set failed, dtype = " + b_dtype);
        }
        NDArray<?> retB = (NDArray<?>) interp.getValue("barray");
        if (!barray.equals(retB)) {
            throw new AssertionError("byte[] before != byte[] after");
        }
        if (barray.hashCode() != retB.hashCode()) {
            throw new AssertionError(
                    "byte[].hashCode() before != byte[].hasCode() after");
        }
        asPyObj = interp.getValue("barray", PyObject.class);

        // test short[]
        NDArray<short[]> sarray = new NDArray<>(new short[] { 5, 3, 1, 8 },
                dimensions);
        interp.set("sarray", sarray);
        String s_dtype = (String) interp.getValue("sarray.dtype");
        if (!"int16".equals(s_dtype)) {
            throw new AssertionError(
                    "short ndarray set failed, dtype = " + s_dtype);
        }
        NDArray<?> retS = (NDArray<?>) interp.getValue("sarray");
        if (!sarray.equals(retS)) {
            throw new AssertionError("short[] before != short[] after");
        }
        if (sarray.hashCode() != retS.hashCode()) {
            throw new AssertionError(
                    "short[].hashCode() before != short[].hasCode() after");
        }
        asPyObj = interp.getValue("sarray", PyObject.class);

        // test int[]
        NDArray<int[]> iarray = new NDArray<>(new int[] { 547, 232, -675, 101 },
                dimensions);
        interp.set("iarray", iarray);
        String i_dtype = (String) interp.getValue("iarray.dtype");
        if (!"int32".equals(i_dtype)) {
            throw new AssertionError(
                    "int ndarray set failed, dtype = " + i_dtype);
        }
        NDArray<?> retI = (NDArray<?>) interp.getValue("iarray");
        if (!iarray.equals(retI)) {
            throw new AssertionError("int[] before != int[] after");
        }
        if (iarray.hashCode() != retI.hashCode()) {
            throw new AssertionError(
                    "int[].hashCode() before != int[].hasCode() after");
        }
        asPyObj = interp.getValue("iarray", PyObject.class);

        // test long[]
        NDArray<long[]> larray = new NDArray<>(
                new long[] { 62724764L, 3424637L, 3426734242L, -3429234L },
                dimensions);
        interp.set("larray", larray);
        String l_dtype = (String) interp.getValue("larray.dtype");
        if (!"int64".equals(l_dtype)) {
            throw new AssertionError(
                    "long ndarray set failed, dtype = " + l_dtype);
        }
        NDArray<?> retL = (NDArray<?>) interp.getValue("larray");
        if (!larray.equals(retL)) {
            throw new AssertionError("long[] before != long[] after");
        }
        if (larray.hashCode() != retL.hashCode()) {
            throw new AssertionError(
                    "long[].hashCode() before != long[].hasCode() after");
        }
        asPyObj = interp.getValue("larray", PyObject.class);

        // test float[]
        NDArray<float[]> farray = new NDArray<>(
                new float[] { 4.32f, -0.0001f, 349.285f, 3201.0f }, dimensions);
        interp.set("farray", farray);
        String f_dtype = (String) interp.getValue("farray.dtype");
        if (!"float32".equals(f_dtype)) {
            throw new AssertionError(
                    "float ndarray set failed, dtype = " + f_dtype);
        }
        NDArray<?> retF = (NDArray<?>) interp.getValue("farray");
        if (!farray.equals(retF)) {
            throw new AssertionError("float[] before != float[] after");
        }
        if (farray.hashCode() != retF.hashCode()) {
            throw new AssertionError(
                    "float[].hashCode() before != float[].hasCode() after");
        }
        asPyObj = interp.getValue("farray", PyObject.class);

        // test double[]
        NDArray<double[]> darray = new NDArray<>(
                new double[] { 0.44321, 0.00015, -9.34278, 235574.53 },
                dimensions);
        interp.set("darray", darray);
        String d_dtype = (String) interp.getValue("darray.dtype");
        if (!"float64".equals(d_dtype)) {
            throw new AssertionError(
                    "double ndarray set failed, dtype = " + d_dtype);
        }
        NDArray<?> retD = (NDArray<?>) interp.getValue("darray");
        if (!darray.equals(retD)) {
            throw new AssertionError("double[] before != double[] after");
        }
        if (darray.hashCode() != retD.hashCode()) {
            throw new AssertionError(
                    "double[].hashCode() before != double[].hasCode() after");
        }
        asPyObj = interp.getValue("darray", PyObject.class);

        // System.out.println("NDArray get/set checked out OK");
    }

    /**
     * Called from Python to verify that a Java method's return type of NDArray
     * can be auto-converted to a numpy ndarray.
     * 
     * @param array
     * @return a copy of the data + 5
     */
    public NDArray<int[]> testArgAndReturn(NDArray<int[]> array) {
        int[] data = array.getData();
        int[] newData = new int[data.length];
        for (int i = 0; i < data.length; i++) {
            newData[i] = data[i] + 5;
        }

        return new NDArray<>(newData, array.getDimensions());
    }

    /**
     * Called from Python to verify that a Java method's return type of
     * DirectNDArray can be auto-converted to a numpy ndarray.
     * 
     * @param array
     * @return the argument
     *
     * @since 3.7
     */
    public DirectNDArray<IntBuffer> testDirectArgAndReturn(
            DirectNDArray<IntBuffer> array) {
        return array;
    }

    /**
     * Verifies a numpy.ndarray of bool can automatically convert to a method
     * arg of boolean[]
     * 
     * @param array
     * @return true on success
     */
    public boolean callBooleanMethod(boolean[] array) {
        return array != null && array.getClass().isArray();
    }

    /**
     * Verifies a numpy.ndarray of byte can automatically convert to a method
     * arg of byte[]
     * 
     * @param array
     * @return true on success
     */
    public boolean callByteMethod(byte[] array) {
        return array != null && array.getClass().isArray();
    }

    /**
     * Verifies a numpy.ndarray of int16 can automatically convert to a method
     * arg of short[]
     * 
     * @param array
     * @return true on success
     */
    public boolean callShortMethod(short[] array) {
        return array != null && array.getClass().isArray();
    }

    /**
     * Verifies a numpy.ndarray of int32 can automatically convert to a method
     * arg of int[]
     * 
     * @param array
     * @return true on success
     */
    public boolean callIntMethod(int[] array) {
        return array != null && array.getClass().isArray();
    }

    /**
     * Verifies a numpy.ndarray of int64 can automatically convert to a method
     * arg of long[]
     * 
     * @param array
     * @return true on success
     */
    public boolean callLongMethod(long[] array) {
        return array != null && array.getClass().isArray();
    }

    /**
     * Verifies a numpy.ndarray of float32 can automatically convert to a method
     * arg of float[]
     * 
     * @param array
     * @return true on success
     */
    public boolean callFloatMethod(float[] array) {
        return array != null && array.getClass().isArray();
    }

    /**
     * Verifies a numpy.ndarray of float64 can automatically convert to a method
     * arg of double[]
     * 
     * @param array
     * @return true on success
     */
    public boolean callDoubleMethod(double[] array) {
        return array != null && array.getClass().isArray();
    }

    /**
     * Verifies that an NDArray will not allow bad/dangerous constructor args.
     */
    public void testNDArraySafety() {
        float[][] f = new float[15][];
        int[] dims = new int[] { 15, 20 };
        try {
            NDArray<float[][]> ndarray = new NDArray<>(f, dims);
            ndarray.getDimensions();
            throw new AssertionError(
                    "NDArray should have failed instantiation");
        } catch (IllegalArgumentException e) {
            assert e.getLocalizedMessage() != null;
            // System.out.println("NDArray blocked bad type args");
        }

        float[] d = new float[200];
        try {
            NDArray<float[]> ndarray = new NDArray<>(d, dims);
            ndarray.getDimensions();
            throw new AssertionError(
                    "NDArray should have failed instantiation");
        } catch (IllegalArgumentException e) {
            assert e.getLocalizedMessage() != null;
            // System.out.println("NDArray blocked bad dimensions args");
        }
    }

    public static Class<?> getDefaultConversionClass(Object obj) {
        return obj.getClass();
    }

    public static void main(String[] args) {
        try (Interpreter interp = new SubInterpreter(
                new JepConfig().addIncludePaths("."))) {
            TestNumpy test = new TestNumpy();
            test.testNDArraySafety();
            test.testSetAndGet(interp);
        } catch (Throwable e) {
            e.printStackTrace();
            System.exit(1);
        }
        System.exit(0);
    }

}
