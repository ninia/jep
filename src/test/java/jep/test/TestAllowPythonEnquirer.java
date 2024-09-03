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
     * Mock naive ClassEnquirer that thinks Java packages are re and java.
     */
    public static class ReAndJavaClassEnquirer implements ClassEnquirer {

        @Override
        public boolean isJavaPackage(String name) {
            return "re".equals(name) || "java".equals(name)
                    || name.startsWith("java.");
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
         * Test that the re and java enquirer fails to import a Python type that
         * exists
         */
        JepConfig config = new JepConfig();
        ClassEnquirer reJavaEnquirer = new ReAndJavaClassEnquirer();
        config.setClassEnquirer(reJavaEnquirer);
        try (Interpreter interp = config.createSubInterpreter()) {

            boolean gotClassNotFoundException = false;
            try {
                interp.exec("import sys");
                interp.exec("if 're' in sys.modules:\n"
                        + "    sys.modules.pop('re')");
                interp.exec("from re import Pattern");
            } catch (JepException e) {
                if (e.getCause() instanceof ClassNotFoundException) {
                    /*
                     * Tested ok, we expected a failure to import as it tried to
                     * import Pattern from re package as if it was a Java class.
                     * ClassNotFoundException indicates it was a failure to
                     * import from Java.
                     */
                    gotClassNotFoundException = true;
                } else {
                    throw e;
                }
            }

            if (!gotClassNotFoundException) {
                System.err.println(
                        "Expected a failed Java import of 'from re import Pattern'");
                System.exit(1);
            }
        }

        /*
         * Test that the allow python enquirer does not delegate to the re and
         * java enquirer when encountering re
         */
        config = new JepConfig();
        config.setClassEnquirer(
                new AllowPythonClassEnquirer(reJavaEnquirer, "re"));
        try (Interpreter interp = config.createSubInterpreter()) {
            interp.exec("from re import Pattern");
            /*
             * This should still work as java.util.HashMap is on the classpath
             * and the enquirer should indicate it's a Java import
             */
            interp.exec("from java.util import HashMap");
        } catch (Exception e) {
            e.printStackTrace();
            System.err.println(
                    "Expected a successful Python import of 'from re import Pattern'"
                            + " and a successful Java import of 'from java.util import HashMap");
            System.exit(1);
        }

        /*
         * Test that even if a Java package is available, if it's declared as a
         * Python package then it will go to the Python importer.
         */
        config = new JepConfig();
        config.setClassEnquirer(
                new AllowPythonClassEnquirer(reJavaEnquirer, "java.lang.ref"));
        try (Interpreter interp = config.createSubInterpreter()) {
            interp.exec("moduleNotFound = False");
            interp.exec("try:\n" + "    from java.lang.ref import Reference\n"
                    + "except ModuleNotFoundError as e:\n"
                    + "    moduleNotFound = True");
            boolean moduleNotFoundErrorWasRaised = interp
                    .getValue("moduleNotFound", Boolean.class);
            if (!moduleNotFoundErrorWasRaised) {
                System.err.println("Expected ModuleNotFoundError when running "
                        + "'from java.lang.ref import Reference'"
                        + " with ClassEnquirer that considers the "
                        + "java.lang.ref package as a Python package");
                System.exit(1);
            }

            /*
             * Verify java.lang.reflect still works even though java.lang.ref
             * was declared as a Python package (i.e. so it's a little smarter
             * than doing just a String.startsWith()). It would throw an
             * uncaught exception if the import failed.
             */
            interp.exec("from java.lang.reflect import Method");
        }
    }

}
