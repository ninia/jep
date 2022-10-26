/**
 * Copyright (c) 2017-2022 JEP AUTHORS.
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

import java.lang.ref.ReferenceQueue;
import java.util.Collections;
import java.util.IdentityHashMap;
import java.util.Iterator;
import java.util.Set;
import java.util.WeakHashMap;

import jep.Jep;
import jep.JepAccess;
import jep.JepException;

/**
 * Manages the native memory associated with PyObjects in an Interpreter.
 *
 * @see <a href=
 *      "http://www.oracle.com/technetwork/articles/java/finalization-137655.html">How
 *      to Handle Java Finalization's Memory-Retention Issues</a>
 * @author Nate Jensen
 * @since 3.8
 */
public final class MemoryManager extends JepAccess{

    private final ThreadLocal<Jep> interpreters = new ThreadLocal<Jep>();

    private final Set<Jep> interpreterSet = Collections
            .newSetFromMap(new WeakHashMap<Jep, Boolean>());
    
    private final ReferenceQueue<PyObject> refQueue = new ReferenceQueue<>();

    private final Set<PyPointer> pointers = Collections
            .newSetFromMap(new IdentityHashMap<PyPointer, Boolean>());

    public void openInterpreter(Jep jep) {
        this.interpreters.set(jep);
        this.interpreterSet.add(jep);
    }

    protected ReferenceQueue<PyObject> getReferenceQueue() throws JepException {
        cleanupWeakReferences();
        return refQueue;
    }

    protected void addReference(PyPointer pointer) {
        pointers.add(pointer);
    }

    protected void removeReference(PyPointer pyPtr) {
        pointers.remove(pyPtr);
    }

    /**
     * Stop managing memory for an interpreter. Any PyPointers managed by this
     * object will no longer be usable from the interpreter thread. If this is
     * the last interpreter used by this manager then all PyPointers managed by
     * this will be disposed.
     * 
     * @throws JepException
     *             if an error occurs
     */
    public void closeInterpreter(Jep jep) throws JepException {
        if (interpreterSet.size() == 1) {
            Iterator<PyPointer> itr = pointers.iterator();
            while (itr.hasNext()) {
                PyPointer ptr = itr.next();
                /*
                 * ptr.dispose() will remove from the set, so we remove it here
                 * first to avoid ConcurrentModificationException
                 */
                itr.remove();
                ptr.dispose();
            }
        }
        interpreters.remove();
        interpreterSet.remove(jep);
    }

    /**
     * Cleans out weak references to PyPointers associated with this
     * Interpreter. Attempts to free memory earlier than a Jep.close() if the
     * developer did not explicitly free the memory with PyObject.close().
     * 
     * @throws JepException
     *             if an error occurs
     */
    public void cleanupWeakReferences() throws JepException {
        PyPointer p = (PyPointer) refQueue.poll();
        while (p != null) {
            p.dispose();
            p = (PyPointer) refQueue.poll();
        }
    }

    private Jep getThreadLocalJep() throws JepException{
        Jep jep = interpreters.get();
        if (jep == null) {
            throw new JepException("Invalid thread access.");
        }
        return jep;
    }

    protected long getThreadState() throws JepException{
        return getThreadState(getThreadLocalJep());
    }

    protected ClassLoader getClassLoader() {
        return getClassLoader(getThreadLocalJep());
    }
}
