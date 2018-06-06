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

    public static void testGetAttr(Jep jep) throws JepException {
        StringBuilder testclass = new StringBuilder();
        testclass.append("class testclass(object):\n");
        testclass.append("  def __init__(self, attr1, attr2, attr3):\n");
        testclass.append("    self.attr1 = attr1\n");
        testclass.append("    self.attr2 = attr2\n");
        testclass.append("    self.attr3 = attr3\n");
        testclass.append("    self.selfattr = self\n");
        jep.eval(testclass.toString());
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
        // Python object must be on the left
        if (!pystr.equals("my python string")) {
            throw new IllegalStateException(
                    "JPyObject equals() does not work as expected");
        }

        /*
         * since Python object is on the left, will use PyObject.equals(obj)
         * which ignores class types
         */
        if (!jep.getValue("1", PyObject.class).equals(1)) {
            throw new IllegalStateException(
                    "JPyObject equals() does not work as expected");
        }

        /*
         * since Java object is on the left, will use Integer.equals(obj) and
         * not be equals since Java compares class types
         */
        if ((new Integer(1)).equals(jep.getValue("1", PyObject.class))) {
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

    public static void main(String[] args) throws JepException {
        try (Jep jep = new Jep()) {
            testIdentity(jep);
            testGetAttr(jep);
            testJPyCallable(jep);
            testToString(jep);
            testEquals(jep);
            testHashCode(jep);
        }
    }

}
