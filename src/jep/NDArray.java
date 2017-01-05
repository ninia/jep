/**
 * Copyright (c) 2016 JEP AUTHORS.
 *
 * This file is licensed under the the zlib/libpng License.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 * 
 *     1. The origin of this software must not be misrepresented; you
 *     must not claim that you wrote the original software. If you use
 *     this software in a product, an acknowledgment in the product
 *     documentation would be appreciated but is not required.
 * 
 *     2. Altered source versions must be plainly marked as such, and
 *     must not be misrepresented as being the original software.
 * 
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 */
package jep;

import java.lang.reflect.Array;
import java.util.Arrays;

/**
 * <p>
 * Represents a <a href=
 * "http://docs.scipy.org/doc/numpy/reference/generated/numpy.ndarray.html"
 * >numpy.ndarray</a> in Java. If Jep was compiled with numpy support, this
 * object will <b>not</b> be wrapped as a PyJobject in the Python
 * sub-interpreter(s), it will instead be transformed into a numpy.ndarray
 * automatically (and vice versa). The transformation in either direction occurs
 * with a <code>memcpy</code>, therefore changes in the array in one language
 * will not affect the array in the other language.
 * </p>
 * 
 * <p>
 * NDArrays only support Java primitive arrays as the underlying type of data.
 * The data can conceptually be multi-dimensional, but it must be represented as
 * a one-dimensional array in Java to ensure the memory is contiguous.
 * </p>
 * 
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 * 
 * @since 3.3
 */
public class NDArray<T extends Object> {

    protected final T data;

    protected final int[] dimensions;

    protected final boolean unsigned;

    /**
     * Constructor for a Java NDArray. Presumes the data is one dimensional.
     * 
     * @param data
     *            a one-dimensional primitive array such as float[], int[]
     */
    public NDArray(T data) {
        this(data, false, null);
    }

    /**
     * Constructor for a Java NDArray. Presumes the data is one dimensional.
     * 
     * @param data
     *            a one-dimensional primitive array such as float[], int[]
     * @param unsigned
     *            whether the data is to be interpreted as unsigned
     */
    public NDArray(T data, boolean unsigned) {
        this(data, unsigned, null);
    }

    /**
     * Constructor for a Java NDArray.
     * 
     * @param data
     *            a one-dimensional primitive array such as float[], int[]
     * @param dimensions
     *            the conceptual dimensions of the data (corresponds to the
     *            numpy.ndarray dimensions in C-contiguous order)
     */
    public NDArray(T data, int... dimensions) {
        this(data, false, dimensions);
    }

    /**
     * Constructor for a Java NDArray.
     * 
     * @param data
     *            a one-dimensional primitive array such as float[], int[]
     * @param unsigned
     *            whether the data is to be interpreted as unsigned
     * @param dimensions
     *            the conceptual dimensions of the data (corresponds to the
     *            numpy.ndarray dimensions in C-contiguous order)
     */
    public NDArray(T data, boolean unsigned, int... dimensions) {
        /*
         * java generics don't give us a nice Class that all the primitive
         * arrays extend, so we must enforce the type safety at runtime instead
         * of compile time
         */
        if (!data.getClass().isArray()
                || !data.getClass().getComponentType().isPrimitive()) {
            throw new IllegalArgumentException(
                    "NDArray only supports primitive arrays, received "
                            + data.getClass().getName());
        } else if (data instanceof char[]) {
            throw new IllegalArgumentException(
                    "NDArray only supports numeric primitives, not char[]");

        }

        int dataLength = Array.getLength(data);
        if (dimensions == null) {
            // presume one dimensional
            dimensions = new int[1];
            dimensions[0] = dataLength;
        }

        // validate data size matches dimensions size
        int dimSize = 1;
        for (int i = 0; i < dimensions.length; i++) {
            if (dimensions[i] < 0) {
                throw new IllegalArgumentException(
                        "Dimensions cannot be negative, received "
                                + dimensions[i]);
            }
            dimSize *= dimensions[i];
        }

        if (dimSize != dataLength) {
            StringBuilder sb = new StringBuilder();
            sb.append("NDArray data length ");
            sb.append(dataLength);
            sb.append(" does not match size specified by dimensions [");
            for (int i = 0; i < dimensions.length; i++) {
                sb.append(dimensions[i]);
                if (i < dimensions.length - 1) {
                    sb.append(" * ");
                }
            }
            sb.append("]");
            throw new IllegalArgumentException(sb.toString());
        }

        // passed the safety checks
        this.data = data;
        this.dimensions = dimensions;
        this.unsigned = unsigned;
    }

    public int[] getDimensions() {
        return dimensions;
    }

    public boolean isUnsigned() {
        return unsigned;
    }

    public T getData() {
        return data;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }

        NDArray<?> other = (NDArray<?>) obj;
        // unsigned should be same
        if (this.unsigned != other.unsigned) {
            return false;
        }

        /*
         * compare dimensions first cause that's most likely a shorter array to
         * compare and will be faster
         */
        if (!Arrays.equals(dimensions, other.dimensions)) {
            return false;
        }

        // compare the data
        if (other.data == null) {
            return false;
        } else {
            // neither has null, let's compare values
            if (!data.getClass().equals(other.data.getClass())) {
                return false;
            }
            Class<?> clz = data.getClass().getComponentType();
            if (clz == Boolean.TYPE) {
                return Arrays.equals((boolean[]) data, (boolean[]) other.data);
            } else if (clz == Byte.TYPE) {
                return Arrays.equals((byte[]) data, (byte[]) other.data);
            } else if (clz == Short.TYPE) {
                return Arrays.equals((short[]) data, (short[]) other.data);
            } else if (clz == Integer.TYPE) {
                return Arrays.equals((int[]) data, (int[]) other.data);
            } else if (clz == Long.TYPE) {
                return Arrays.equals((long[]) data, (long[]) other.data);
            } else if (clz == Float.TYPE) {
                return Arrays.equals((float[]) data, (float[]) other.data);
            } else if (clz == Double.TYPE) {
                return Arrays.equals((double[]) data, (double[]) other.data);
            } else {
                // should be impossible to get here
                throw new IllegalStateException(
                        "NDArray only supports primitive arrays, received "
                                + clz.getName());
            }
        }
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        if (data == null) {
            result = prime * result + 0;
        } else {
            Class<?> clz = data.getClass().getComponentType();
            if (clz == Boolean.TYPE) {
                result = prime * result + Arrays.hashCode((boolean[]) data);
            } else if (clz == Byte.TYPE) {
                result = prime * result + Arrays.hashCode((byte[]) data);
            } else if (clz == Short.TYPE) {
                result = prime * result + Arrays.hashCode((short[]) data);
            } else if (clz == Integer.TYPE) {
                result = prime * result + Arrays.hashCode((int[]) data);
            } else if (clz == Long.TYPE) {
                result = prime * result + Arrays.hashCode((long[]) data);
            } else if (clz == Float.TYPE) {
                result = prime * result + Arrays.hashCode((float[]) data);
            } else if (clz == Double.TYPE) {
                result = prime * result + Arrays.hashCode((double[]) data);
            } else {
                // should be impossible to get here
                throw new IllegalStateException(
                        "NDArray only supports primitive arrays, received "
                                + clz.getName());
            }

        }
        result = prime * result + Arrays.hashCode(dimensions)
                + (unsigned ? 1 : 0);
        return result;
    }

    // TODO override toString() to make it look like ndarray.__str__()

}
