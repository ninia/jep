package jep.test;

import java.lang.reflect.UndeclaredThrowableException;
import java.util.Arrays;
import java.util.Deque;

import jep.Interpreter;
import jep.JepException;
import jep.SubInterpreter;
import jep.python.PyCallable;
import jep.python.PyObject;

/**
 * @author bsteffen
 * @since 3.8
 */
public class TestGetJPyObject {

    public static void testIdentity(Interpreter interp) throws JepException {
        interp.eval(
                "t = [object(), 1, 1.5, True, None, [], (), {'key':'value'} ]");
        PyObject[] diverseTypes = interp.getValue("t", PyObject[].class);
        for (int i = 0; i < diverseTypes.length; i += 1) {
            interp.set("t2", diverseTypes[i]);
            Boolean b = interp.getValue("t[" + i + "] is not t2",
                    Boolean.class);
            if (b.booleanValue()) {
                throw new IllegalStateException(
                        "JPyObject " + i + " is not preserving identity.");
            }
        }
    }

    private static String buildTestClassPython() {
        StringBuilder testclass = new StringBuilder();
        testclass.append("class testclass(object):\n");
        testclass.append("  def __init__(self, attr1, attr2, attr3):\n");
        testclass.append("    self.attr1 = attr1\n");
        testclass.append("    self.attr2 = attr2\n");
        testclass.append("    self.attr3 = attr3\n");
        testclass.append("    self.selfattr = self\n");
        return testclass.toString();
    }

    public static void testGetAttr(Interpreter interp) throws JepException {
        interp.eval(buildTestClassPython());
        interp.eval("t = testclass(1, 'A String', None)");
        PyObject t = interp.getValue("t", PyObject.class);
        Number attr1 = t.getAttr("attr1", Number.class);
        if (attr1.intValue() != 1) {
            throw new IllegalStateException("JPyObject attr1 lookup failed.");
        }
        Object attr2 = t.getAttr("attr2");
        if (!attr2.equals("A String")) {
            throw new IllegalStateException("JPyObject attr2 lookup failed.");
        }
        Object attr3 = t.getAttr("attr3");
        if (attr3 != null) {
            throw new IllegalStateException("JPyObject attr3 lookup failed.");
        }
        PyObject selfattr = t.getAttr("selfattr", PyObject.class);
        interp.set("t2", selfattr);
        Boolean b = interp.getValue("t is not t2", Boolean.class);
        if (b.booleanValue()) {
            throw new IllegalStateException(
                    "JPyObject selfattr lookup failed.");
        }

        try {
            t.getAttr(null, PyObject.class);
            throw new IllegalStateException("JPyObject null attr_name failed");
        } catch (JepException e) {
            /*
             * We want an exception as Jep should not have allowed null. Python
             * will crash when we use the CPython API if the attr_name is null.
             */
        }
    }

    public static void testSetAttr(Interpreter interp) throws JepException {
        interp.eval(buildTestClassPython());
        interp.eval("t = testclass(1, 'A String', None)");
        PyObject t = interp.getValue("t", PyObject.class);

        t.setAttr("attr1", 5.5);
        if (interp.getValue("t.attr1", Double.class) != 5.5) {
            throw new IllegalStateException("JPyObject attr1 setting failed.");
        }

        t.setAttr("attr2", "B String");
        if (!interp.getValue("t.attr2", String.class).equals("B String")) {
            throw new IllegalStateException("JPyObject attr2 setting failed.");
        }

        t.setAttr("attr3", null);
        if (!interp.getValue("t.attr3 is None", Boolean.class)) {
            throw new IllegalStateException("JPyObject attr3 setting failed.");
        }

        t.setAttr("attr4", "C String");
        if (!interp.getValue("t.attr4", String.class).equals("C String")) {
            throw new IllegalStateException("JPyObject attr4 setting failed.");
        }

        try {
            t.setAttr(null, "ABC");
            throw new IllegalStateException("JPyObject null attr_name failed");
        } catch (JepException e) {
            /*
             * We want an exception as Jep should not have allowed null. Python
             * will crash when we use the CPython API if the attr_name is null.
             */
        }
    }

