package jep.test;

import java.util.List;

import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;
import jep.MainInterpreter;
import jep.SubInterpreter;

/**
 * Tests that shared modules can have sys.argv set before they are imported as
 * shared modules.
 * 
 * Created: July 2017
 * 
 * @author Nate Jensen
 * @see "https://github.com/ninia/jep/issues/81"
 */
public class TestSharedArgv {

    @SuppressWarnings("unchecked")
    public static void main(String[] args) throws JepException {

        final String[] argv = new String[] { "", "-h", "other" };

        MainInterpreter.setSharedModulesArgv(argv);
        JepConfig cfg = new JepConfig();
        cfg.addSharedModules("logging");
        cfg.addIncludePaths(".");

        try (Interpreter interp = new SubInterpreter(cfg)) {
            /*
             * since logging is a shared module in this test it will
             * automatically import and get its sys.argv setup in the Jep
             * constructor call
             */
            interp.eval("import logging");
            List<String> result = (List<String>) interp
                    .getValue("logging.sys.argv");
            for (int i = 0; i < result.size(); i++) {
                if (!result.get(i).equals(argv[i])) {
                    throw new RuntimeException("argv[" + i + "] did not match");
                }
            }
        }
    }

}
