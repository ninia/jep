package jep.test;

import jep.Jep;
import jep.JepException;
import jep.python.PyCallable;
import jep.python.PyObject;

/**
 * @author bsteffen
 * @since 3.8
 */
public class TestGetJPyObject {

    public static void testIdentity(Jep jep) throws JepException {
        jep.eval(
                "t = [object(), 1, 1.5, True, None, [], (), {'key':'value'} ]");
        PyObject[] diverseTypes = jep.getValue("t", PyObject[].class);
        for (int i = 0; i < diverseTypes.length; i += 1) {
            jep.set("t2", diverseTypes[i]);
            Boolean b = jep.getValue("t[" + i + "] is not t2", Boolean.class);
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

    public static void testGetAttr(Jep jep) throws JepException {
        jep.eval(buildTestClassPython());
        jep.eval("t = testclass(1, 'A String', None)");
        PyObject t = jep.getValue("t", PyObject.class);
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
        jep.set("t2", selfattr);
        Boolean b = jep.getValue("t is not t2", Boolean.class);
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

    public static void testSetAttr(Jep jep) throws JepException {
        jep.eval(buildTestClassPython());
        jep.eval("t = testclass(1, 'A String', None)");
        PyObject t = jep.getValue("t", PyObject.class);

        t.setAttr("attr1", 5.5);
        if (jep.getValue("t.attr1", Double.class) != 5.5) {
            throw new IllegalStateException("JPyObject attr1 setting failed.");
        }

        t.setAttr("attr2", "B String");
        if (!jep.getValue("t.attr2", String.class).equals("B String")) {
            throw new IllegalStateException("JPyObject attr2 setting failed.");
        }

        t.setAttr("attr3", null);
        if (!jep.getValue("t.attr3 is None", Boolean.class)) {
            throw new IllegalStateException("JPyObject attr3 setting failed.");
        }

        t.setAttr("attr4", "C String");
        if (!jep.getValue("t.attr4", String.class).equals("C String")) {
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

    public static void testDelAttr(Jep jep) throws JepException {
        jep.eval(buildTestClassPython());
        jep.eval("t = testclass(1, 'A String', None)");
        PyObject t = jep.getValue("t", PyObject.class);

        t.delAttr("attr1");
        if (jep.getValue("'attr1' in t.__dict__", Boolean.class)) {
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

    public static void testJPyCallable(Jep jep) throws JepException {
        PyCallable chr = jep.getValue("chr", PyCallable.class);
        Object result = chr.call(32);
        if (!" ".equals(result)) {
            throw new IllegalStateException(
                    "JPyCallable chr does not work as expected.");
        }
        PyCallable count = jep.getValue("[1,2,1,4,1,4].count",
                PyCallable.class);
        result = count.call(1);
        if (((Number) result).intValue() != 3) {
            throw new IllegalStateException(
                    "JPyCallable list.count does not work as expected.");
        }
    }

    public static void testToString(Jep jep) throws JepException {
        PyObject pylist = jep.getValue("[1, 2, 3, 4, 5]", PyObject.class);
        if (!"[1, 2, 3, 4, 5]".equals(pylist.toString())) {
            throw new IllegalStateException(
                    "JPyObject toString() does not work as expected");
        }
    }

    public static void testEquals(Jep jep) throws JepException {
        jep.eval("s = 'my python string'");
        PyObject pystr = jep.getValue("s", PyObject.class);
        if (pystr.equals("my python string")) {
            throw new IllegalStateException(
                    "JPyObject equals() does not work as expected");
        }

        if (jep.getValue("1", PyObject.class).equals(1)) {
            throw new IllegalStateException(
                    "JPyObject equals() does not work as expected");
        }

        if ((Integer.valueOf(1)).equals(jep.getValue("1", PyObject.class))) {
            throw new IllegalStateException(
                    "Java equals(JPyObject) does not work as expected");
        }
    }

    public static void testHashCode(Jep jep) throws JepException {
        PyObject pyObj1 = jep.getValue("'xyz'", PyObject.class);
        PyObject pyObj2 = jep.getValue("'xyz'", PyObject.class);
        Integer hash1 = pyObj1.hashCode();
        Integer hash2 = pyObj2.hashCode();
        if (!hash1.equals(hash2)) {
            throw new IllegalStateException(
                    "JPyObject hashCode() does not work as expected");
        }

        PyObject pyObj3 = jep.getValue("45", PyObject.class);
        PyObject pyObj4 = jep.getValue("44 + 1", PyObject.class);
        Integer hash3 = pyObj3.hashCode();
        Integer hash4 = pyObj4.hashCode();
        if (!hash3.equals(hash4)) {
            throw new IllegalStateException(
                    "JPyObject hashCode() does not work as expected");
        }
    }

    public static void testThreading(Jep jep) throws JepException, InterruptedException {
        final PyObject o = jep.getValue("object()", PyObject.class);
        final boolean[] exception = { false };
        Thread t = new Thread() {
            @Override
            public void run() {
                try {
                    o.getAttr("__doc__");
                } catch(JepException e){
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

    public static void testClosing(Jep jep) throws JepException {
        PyObject leaky = null;
        try (PyObject o = jep.getValue("object()", PyObject.class)) {
            leaky = o;
            /* 
             * The AutoCloseable interface strongly recommends making it
             *  harmless to call close more than once.
             */
            o.close();
        }
        try {
            leaky.getAttr("__doc__");
            throw new IllegalStateException(
                    "JPyObject allowed access on wrong thread");
        } catch(JepException e){
            /*
             * We want an exception as Jep should not have allowed access.
             */
        }
    }

    public static void testClosingJep() throws JepException {
        PyObject leaky = null;
        try (Jep jep = new Jep()) {
            leaky = jep.getValue("object()", PyObject.class);
        }
        try {
            leaky.getAttr("__doc__");
            throw new IllegalStateException(
                    "JPyObject allowed access on wrong thread");
        } catch(JepException e){
            /*
             * We want an exception as Jep should not have allowed access.
             */
        }
    }

    public static void main(String[] args) throws Exception {
        try (Jep jep = new Jep()) {
            testIdentity(jep);
            testGetAttr(jep);
            testSetAttr(jep);
            testDelAttr(jep);
            testJPyCallable(jep);
            testToString(jep);
            testEquals(jep);
            testHashCode(jep);
            testThreading(jep);
            testClosing(jep);
        }
        testClosingJep();
    }

}
