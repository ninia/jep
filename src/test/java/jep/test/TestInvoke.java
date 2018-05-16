package jep.test;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import jep.Jep;
import jep.JepConfig;
import jep.JepException;

/**
 * Tests that the variations of the Jep.invoke(...) method work correctly. Also
 * tests a few error possibilities.
 * 
 * Created: September 2017
 * 
 * @author Nate Jensen
 * @since 3.8
 */
public class TestInvoke {

    public static void main(String[] args) throws JepException {
        JepConfig config = new JepConfig();
        config.addIncludePaths("src/test/python/subprocess");

        try (Jep jep = new Jep(config)) {
            jep.eval("from invoke_args import *");

            // test a basic invoke with no args
            Object result = jep.invoke("invokeNoArgs");
            if (result != null) {
                throw new IllegalStateException(
                        "Received " + result + " but expected null");
            }

            // test a basic invoke with arguments
            result = jep.invoke("invokeArgs", "a", null, 5.4);
            if (result == null || !result.equals(Boolean.TRUE)) {
                throw new IllegalStateException(
                        "Received " + result + " but expected true");
            }

            // test that args are passed in order
            result = jep.invoke("invokeVarArgsExplicit", true, null, 2, "xyz");
            if (result != null) {
                throw new IllegalStateException(
                        "Received " + result + " but expected null");
            }

            // test *args
            result = jep.invoke("invokeVarArgs", true, null, 2, "xyz");
            if (result == null || !result.equals("xyz")) {
                throw new IllegalStateException(
                        "Received " + result + " but expected xyz");
            }

            Map<String, Object> kwMap = new HashMap<>();
            kwMap.put("arg4", "xyz");
            kwMap.put("arg5", new ArrayList<>());
            kwMap.put("argnull", null);

            // test that keys/values are mapped to correct arguments
            result = jep.invoke("invokeKeywordArgsExplicit", kwMap);
            if (result == null || !(result instanceof ArrayList)) {
                throw new IllegalStateException(
                        "Received " + result + " but expected ArrayList");
            }

            // test **kwargs
            result = jep.invoke("invokeKeywordArgs", kwMap);
            if (result != null) {
                throw new IllegalStateException(
                        "Received " + result + " but expected null");
            }

            // test a mixture of varargs and kwargs
            result = jep.invoke("invokeArgsAndKeywordArgs",
                    new Object[] { 15, "add", false }, kwMap);
            if (result == null || !(result instanceof List)) {
                throw new IllegalStateException(
                        "Received " + result + " but expected List");
            }

            // keywords must be strings
            try {
                kwMap.put(null, "default");
                result = jep.invoke("invokeArgsAndKeywordArgs",
                        new Object[] { 15, "add", false }, kwMap);
            } catch (JepException e) {
                if (!e.getMessage().contains("TypeError")) {
                    throw new IllegalStateException(
                            "Bad error message, error did not include TypeError");
                }
            }

            // test that you can't call an Object that doesn't exist
            try {
                result = jep.invoke("anything");
            } catch (JepException e) {
                if (!e.getMessage().contains("anything")) {
                    throw new IllegalStateException(
                            "Bad error message, error did not include missing object name");
                }
            }

            // allow attribute lookup
            result = jep.invoke("objectWithMethod.theMethod", new Object());

            // test that you can't call attributes that don't exist.
            try {
                result = jep.invoke("objectWithMethod.anything");
            } catch (JepException e) {
                if (!e.getMessage().contains("anything")) {
                    throw new IllegalStateException(
                            "Bad error message, error did not include missing attribute name");
                }
            }
        }
    }

}
