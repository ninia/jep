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
            interp.exec("import datetime");
            // Modules store loader in two places, they should always be the same
            Boolean loaderMatch = interp.getValue("datetime.__loader__ == datetime.__spec__.loader", Boolean.class);
            if (!Boolean.TRUE.equals(loaderMatch)) {
                throw new IllegalStateException(
                        "datetime module spec has wrong loader");
            }
            /*
             * Shared modules are loaded by the normal loader on the main thread and the jep loader
             * on the sub-interpreter thread. Since it is the same module it can only have one loader
             * and it should be the normal loader so that jep does not interfere with any further loading.
             */
            Boolean jepLoader = interp.getValue("isinstance(datetime.__spec__.loader, jep.shared_modules_hook.JepSharedModuleImporter)", Boolean.class);
            if (!Boolean.FALSE.equals(jepLoader)) {
                throw new IllegalStateException(
                        "datetime module spec is JepSharedModuleImporter");
            }
            Boolean initializing = interp.getValue("datetime.__spec__._initializing", Boolean.class);
            if (!Boolean.FALSE.equals(initializing)) {
                throw new IllegalStateException(
                        "datetime module is still initializing");
            }
            Boolean inSysModules = interp.getValue("sys.modules['datetime'] is datetime", Boolean.class);
            if (!Boolean.TRUE.equals(inSysModules)) {
                throw new IllegalStateException(
                        "datetime module is not in sys.modules");
            }
            interp.exec("setattr(datetime, 'shared', True)");
        }
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.exec("import datetime");
            Boolean hasattr = interp.getValue("hasattr(datetime,'shared')", Boolean.class);
            if (!Boolean.TRUE.equals(hasattr)) {
                throw new IllegalStateException(
                        "datetime module was not shared when it should be.");
            }
        }
        config = new JepConfig().addIncludePaths(".");
        try (Interpreter interp = new SubInterpreter(config)) {
            interp.exec("import datetime");
            Boolean hasattr = interp.getValue("hasattr(datetime,'shared')", Boolean.class);
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
            interp.exec("import xml.etree.ElementTree as ET");
            interp.exec("ET.parse('fakeFile.xml')");
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
