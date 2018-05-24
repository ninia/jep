package jep.test;

import java.util.List;
import java.util.Map;

import jep.Jep;
import jep.JepException;

/**
 * @author bsteffen
 * @since 3.8
 */
public class TestGetWithClass {

    public static void testStr(Jep jep) throws JepException {
         String s = jep.getValue("'abc'", String.class);
         if (!s.equals("abc")) {
             throw new IllegalStateException(s + " is not 'abc'"); 
         } 
         Character c = jep.getValue("'a'", Character.class);
         if (!c.equals('a')) {
             throw new IllegalStateException(c + " is not 'a'"); 
         }
         try {
             c = jep.getValue("'abc'", Character.class);
             throw new IllegalStateException("'abc' is not a Character(" + c + ")");  
         } catch (JepException e) {
             /* This is what should happen. */
         }
    }

    public static void testBool(Jep jep) throws JepException {
         Boolean b = jep.getValue("True", Boolean.class);
         if (!b.booleanValue()) {
             throw new IllegalStateException(b + " is not True"); 
         }
         b = jep.getValue("False", Boolean.class);
         if (b.booleanValue()) {
             throw new IllegalStateException(b + " is not False"); 
         }
    }

    public static void testInt(Jep jep) throws JepException {
         Byte b = jep.getValue("2", Byte.class);
         if (!b.equals((byte) 2)) {
             throw new IllegalStateException(b + " is not 2"); 
         }
         Short s = jep.getValue("2", Short.class);
         if (!s.equals((short) 2)) {
             throw new IllegalStateException(s + " is not 2"); 
         }
         Integer i = jep.getValue("2", Integer.class);
         if (!i.equals((int) 2)) {
             throw new IllegalStateException(i + " is not 2"); 
         }
         Long l = jep.getValue("2", Long.class);
         if (!l.equals((long) 2)) {
             throw new IllegalStateException(l + " is not 2"); 
         }
         /* Test byte overflow causes a JepException */
         try {
             b = jep.getValue("1000", Byte.class);
             throw new IllegalStateException("1000 is not a Byte(" + b + ")");  
         } catch (JepException e) {
             /* This is what should happen. */
         }
    }

    public static void testFloat(Jep jep) throws JepException {
         Float f = jep.getValue("1.5", Float.class);
         if (f.floatValue() != 1.5f) {
             throw new IllegalStateException(f + " is not 1.5"); 
         }
         Double d = jep.getValue("1.5", Double.class);
         if (d.doubleValue() != 1.5) {
             throw new IllegalStateException(d + " is not 1.5"); 
         }
    }

    public static void testSeq(Jep jep) throws JepException {
         List l = jep.getValue("[True]", List.class);
         if (l.size() != 1 || !l.get(0).equals(Boolean.TRUE)) {
             throw new IllegalStateException(l + " is not [True]"); 
         }
         l = jep.getValue("(True,)", List.class);
         if (l.size() != 1 || !l.get(0).equals(Boolean.TRUE)) {
             throw new IllegalStateException(l + " is not [True]"); 
         }
         Boolean[] a = jep.getValue("(True,)", Boolean[].class);
         if (a.length != 1 || !a[0].booleanValue()) {
             throw new IllegalStateException(a + " is not [True]"); 
         }
         boolean[] a2 = jep.getValue("[True]", boolean[].class);
         if (a2.length != 1 || !a2[0]) {
             throw new IllegalStateException(a + " is not [True]"); 
         }
    }

    public static void testDict(Jep jep) throws JepException {
         Map m = jep.getValue("{'abc':'def'}", Map.class);
         if (m.size() != 1 || !m.get("abc").equals("def")) {
             throw new IllegalStateException(m + " is not {'abc':'def'}"); 
         }
    }

    public static void testNone(Jep jep) throws JepException {
         Object o = jep.getValue("None", Object.class);
         if (o != null) {
             throw new IllegalStateException(o + " is not null"); 
         }
         Double d = jep.getValue("None", Double.class);
         if (d != null) {
             throw new IllegalStateException(d + " is not null"); 
         }
         String s = jep.getValue("None", String.class);
         if (s != null) {
             throw new IllegalStateException(s + " is not null"); 
         }
         Jep j = jep.getValue("None", Jep.class);
         if (j != null) {
             throw new IllegalStateException(j + " is not null"); 
         }
    }

    public static void testIncompatible(Jep jep) throws JepException {
         try {
             Integer i = jep.getValue("object", Integer.class);
             throw new IllegalStateException("object is not a Integer(" + i + ")");  
         } catch (JepException e) {
             /* This is what should happen. */
         }
         try {
             Integer i = jep.getValue("object()", Integer.class);
             throw new IllegalStateException("object() is not a Integer(" + i + ")");  
         } catch (JepException e) {
             /* This is what should happen. */
         }
    }

    public static void main(String[] args) throws JepException {
        try (Jep jep = new Jep()) {
            testStr(jep);
            testBool(jep);
            testInt(jep);
            testFloat(jep);
            testSeq(jep);
            testDict(jep);
            testNone(jep);
            testIncompatible(jep);
        }
    }

}
