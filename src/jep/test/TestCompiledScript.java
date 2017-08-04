package jep.test;

import jep.Jep;
import jep.JepConfig;
import jep.JepException;

/**
 * Tests that a compiled script can be loaded by Jep.runscript()
 * 
 * Created: May 2017
 * 
 * @author Ben Steffensmeier
 * @see "https://github.com/ninia/jep/issues/77"
 */
public class TestCompiledScript {

    public static void main(String[] args) throws JepException {
        Object result = null;
        JepConfig config = new JepConfig();
        config.addIncludePaths(".");
        try (Jep jep = new Jep(config)) {
            jep.eval("import py_compile");
            jep.eval(
                    "py_compile.compile(file='build/testScript.py', cfile='build/testScript.pyc')");
            jep.eval(null);
            jep.runScript("build/testScript.pyc");
            result = jep.getValue("isGood()");
        }
        if (!Boolean.TRUE.equals(result)) {
            throw new IllegalStateException("isGood() returned " + result);
        }
    }

}