    public static void testDelAttr(Interpreter interp) throws JepException {
        interp.eval(buildTestClassPython());
        interp.eval("t = testclass(1, 'A String', None)");
        PyObject t = interp.getValue("t", PyObject.class);

        t.delAttr("attr1");
        if (interp.getValue("'attr1' in t.__dict__", Boolean.class)) {
            throw new IllegalStateException("JPyObject attr1 deleting failed.");
        }
        try {
            t.delAttr(null);
            throw new IllegalStateException("JPyObject null attr_name failed");
        } catch (JepException e) {
            /*
             * We want an exception as Jep should not have allowed null. Python
             * will crash when we use the CPython API if the attr_name is null.
             */
        }
    }

    public static void testJPyCallable(Interpreter interp) throws JepException {
        PyCallable chr = interp.getValue("chr", PyCallable.class);
        Object result = chr.call(32);
        if (!" ".equals(result)) {
            throw new IllegalStateException(
                    "JPyCallable chr does not work as expected.");
        }

        String typedResultStr = chr.callAs(String.class, 32);
        if (!" ".equals(typedResultStr)) {
            throw new IllegalStateException(
                    "JPyCallable chr does not work as expected.");
        }

        PyCallable count = interp.getValue("[1,2,1,4,1,4].count",
                PyCallable.class);
        result = count.call(1);
        if (((Number) result).intValue() != 3) {
            throw new IllegalStateException(
                    "JPyCallable list.count does not work as expected.");
        }

        Long typedResultLong = count.callAs(Long.class, 4);
        if (typedResultLong.intValue() != 2) {
            throw new IllegalStateException(
                    "JPyCallable list.count does not work as expected.");
        }

        // test that the requested return type is actually respected - if not,
        // this would return String instead of PyObject
        PyCallable str = interp.getValue("str", PyCallable.class);
        PyObject typedResultObj = str.callAs(PyObject.class, 12342);
        count = typedResultObj.getAttr("count", PyCallable.class);
        typedResultLong = count.callAs(Long.class, "2");

        if (typedResultLong.intValue() != 2) {
            throw new IllegalStateException(
                    "JPyCallable str.count does not work as expected.");
        }

        typedResultLong = count.callAs(Long.class, "1");

        if (typedResultLong.intValue() != 1) {
            throw new IllegalStateException(
                    "JPyCallable str.count does not work as expected.");
        }

    }

    public static void testToString(Interpreter interp) throws JepException {
        PyObject pylist = interp.getValue("[1, 2, 3, 4, 5]", PyObject.class);
        if (!"[1, 2, 3, 4, 5]".equals(pylist.toString())) {
            throw new IllegalStateException(
                    "JPyObject toString() does not work as expected");
        }
    }

    public static void testEquals(Interpreter interp) throws JepException {
        interp.eval("s = 'my python string'");
        PyObject pystr = interp.getValue("s", PyObject.class);
        if (pystr.equals("my python string")) {
            throw new IllegalStateException(
                    "JPyObject equals() does not work as expected");
        }

        if (interp.getValue("1", PyObject.class).equals(1)) {
            throw new IllegalStateException(
                    "JPyObject equals() does not work as expected");
        }

        if ((Integer.valueOf(1)).equals(interp.getValue("1", PyObject.class))) {
            throw new IllegalStateException(
                    "Java equals(JPyObject) does not work as expected");
        }
    }

    public static void testHashCode(Interpreter interp) throws JepException {
        PyObject pyObj1 = interp.getValue("'xyz'", PyObject.class);
        PyObject pyObj2 = interp.getValue("'xyz'", PyObject.class);
        Integer hash1 = pyObj1.hashCode();
        Integer hash2 = pyObj2.hashCode();
        if (!hash1.equals(hash2)) {
            throw new IllegalStateException(
                    "JPyObject hashCode() does not work as expected");
        }

        PyObject pyObj3 = interp.getValue("45", PyObject.class);
        PyObject pyObj4 = interp.getValue("44 + 1", PyObject.class);
        Integer hash3 = pyObj3.hashCode();
        Integer hash4 = pyObj4.hashCode();
        if (!hash3.equals(hash4)) {
            throw new IllegalStateException(
                    "JPyObject hashCode() does not work as expected");
        }
    }

