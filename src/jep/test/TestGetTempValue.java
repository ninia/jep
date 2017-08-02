package jep.test;

import jep.Jep;
import jep.JepConfig;

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
        try (Jep jep = new Jep(new JepConfig().setIncludePath("."))) {
            jep.eval("import tempfile");
            jep.eval("import os.path");
            jep.eval("import os");
            jep.eval("testfile = os.path.join(tempfile.gettempdir(), 'jepTestFile')");
            /* This may leak a file handle but it is the best way to test for crashes */
            jep.getValue("open(testfile, 'w')");
            jep.eval("os.remove(testfile)");
            /* Extra objects that didn't crash all versions of python */
	    jep.getValue("[]");
            jep.getValue("()");
            jep.getValue("{}");
            jep.getValue("0");
            jep.getValue("0.0");
            jep.getValue("''");
            jep.getValue("([0.0], {0:''})");
        }
    }

}
