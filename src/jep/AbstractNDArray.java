/**
 * Copyright (c) 2017 JEP AUTHORS.
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

/**
 * <p>
 * Represents a <a href=
 * "http://docs.scipy.org/doc/numpy/reference/generated/numpy.ndarray.html"
 * >numpy.ndarray</a> in Java. If Jep was compiled with numpy support, this
 * object will <b>not</b> be wrapped as a PyJObject in the Python
 * sub-interpreter(s), it will instead be transformed into a numpy.ndarray
 * automatically (and vice versa).
 *
 * This class provides a base implementation that can be subclassed for specifc
 * representations of the data in java (like primitive arrays or Buffers).
 *
 * @since 3.7
 */
abstract class AbstractNDArray<T> {

    protected final T data;

    protected final int[] dimensions;

    protected final boolean unsigned;

    /**
     * Constructor for a Java NDArray. Presumes the data is one dimensional.
     * 
     * @param data
     *            a data object
     */
    protected AbstractNDArray(T data) {
        this(data, false, null);
    }

    /**
     * Constructor for a Java NDArray. Presumes the data is one dimensional.
     * 
     * @param data
     *            a data object
     * @param unsigned
     *            whether the data is to be interpreted as unsigned
     */
    protected AbstractNDArray(T data, boolean unsigned) {
        this(data, unsigned, null);
    }

    /**
     * Constructor for a Java NDArray.
     * 
     * @param data
     *            a data object
     * @param dimensions
     *            the conceptual dimensions of the data (corresponds to the
     *            numpy.ndarray dimensions in C-contiguous order)
     */
    protected AbstractNDArray(T data, int... dimensions) {
        this(data, false, dimensions);
    }

    /**
     * Constructor for a Java NDArray.
     * 
     * @param data
     *            a data object
     * @param unsigned
     *            whether the data is to be interpreted as unsigned
     * @param dimensions
     *            the conceptual dimensions of the data (corresponds to the
     *            numpy.ndarray dimensions in C-contiguous order)
     */
    protected AbstractNDArray(T data, boolean unsigned, int... dimensions) {
        validate(data);
        int dataLength = getLength(data);
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

    /**
     * Validate that this data is usable as an NDArray. This will be called from
     * the constuctor and should throw an IllegalArgumentException if the data
     * object is not valid.
     * 
     * @param data
     *            a data object
     */
    protected abstract void validate(T data);

    /**
     * Get the total length of the data object.
     *
     * @param data
     *            a data object
     */
    protected abstract int getLength(T data);

    public int[] getDimensions() {
        return dimensions;
    }

    public boolean isUnsigned() {
        return unsigned;
    }

    public T getData() {
        return data;
    }

}
