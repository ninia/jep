package jep.test;

import java.util.List;
import java.util.Map;

import jep.Jep;
import jep.JepException;

/**
 * A test class for verifying that pyembed_box_py (ie Jep.getValue(String)) is
 * working well and not leaking memory.
 * 
 * 
 * Created: Thu Jul 09 2015
 * 
 * @author [ndjensen at gmail.com] Nate Jensen
 * @version $Id$
 */
public class TestBoxing {

    // high number to check for memory leaks
    private static final int REPEAT = 10000000;

    /**
     * @param args
     * @throws Exception
     */
    public static void main(String[] args) throws Exception {
        Jep jep = null;
        try {
            jep = new Jep(false);
            for (int i = 0; i < REPEAT; i++) {
                testList(jep);
                testTuple(jep);
                testDictionary(jep);
            }
        } catch (JepException e) {
            e.printStackTrace();
        } finally {
            if (jep != null) {
                System.out.println("closing jep interpreter");
                jep.close();
            }
        }
        System.out.println("java main() finished");
    }

    public static void testList(Jep jep) throws Exception {
        jep.eval("x = [0, 1, 2, 'three', 4, 5.0, 6]");
        List<?> x = (List<?>) jep.getValue("x");
        assert x.size() == 7;
        assert x.get(0).equals(0);
        assert x.get(1).equals(1);
        assert x.get(2).equals(2);
        assert x.get(3).equals("three");
        assert x.get(4).equals(4);
        assert x.get(5).equals(5.0f);
        assert x.get(6).equals(6);
        jep.eval("del x");
    }

    @SuppressWarnings("unchecked")
    public static void testTuple(Jep jep) throws Exception {
        jep.eval("x = ('abc', 'def')");
        List<String> x = (List<String>) jep.getValue("x");
        assert x.size() == 2;
        assert x.get(0).equals("abc");
        assert x.get(1).equals("def");
        try {
            x.add("shouldn't be allowed");
            assert false;
        } catch (UnsupportedOperationException e) {
            // good to reach this
        }
        jep.eval("del x");
    }

    @SuppressWarnings("unchecked")
    public static void testDictionary(Jep jep) throws Exception {
        jep.eval("y = [1, 2, 3, 4]");
        jep.eval("x = {'a':'123', 'b':'cdef', 'c':y}");
        Map<String, Object> x = (Map<String, Object>) jep.getValue("x");
        assert x.size() == 3;
        assert x.get("a").equals("123");
        assert x.get("b").equals("cdef");
        List<Integer> y = (List<Integer>) x.get("c");
        assert y.size() == 4;
        assert (y.get(0)) == 1;
        assert (y.get(1)) == 2;
        assert (y.get(2)) == 3;
        assert (y.get(3)) == 4;
        jep.eval("del x");
        jep.eval("del y");
    }
}
