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

import java.nio.Buffer;
import java.nio.CharBuffer;
import java.util.Arrays;

/**
 * <p>
 * Represents a <a href=
 * "http://docs.scipy.org/doc/numpy/reference/generated/numpy.ndarray.html"
 * >numpy.ndarray</a> in Java. If Jep was compiled with numpy support, this
 * object will <b>not</b> be wrapped as a PyJobject in the Python
 * sub-interpreter(s), it will instead be wrapped as a numpy.ndarray
 * automatically. The numpy.ndarray will reference the exact same memory as the
 * buffer so changes in either language will be immediately visible in both.
 *
 * <p>
 * DirectNDArrays only support direct buffers as the underlying type of data.
 * The data can conceptually be multi-dimensional, but it must be represented as
 * a one-dimensional direct buffer in Java to ensure the memory is contiguous.
 * </p>
 * 
 * 
 * @author Ben Steffensmeier
 * @since 3.7
 */
public class DirectNDArray<T extends Buffer> extends AbstractNDArray<T> {

    /**
     * Constructor for a Java DirectNDArray. Presumes the data is one
     * dimensional.
     * 
     * @param data
     *            a direct Buffer
     */
    public DirectNDArray(T data) {
        super(data);
    }

    /**
     * Constructor for a Java DirectNDArray. Presumes the data is one
     * dimensional.
     * 
     * @param data
     *            a direct Buffer
     * @param unsigned
     *            whether the data is to be interpreted as unsigned
     */
    public DirectNDArray(T data, boolean unsigned) {
        super(data, unsigned);
    }

    /**
     * Constructor for a Java DirectNDArray.
     * 
     * @param data
     *            a direct Buffer
     * @param dimensions
     *            the conceptual dimensions of the data (corresponds to the
     *            numpy.ndarray dimensions in C-contiguous order)
     */
    public DirectNDArray(T data, int... dimensions) {
        super(data, dimensions);
    }

    /**
     * Constructor for a Java DirectNDArray.
     * 
     * @param data
     *            a direct Buffer
     * @param unsigned
     *            whether the data is to be interpreted as unsigned
     * @param dimensions
     *            the conceptual dimensions of the data (corresponds to the
     *            numpy.ndarray dimensions in C-contiguous order)
     */
    public DirectNDArray(T data, boolean unsigned, int... dimensions) {
        super(data, unsigned, dimensions);
    }

    @Override
    protected void validate(T data) {
        if (!data.isDirect()) {
            throw new IllegalArgumentException(
                    "DirectNDArray only supports direct buffers.");
        } else if (data instanceof CharBuffer) {
            throw new IllegalArgumentException(
                    "DirectNDArray only supports numeric primitives, not CharBuffer");
        }
    }

    @Override
    public int getLength(T data) {
        return data.capacity();
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

        DirectNDArray<?> other = (DirectNDArray<?>) obj;
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
        }

        // neither has null, let's compare values
        return data.equals(other.data);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        if (data == null) {
            result = prime * result + 0;
        } else {
            result = prime * result + data.hashCode();
        }
        result = prime * result + Arrays.hashCode(dimensions)
                + (unsigned ? 1 : 0);
        return result;
    }

    // TODO override toString() to make it look like ndarray.__str__()

}
