package jep.test;

import jep.Jep;
import jep.JepException;
import jep.python.PyObject;

/**
 * @author bsteffen
 * @since 3.8
 */
public class TestGetJPyObject {

    public static void main(String[] args) throws JepException {
        try (Jep jep = new Jep()) {
            jep.eval("t = object()");
            PyObject t = jep.getValue("t", PyObject.class);
            jep.set("t2", t);
            Boolean b = jep.getValue("t is not t2", Boolean.class);
            if(b.booleanValue()){
                throw new IllegalArgumentException("JPyObject is not preserving identity.");
            }
        }
    }

}
