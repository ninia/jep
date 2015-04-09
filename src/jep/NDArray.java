package jep;

import java.util.Arrays;

/**
 * <pre>
 * NDArray.java - Represents a numpy.ndarray in Java.  Should
 * seamlessly transition between this representation in Java and
 * a numpy.ndarray representation in Python.
 * 
 * Copyright (c) 2015 Nate Jensen.
 * 
 * This file is licenced under the the zlib/libpng License.
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
 * 
 * Created: Tues Apr 07 2015
 * 
 * </pre>
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 * @version $Id$
 */
public class NDArray<T extends Object> {

    protected final T data;

    protected final int[] dimensions;

    public NDArray(T data, int[] dimensions) {
        this.data = data;
        this.dimensions = dimensions;

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
        }
    }

    public int[] getDimensions() {
        return dimensions;
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
        /*
         * compare dimensions first cause that's most likely a shorter array to
         * compare and will be faster
         */
        if (!Arrays.equals(dimensions, other.dimensions)) {
            return false;
        }

        // compare the data
        if (data == null && other.data == null) {
            return true;
        } else if (data == null && other.data != null) {
            return false;
        } else if (data != null && other.data == null) {
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
        result = prime * result + Arrays.hashCode(dimensions);
        return result;
    }

}
