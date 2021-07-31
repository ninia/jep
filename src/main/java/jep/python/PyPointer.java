/**
 * Copyright (c) 2017-2021 JEP AUTHORS.
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
package jep.python;

import java.lang.ref.WeakReference;

import jep.JepException;

/**
 * Contains pointers to a JepThread* and a PyObject* for using a PyObject* in
 * Java as a PyObject or its subclasses.
 * 
 *
 * @author Nate Jensen
 * @since 3.8
 */
public class PyPointer extends WeakReference<PyObject> {

    protected final long tstate;

    protected final long pyObject;

    protected final MemoryManager memoryManager;

    protected volatile boolean disposed;

    /**
     * Constructor
     * 
     * @param referrent
     *            the PyObject (or subclass) corresponding to this PyPointer
     * @param memoryManager
     *            the MemoryManager responsible for tracking this pointer
     * @param tstate
     *            the pointer to the JepThreadState
     * @param pyObject
     *            the pointer to the PyObject*
     * @throws JepException
     *             if an error occurs
     */
    protected PyPointer(PyObject referrent, MemoryManager memoryManager,
            long tstate, long pyObject) throws JepException {
        super(referrent, memoryManager.getReferenceQueue());
        this.tstate = tstate;
        this.pyObject = pyObject;
        this.memoryManager = memoryManager;
        this.memoryManager.addReference(this);
        disposed = false;
    }

    /**
     * Decrefs the PyObject* associated with this PyPointer. When a Python
     * object reaches zero references, it is available for Python garbage
     * collection.
     * 
     * @throws JepException
     *             if an error occurs
     */
    protected synchronized void dispose() throws JepException {
        if (!disposed) {
            disposed = true;
            memoryManager.removeReference(this);
            decref(tstate, pyObject);
        }
    }

    protected boolean isDisposed() {
        return disposed;
    }

    private native void decref(long tstate, long pyObject) throws JepException;

}
