package jep.test;

import jep.Interpreter;
import jep.JepConfig;
import jep.SubInterpreter;

/**
 * A test class for regression testing that Jep.getValue() does not crash when
 * it returns a temporary object. Temporary objects are created within the code
 * passed to getValue and are not referenced anywhere else so that they are
 * garbage collected before the call to getValue returns.
 * 
 * @author Ben Steffensmeier
 * @since 3.7
 */
public class TestGetTempValue {

    public static void main(String[] args) throws Exception {
        try (Interpreter interp = new SubInterpreter(
                new JepConfig().setIncludePath("."))) {
            interp.eval("import tempfile");
            interp.eval("import os.path");
            interp.eval("import os");
            interp.eval(
                    "testfile = os.path.join(tempfile.gettempdir(), 'jepTestFile')");
            /*
             * This may leak a file handle but it is the best way to test for
             * crashes
             */
            interp.getValue("open(testfile, 'w')");
            interp.eval("os.remove(testfile)");
            /* Extra objects that didn't crash all versions of python */
            interp.getValue("[]");
            interp.getValue("()");
            interp.getValue("{}");
            interp.getValue("0");
            interp.getValue("0.0");
            interp.getValue("''");
            interp.getValue("([0.0], {0:''})");
        }
    }

}
