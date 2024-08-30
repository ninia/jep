package jep.test;

import jep.AllowPythonClassEnquirer;
import jep.ClassEnquirer;
import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;

/**
 * Tests for the AllowPythonClassEnquirer.
 *
 * Created: August 2024
 *
 * @author Nate Jensen
 */
public class TestAllowPythonEnquirer {

    /**
     * Mock naive ClassEnquirer that thinks every package is a Java package.
     */
    public static class EverythingIsJavaClassEnquirer implements ClassEnquirer {

        @Override
        public boolean isJavaPackage(String name) {
            return true;
        }

        @Override
        public String[] getClassNames(String pkgName) {
            return null;
        }

        @Override
        public String[] getSubPackages(String pkgName) {
            return null;
        }

    }

    public static void main(String[] args) throws JepException {
        /*
         * Test that the always Java enquirer fails to import a Python type that
         * exists
         */
        JepConfig config = new JepConfig();
        ClassEnquirer allJavaEnquirer = new EverythingIsJavaClassEnquirer();
        config.setClassEnquirer(allJavaEnquirer);
        try (Interpreter interp = config.createSubInterpreter()) {

            boolean gotClassNotFoundException = false;
            try {
                interp.exec("import sys");
                interp.exec("if 'io' in sys.modules:\n"
                        + "    sys.modules.pop('io')");
                interp.exec("from io import BytesIO");
            } catch (JepException e) {
                if (e.getCause() instanceof ClassNotFoundException) {
                    /*
                     * Tested ok, we expected a failure to import io as it tried
                     * to import BytesIO from Java. ClassNotFoundException
                     * indicates it was a failure to import from Java.
                     */
                    gotClassNotFoundException = true;
                }
            }

            if (!gotClassNotFoundException) {
                System.err.println(
                        "Expected a failed Java import of 'from io import BytesIO'");
                System.exit(1);
            }
        }

        /*
         * Test that the allow python enquirer does not delegate to the all Java
         * enquirer when encountering io
         */
        config = new JepConfig();
        config.setClassEnquirer(
                new AllowPythonClassEnquirer(allJavaEnquirer, "io"));
        try (Interpreter interp = config.createSubInterpreter()) {
            interp.exec("import sys");
            interp.exec("if 'io' in sys.modules:\n    sys.modules.pop('io')");
            interp.exec("from io import BytesIO");
            /*
             * This should still work as java.util.HashMap is on the classpath
             * and the enquirer should indicate it's a Java import
             */
            interp.exec("from java.util import HashMap");
        } catch (Exception e) {
            System.err.println(
                    "Expected a successful Python import of 'from io import BytesIO'"
                            + " and a successful Java import of 'from java.util import HashMap");
            System.exit(1);
        }

        /*
         * Test that even if a Java package is available, if it's declared as a
         * Python package then it will go to the Python importer.
         */
        config = new JepConfig();
        config.setClassEnquirer(
                new AllowPythonClassEnquirer(allJavaEnquirer, "java"));
        try (Interpreter interp = config.createSubInterpreter()) {
            interp.exec("moduleNotFound = False");
            interp.exec("try:\n" + "    from java.util import ArrayList\n"
                    + "except ModuleNotFoundError as e:\n"
                    + "    moduleNotFound = True");
            boolean moduleNotFoundErrorWasRaised = interp
                    .getValue("moduleNotFound", Boolean.class);
            if (!moduleNotFoundErrorWasRaised) {
                System.err.println("Expected ModuleNotFoundError when running "
                        + "'from java import util'"
                        + " with ClassEnquirer that considers java package as a Python package");
                System.exit(1);
            }

            /*
             * Verify javax still works even though java was declared as a
             * Python package (i.e. so it's a little smarter than doing just a
             * String.startsWith()). It would throw an uncaught exception if the
             * import failed.
             */
            interp.exec("from javax.xml.bind import JAXB");
        }
    }

}
