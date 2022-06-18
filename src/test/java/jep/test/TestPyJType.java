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

    public static interface InterfaceWithDefault {
        public default String checkPrecedence() {
            return "InterfaceWithDefault";
        }
    }

    public static class ParentClassWithMethod {
        public String checkPrecedence() {
            return "ParentClassWithMethod";
        }
    }

    /**
     * In Java the parent class method will override interface default method. PyJType should match Java behavior.
     */
    public static class ChildTestingMethodInheritance extends ParentClassWithMethod implements InterfaceWithDefault {
    }

    public static class ClassInheritingDefault implements InterfaceWithDefault {
    }

}
