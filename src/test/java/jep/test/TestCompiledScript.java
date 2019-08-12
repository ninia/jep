package jep.test;

import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;
import jep.SubInterpreter;

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
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.eval("import py_compile");
            interp.eval(
                    "py_compile.compile(file='build/testScript.py', cfile='build/testScript.pyc')");
            interp.eval(null);
            interp.runScript("build/testScript.pyc");
            result = interp.getValue("isGood()");
        }
        if (!Boolean.TRUE.equals(result)) {
            throw new IllegalStateException("isGood() returned " + result);
        }
    }

}
