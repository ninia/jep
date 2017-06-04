package jep.test;

import java.lang.IllegalStateException;
import java.lang.Boolean;

import jep.Jep;
import jep.JepConfig;
import jep.JepException;

/**
 * Tests that a compiled script can be loaded by Jep.runscript()
 * 
 * Created: May 2017
 * 
 * @author Ben Steffensmeier
 */
public class TestCompiledScript {

    public static void main(String[] args) throws JepException{
        JepConfig config = new JepConfig();
        config.addIncludePaths(".");
        Jep jep = new Jep(config);
        jep.eval("import py_compile");
        jep.eval("py_compile.compile(file='build/testScript.py', cfile='build/testScript.pyc')");
        jep.eval(null);
        jep.runScript("build/testScript.pyc");
        Object result = jep.getValue("isGood()");
        jep.close();
        if(!Boolean.TRUE.equals(result)){
            throw new IllegalStateException("isGood() returned " + result);
        }
    }

}
