package jep.test;

public class TestPyJType {

    public static interface Root {
    }

    public static interface Child extends Root{
    }

    /**
     * The default Python mro cannot create a consistent method resolution
     * order (MRO) for this interface.
     */
    public static interface ProblemInterface extends Root, Child {
    }

    /**
     * The default Python mro cannot create a consistent method resolution
     * order (MRO) for this class
     */
    public static class ProblemClass implements Root, Child {
    }

    public static class InheritedProblemClass implements ProblemInterface {
    }
}
