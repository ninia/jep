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

    public static class AddClass {
        public int __add__(int value) {
            return value + 1;
        }
    }
}
