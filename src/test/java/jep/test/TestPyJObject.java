package jep.test;

/**
 * Supporting test_object.py
 */
public class TestPyJObject {

    public static class ReprClass {
        public String __repr__() {
            return getClass().getSimpleName();
        }
    }

    public static class ReprSubClass extends ReprClass {
    }

}
