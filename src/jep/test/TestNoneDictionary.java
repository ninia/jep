package jep.test;

import jep.Jep;
import jep.JepException;

/**
 * A test class that verifies a Python dictionary is correctly transformed into
 * a Java HashMap with Jep.getValue(String).
 * 
 * https://github.com/mrj0/jep/issues/40
 * 
 * @author soyvv and ndjensen
 */
public class TestNoneDictionary {

    public static void main(String[] args) {
        try {
            final Jep jep = new Jep(false);
            jep.eval("x = {'abc': 111, 'bcd': '222', 'cde': None, None: '333'}");
            jep.eval("def gx():\n\treturn x");
            final Object x = jep.getValue("x");
            final Object xs = jep.invoke("gx");
            System.out.println(x);
            System.out.println(xs);
            jep.close();
        } catch (final JepException e) {
            e.printStackTrace();
        }

    }

}
