package jep.test;

import jep.Jep;
import jep.JepConfig;

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

    public static void main(String[] args) {
        Jep jep = null;
        try {
            JepConfig config = new JepConfig().addIncludePaths(".")
                    .addSharedModules("datetime");
            jep = new Jep(config);
            jep.eval("import datetime");
            jep.eval("setattr(datetime, 'shared', True)");
            jep.close();
            jep = new Jep(config);
            jep.eval("import datetime");
            Object hasattr = jep.getValue("hasattr(datetime,'shared')");
            jep.close();
            if (!Boolean.TRUE.equals(hasattr)) {
                throw new IllegalStateException(
                        "datetime module was not shared when it should be.");
            }
            config = new JepConfig().addIncludePaths(".");
            jep = new Jep(config);
            jep.eval("import datetime");
            hasattr = jep.getValue("hasattr(datetime,'shared')");
            jep.close();
            if (!Boolean.FALSE.equals(hasattr)) {
                throw new IllegalStateException(
                        "datetime module was shared when it should not be.");
            }
            System.exit(0);
        } catch (Throwable e) {
            if (jep != null) {
                jep.close();
            }
            e.printStackTrace();
            System.exit(1);
        }
        System.exit(0);
    }

}