    public static void testThreading(Interpreter interp)
            throws JepException, InterruptedException {
        final PyObject o = interp.getValue("object()", PyObject.class);
        final boolean[] exception = { false };
        Thread t = new Thread() {
            @Override
            public void run() {
                try {
                    o.getAttr("__doc__");
                } catch (JepException e) {
                    exception[0] = true;
                }
            }
        };
        t.start();
        t.join();
        if (!exception[0]) {
            throw new IllegalStateException(
                    "JPyObject allowed access on wrong thread");

        }
    }

    public static void testClosing(Interpreter interp) throws JepException {
        PyObject leaky = null;
        try (PyObject o = interp.getValue("object()", PyObject.class)) {
            leaky = o;
            /*
             * The AutoCloseable interface strongly recommends making it
             * harmless to call close more than once.
             */
            o.close();
        }
        try {
            leaky.getAttr("__doc__");
            throw new IllegalStateException(
                    "JPyObject allowed access on wrong thread");
        } catch (JepException e) {
            /*
             * We want an exception as Jep should not have allowed access.
             */
        }
    }

    public static void testClosingJep() throws JepException {
        PyObject leaky = null;
        try (Interpreter interp = new SubInterpreter()) {
            leaky = interp.getValue("object()", PyObject.class);
        }
        try {
            leaky.getAttr("__doc__");
            throw new IllegalStateException(
                    "JPyObject allowed access on wrong thread");
        } catch (JepException e) {
            /*
             * We want an exception as Jep should not have allowed access.
             */
        }
    }

    private static interface PyInt {
        public Byte bit_length();
    }

    private static interface PyInt2 {
        public byte bit_length();
    }

    public static void testProxy(Interpreter interp) throws JepException {
        interp.eval("l = [7]");
        PyObject list = interp.getValue("l", PyObject.class);
        @SuppressWarnings("unchecked")
        Deque<Number> q = list.proxy(Deque.class);
        Number n = q.pop();
        if (n.intValue() != 7) {
            throw new IllegalStateException("list.pop returned wrong value");
        }
        /* Make sure it is empty now */
        Boolean b = interp.getValue("len(l) == 0", Boolean.class);
        if (!b.booleanValue()) {
            throw new IllegalStateException("list.pop proxy failed");
        }
        try {
            q.push(n);
            throw new IllegalStateException("list.push worked");
        } catch (UndeclaredThrowableException e) {
            /* list doesn't have a push so this is correct */
        }

        PyInt i = interp.getValue("1", PyObject.class).proxy(PyInt.class);
        Byte bit_length = i.bit_length();
        if (bit_length.intValue() != 1) {
            throw new IllegalStateException(
                    "bit_length boxed is wrong: " + bit_length);
        }
        PyInt2 i2 = interp.getValue("1", PyObject.class).proxy(PyInt2.class);
        bit_length = i2.bit_length();
        if (bit_length.intValue() != 1) {
            throw new IllegalStateException(
                    "bit_length primitive is wrong: " + bit_length);
        }

        interp.set("i", i);
        bit_length = interp.getValue("int.bit_length(i)", Byte.class);
        if (bit_length.intValue() != 1) {
            throw new IllegalStateException(
                    "bit_length passback is wrong: " + bit_length);
        }
    }

    public static void testAs(Interpreter interp) throws JepException {
        int[] test = { 1, 2, 3 };
        interp.set("test", test);
        PyObject list = interp.getValue("list(test)", PyObject.class);
        if (!Arrays.equals(test, list.as(int[].class))) {
            throw new IllegalStateException("PyObject.as did not work.");
        }
    }

    public static void main(String[] args) throws Exception {
        try (Interpreter interp = new SubInterpreter()) {
            testIdentity(interp);
            testGetAttr(interp);
            testSetAttr(interp);
            testDelAttr(interp);
            testJPyCallable(interp);
            testToString(interp);
            testEquals(interp);
            testHashCode(interp);
            testThreading(interp);
            testClosing(interp);
            testProxy(interp);
            testAs(interp);
        }
        testClosingJep();
    }

}
