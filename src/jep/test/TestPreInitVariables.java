package jep.test;

import jep.Jep;
import jep.JepException;
import jep.PyConfig;

/**
 * A test class for verifying that Jep can correctly configure the global Python
 * interpreter before actually starting the interpreter. This test exercises
 * PyConfig options to match Python command line arguments, and checks the
 * output against sys.flags.
 * 
 * https://github.com/mrj0/jep/issues/49
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 */
public class TestPreInitVariables {

    public static void main(String[] args) {
        Jep jep = null;
        PyConfig pyConfig = new PyConfig();
        pyConfig.setIgnoreEnvironmentFlag(1);
        // TODO fix test so no site flag can be tested
        // pyConfig.setNoSiteFlag(1);
        pyConfig.setNoUserSiteDirectory(1);
        try {
            Jep.setInitParams(pyConfig);
            jep = new Jep();
            jep.runScript("tests/subprocess/py_preinit.py");
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (jep != null) {
                jep.close();
            }
        }

    }

}
