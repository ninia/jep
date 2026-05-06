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
            // A pure Python exception must be a PythonException with a
            // formatted traceback and a non-null PyObject proxy.
            try {
                interp.eval("raise ValueError('test traceback')");
                throw new RuntimeException("Expected JepException was not thrown");
            } catch (JepException e) {
                // Ensure the traceback string is a formatted Python traceback
                testTracebackString(e, "ValueError");

                // Ensure the exception is a PythonException and that the
                // attached exception object is a Python exception object
                // with a traceback attribute
                if (!(e instanceof PythonException pe)) {
                    throw new RuntimeException(
                            "Expected PythonException for a pure Python exception, but got " +
                                    e.getClass().getName());
                }
                PyObject pyExc = pe.getPythonException();
                if (pyExc == null) {
                    throw new RuntimeException(
                            "getPythonException() returned null for a Python exception");
                }
                Object py = pyExc.getAttr("__traceback__");
                if (!(py instanceof PyObject pyObj)) {
                    throw new RuntimeException(
                            "Could not fetch Python traceback from exception object");
                }
                String tbType = pyObj.getAttr("__class__", PyObject.class)
                        .getAttr("__name__", String.class);
                if (!tbType.equals("traceback")) {
                    throw new RuntimeException(
                            "Expected traceback type, got: " + tbType);
                }
            }

            // A Java exception surfaced through Python must carry a traceback
            // and preserve the Java cause, but must not be a PythonException.
            interp.eval("from java.util import ArrayList");
            try {
                interp.eval("ArrayList().get(0)");
                throw new RuntimeException("Expected JepException was not thrown");
            } catch (JepException e) {
                testJavaTraceback(e);
            }
            try {
                interp.eval(
                        "try:\n  ArrayList().get(0)\nexcept AttributeError:\n  pass");
                throw new RuntimeException("Expected JepException was not thrown");
            } catch (JepException e) {
                testJavaTraceback(e);
            }
        }

        // A JepException created directly in Java must not carry a traceback.
        JepException direct = new JepException("created in Java");
        if (direct.getPythonTraceback() != null) {
            throw new RuntimeException(
                    "getPythonTraceback() should be null for a directly constructed JepException");
        }
    }

    private static void testJavaTraceback(JepException e) {
        // Ensure exception is not a PythonException
        if (e instanceof PythonException) {
            throw new RuntimeException(
                    "Java-origin exception should not be a PythonException");
        }

        // Ensure the cause matches the exception thrown
        if (!(e.getCause() instanceof IndexOutOfBoundsException)) {
            throw e;
        }
        // Ensure the traceback string is a formatted Python traceback
        testTracebackString(e, "IndexError");
    }

    private static void testTracebackString(JepException e, String expectedType) {
        // Ensure the exception has a traceback
        if (e.getPythonTraceback() == null) {
            throw new RuntimeException(
                    "Expected a Python traceback, but got null instead");
        }

        // Ensure the traceback string matches a formatted Python traceback
        String tbString = e.getPythonTraceback();
        if (tbString == null) {
            throw new RuntimeException(
                    "getPythonTraceback() returned null for a Python exception");
        }
        if (!tbString.contains("Traceback")) {
            throw new RuntimeException(
                    "Traceback missing expected header: " + tbString);
        }
        if (!tbString.contains(expectedType)) {
            throw new RuntimeException(
                    "Traceback missing exception type '" + expectedType + "': " + tbString);
        }
    }
}
