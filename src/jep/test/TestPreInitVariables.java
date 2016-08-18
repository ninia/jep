package jep.test;

import jep.Jep;
import jep.PyConfig;

/**
 * A test class for verifying that Jep can correctly configure the global Python
 * interpreter with specific flags before actually starting the interpreter.
 * This test exercises PyConfig options to match Python command line arguments
 * and checks the output against sys.flags.
 * 
 * https://github.com/mrj0/jep/issues/49
 * 
 * TODO: If you wanted to be extra-thorough you could write tests that don't
 * explicitly trust sys.flags, e.g. verify that when setting
 * Py_DontWriteBytecodeFlag that a .pyc or .pyo is not actually written out.
 * 
 * @author Nate Jensen
 */
public class TestPreInitVariables {

    public static void main(String[] args) {
        Jep jep = null;
        PyConfig pyConfig = new PyConfig();
        pyConfig.setIgnoreEnvironmentFlag(1);
        // TODO fix test so no site flag can be tested
        // pyConfig.setNoSiteFlag(1);
        pyConfig.setNoUserSiteDirectory(1);
        // verbose prints out too much, when it's on, it's clear it's on
        // pyConfig.setVerboseFlag(1);
        pyConfig.setOptimizeFlag(1);
        pyConfig.setDontWriteBytecodeFlag(1);
        pyConfig.setHashRandomizationFlag(1);
        try {
            Jep.setInitParams(pyConfig);
            jep = new Jep(false, ".");
            jep.eval("import sys");
            assert 1 == (Integer) jep.getValue("sys.flags.ignore_environment");
            assert 0 == (Integer) jep.getValue("sys.flags.no_site");
            assert 1 == (Integer) jep.getValue("sys.flags.no_user_site");
            assert 0 == (Integer) jep.getValue("sys.flags.verbose");
            assert 1 == (Integer) jep.getValue("sys.flags.optimize");
            assert 1 == (Integer) jep.getValue("sys.flags.dont_write_bytecode");
            assert 1 == (Integer) jep.getValue("sys.flags.hash_randomization");
        } catch (Throwable e) {
            e.printStackTrace();
            System.exit(1);
        } finally {
            if (jep != null) {
                jep.close();
            }
        }
        System.exit(0);
    }

}
