package jep.test;

import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;
import jep.SubInterpreter;

/**
 * Tests that shared modules are really shared between multiple instances of Jep
 * and that an instance of Jep that is configured to not share the module will
 * recieve its own independent instance.
 * 
 * Created: August 2016
 * 
 * @author Ben Steffensmeier
 */
public class TestSharedModules {

    public static void main(String[] args) throws JepException {
        JepConfig config = new JepConfig().addIncludePaths(".")
                .addSharedModules("datetime");
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.eval("import datetime");
            interp.eval("setattr(datetime, 'shared', True)");
        }
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.eval("import datetime");
            Object hasattr = interp.getValue("hasattr(datetime,'shared')");
            if (!Boolean.TRUE.equals(hasattr)) {
                throw new IllegalStateException(
                        "datetime module was not shared when it should be.");
            }
        }
        config = new JepConfig().addIncludePaths(".");
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.eval("import datetime");
            Object hasattr = interp.getValue("hasattr(datetime,'shared')");
            if (!Boolean.FALSE.equals(hasattr)) {
                throw new IllegalStateException(
                        "datetime module was shared when it should not be.");
            }
        }
        /*
         * Check that python 2.7 is not putting shared modules in restricted
         * mode
         */
        config = new JepConfig().addIncludePaths(".")
                .addSharedModules("xml.etree.ElementTree");
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.eval("import xml.etree.ElementTree as ET");
            interp.eval("ET.parse('fakeFile.xml')");
        } catch (JepException e) {
            /*
             * Ignore exception because file does not exist but throw exceptions
             * about restricted mode.
             */
            if (e.getMessage().contains("restricted mode")) {
                throw e;
            }
        }
        System.exit(0);
    }

}
