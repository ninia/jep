package jep.tests;

import java.util.Iterator;
import java.util.List;

import javax.script.ScriptEngineFactory;
import javax.script.ScriptEngineManager;

import junit.framework.TestCase;

public final class TestJepScriptEngineFactory extends TestCase {
    /**
     * Check that JepScriptEngineFactory is discovered.
     */
    public void testDiscovery() {
        ScriptEngineManager scriptManager = new ScriptEngineManager();
        List<ScriptEngineFactory> factories = scriptManager.getEngineFactories();
        Iterator<ScriptEngineFactory> iter = factories.iterator();
        while (iter.hasNext()) {
            ScriptEngineFactory factory = iter.next();
            if (factory.getEngineName().equals("jep")) {
                return;
            }
        }
        assertTrue("JepScriptEngineFactory not discovered", true);
    }
}
