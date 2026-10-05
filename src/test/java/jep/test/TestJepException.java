package jep.test;

import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;
import jep.PythonException;
import jep.SubInterpreter;
import jep.python.PyObject;

/**
 * Tests the behavior of {@link JepException} and {@link PythonException}
 * across the three exception origins: pure Python, Java surfaced through
 * Python, and directly constructed in Java.
 *
 * Created: May 2026
 *
 * @author Michael Quindt
 */
public class TestJepException {

    public static void main(String[] args) throws JepException {
        JepConfig config = new JepConfig().addIncludePaths(".");
        try (Interpreter interp = new SubInterpreter(config)) {
            // A pure Python exception must be a PythonException with a non-null PyObject proxy.
            try {
                interp.eval("raise ValueError('test traceback')");
                throw new RuntimeException("Expected JepException was not thrown");
            } catch (JepException e) {
                testPythonException(e, null);
                PythonException pe = (PythonException) e;
                // Verify the attached exception object actually has a traceback attribute.
                PyObject pyObj = pe.getPythonException()
                        .getAttr("__traceback__", PyObject.class);
                String tbType = pyObj.getAttr("__class__", PyObject.class)
                        .getAttr("__name__", String.class);
                if (!tbType.equals("traceback")) {
                    throw new RuntimeException(
                            "Expected traceback type, got: " + tbType);
                }
            }

            // A Java exception surfaced through Python must also be a PythonException,
            // preserving the original Java exception as the cause.
            interp.eval("from java.util import ArrayList");
            try {
                interp.eval("ArrayList().get(0)");
                throw new RuntimeException("Expected JepException was not thrown");
            } catch (JepException e) {
                testPythonException(e, IndexOutOfBoundsException.class);
            }
            try {
                interp.eval(
                        "try:\n  ArrayList().get(0)\nexcept AttributeError:\n  pass");
                throw new RuntimeException("Expected JepException was not thrown");
            } catch (JepException e) {
                testPythonException(e, IndexOutOfBoundsException.class);
            }
        }
    }

    private static void testPythonException(JepException e,
            Class<? extends Throwable> expectedCause) {
        if (!(e instanceof PythonException)) {
            throw new RuntimeException(
                    "Expected PythonException but got " + e.getClass().getName());
        }
        PythonException pe = (PythonException) e;
        if (pe.getPythonException() == null) {
            throw new RuntimeException(
                    "getPythonException() returned null");
        }
        if (pe.getPythonTraceback() == null) {
            throw new RuntimeException(
                    "getPythonTraceback() returned null");
        }
        if (expectedCause != null && !(expectedCause.isInstance(e.getCause()))) {
            throw new RuntimeException(
                    "Expected cause " + expectedCause.getName() +
                    " but got " + (e.getCause() == null ? "null" : e.getCause().getClass().getName()));
        }
    }
}
