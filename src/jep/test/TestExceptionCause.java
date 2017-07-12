package jep.test;

import java.lang.IndexOutOfBoundsException;

import jep.Jep;
import jep.JepConfig;
import jep.JepException;

/**
 * Tests that a java excetion thrown while executing python is set as the cause
 * of the JepException.
 * 
 * Created: July 2017
 * 
 * @author Ben Steffensmeier
 */
public class TestExceptionCause {

    public static void main(String[] args) throws JepException {
        JepConfig config = new JepConfig().addIncludePaths(".");
        try (Jep jep = new Jep(config)){
            jep.eval("from java.util import ArrayList");
            try{
                jep.eval("ArrayList().get(0)");
            }catch(JepException e){
                 if (!(e.getCause() instanceof IndexOutOfBoundsException)) {
                     throw e;
                 }
            }
            try{
                jep.eval("try:\n  ArrayList().get(0)\nexcept AttributeError:\n  pass");
            }catch(JepException e){
                 if (!(e.getCause() instanceof IndexOutOfBoundsException)) {
                     throw e;
                 }
            }
        }
    }

}
