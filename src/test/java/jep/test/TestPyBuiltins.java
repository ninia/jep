package jep.test;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import jep.Interpreter;
import jep.SharedInterpreter;
import jep.JepException;
import jep.python.PyBuiltins;
import jep.python.PyCallable;
import jep.python.PyObject;

public class TestPyBuiltins {

    private Interpreter interp;

    private PyBuiltins builtins;

    /* An instance of the object type */
    private PyObject object;
    
    /* The object type */
    private PyCallable objectType;

    /* A type that extends object */
    private PyCallable subtypeOne;

    /* Another type that extends object */
    private PyCallable subtypeTwo;

    /* An instance of one of the subtypes above*/
    private PyObject subobject;

    /* A dict for testing exec and eval */
    private PyObject globals;

    /* A dict for testing exec and eval */
    private PyObject locals;

    /* Set to a non-null value to fail the test */
    private String failure;

    private boolean testCallable() {
        if (!builtins.callable(objectType)){
            failure = "callable builtin returns object is not callable";
            return false;
        }
        if (builtins.callable(object)){
            failure = "callable builtin returns an object is callable";
            return false;
        }
        if (builtins.callable(7)){
            failure = "callable builtin returns a number is callable";
            return false;
        }
        return true;
    }

    private boolean testDelAttr() {
        subobject.setAttr("name", "value");
        builtins.delattr(subobject, "name");
        List<String> dir = Arrays.asList(builtins.dir(subobject));
        if (dir.contains("name")){
            failure = "delattr builtin does not delete value";
            return false;
        }
        try {
            builtins.delattr(subobject, "fakeAttribute");
            failure = "delattr did not detect attribute error";
        } catch (JepException e) {
            if (!e.getMessage().contains("AttributeError")) {
                failure = "Not a AttributeError: " + e.getMessage();
            }
        }
        return true;
    }

    private boolean testDict() {
        PyObject dictBuiltin = builtins.dict();
        PyObject dictGetValue = interp.getValue("{}", PyObject.class);
        if (!dictBuiltin.equals(dictGetValue)){
            failure = "dict builtin does not return an empty dict";
            return false;
        }
        Map<String,Integer> map = new HashMap<>();
        map.put("a", 1);
        map.put("b", 2);
        map.put("c", 3);
        dictBuiltin = builtins.dict(map);
        dictGetValue = interp.getValue("{'a':1, 'b':2, 'c':3}", PyObject.class);
        if (!dictBuiltin.equals(dictGetValue)){
            failure = "dict builtin does not return a dict from map";
            return false;
        }
        return true;
    }

    private boolean testDir() {
        List<String> dir = Arrays.asList(builtins.dir(object));
        if (!dir.contains("__str__")){
            failure = "dir builtin does not report __str__";
            return false;
        }
        if (dir.contains("fakeAttribute")){
            failure = "dir builtin reports non-existent attribute";
            return false;
        }
        return true;
    }

    private boolean testEval() {
        Number result = (Number) builtins.eval("1+1", globals);
        if (result.intValue() != 2){
            failure = "eval builtin does not return 1 + 1 = 2";
            return false;
        }
        result = (Number) builtins.eval("1+1", globals, locals);
        if (result.intValue() != 2){
            failure = "eval builtin with locals does not return 1 + 1 = 2";
            return false;
        }
        try {
            result = (Number) builtins.eval("(:", globals, locals);
            failure = "Eval did not detect syntax error";
        } catch (JepException e) {
            if (!e.getMessage().contains("SyntaxError")) {
                failure = "Not a SyntaxError: " + e.getMessage();
            }
        }
        return true;
    }

    private boolean testExec() {
        builtins.exec("result = 1 + 1", globals);
        Number result = (Number) builtins.eval("result", globals);
        if (result.intValue() != 2){
            failure = "exec builtin does not return 1 + 1 = 2";
            return false;
        }
        builtins.exec("result = 1 + 1", globals);
        result = (Number) builtins.eval("result", globals, locals);
        if (result.intValue() != 2){
            failure = "exec builtin with locals does not return 1 + 1 = 2";
            return false;
        }
        try {
            builtins.exec("(:", globals);
            failure = "Exec did not detect syntax error";
        } catch (JepException e) {
            if (!e.getMessage().contains("SyntaxError")) {
                failure = "Not a SyntaxError: " + e.getMessage();
            }
        }
        return true;
    }

    private boolean testFrozenSet() {
        PyObject setBuiltin = builtins.frozenset();
        PyObject setGetValue = interp.getValue("frozenset()", PyObject.class);
        if (!setBuiltin.equals(setGetValue)){
            failure = "frozenset builtin does not return an empty set";
            return false;
        }
        String[] array = new String[] {"1", "2", "3"};
        setGetValue = interp.getValue("{'1','2','3'}", PyObject.class);
        setBuiltin = builtins.frozenset(array);
        if (!setBuiltin.equals(setGetValue)){
            failure = "frozenset builtin does not return a set from array";
            return false;
        }
        setBuiltin = builtins.frozenset(Arrays.asList(array));
        if (!setBuiltin.equals(setGetValue)){
            failure = "frozenset builtin does not return a set from List";
            return false;
        }
        return true;
    }

    private boolean testGetAttr() {
        if (builtins.getattr(object, "__str__") == null){
            failure = "getattr builtin does not find __str__";
            return false;
        }
        try {
            builtins.getattr(subobject, "fakeAttribute");
            failure = "getattr did not detect attribute error";
        } catch (JepException e) {
            if (!e.getMessage().contains("AttributeError")) {
                failure = "Not a AttributeError: " + e.getMessage();
            }
        }
        return true;
    }

    private boolean testHasAttr() {
        if (!builtins.hasattr(object, "__str__")){
            failure = "hasattr builtin does not find __str__";
            return false;
        }
        if (builtins.hasattr(object, "fakeAttribute")){
            failure = "hasattr builtin found fakeAttribute";
            return false;
        }
        return true;
    }

    private boolean testId() {
        if (builtins.id(object) != builtins.id(object)){
            failure = "id builtin returns inconsistent id";
            return false;
        }
        if (builtins.id(object) == builtins.id(objectType)){
            failure = "id builtin returns equal id for inequal objects";
            return false;
        }
        return true;
    }

    private boolean testIsInstance() {
        if (!builtins.isinstance(object, objectType)){
            failure = "isinstance builtin returns an object is not an object type";
            return false;
        }
        if (builtins.isinstance(object, subtypeOne)){
            failure = "isinstance builtin returns an object is a subtype";
            return false;
        }
        try {
            builtins.isinstance(object, object);
            failure = "isinstance did not detect type error";
        } catch (JepException e) {
            if (!e.getMessage().contains("TypeError")) {
                failure = "Not a TypeError: " + e.getMessage();
            }
        }
        return true;
    }

    private boolean testIsSubClass() {
        if (!builtins.issubclass(subtypeOne, objectType)){
            failure = "issubclass builtin returns subtype is not a subclass of object";
            return false;
        }
        if (builtins.issubclass(subtypeOne, subtypeTwo)){
            failure = "issubclass builtin returns unrelated subtype is a subclass";
            return false;
        }
        try {
            builtins.issubclass(object, object);
            failure = "issubclass did not detect type error";
        } catch (JepException e) {
            if (!e.getMessage().contains("TypeError")) {
                failure = "Not a TypeError: " + e.getMessage();
            }
        }
        return true;
    }

    private boolean testList() {
        PyObject listBuiltin = builtins.list();
        PyObject listGetValue = interp.getValue("[]", PyObject.class);
        if (!listBuiltin.equals(listGetValue)){
            failure = "list builtin does not return an empty list";
            return false;
        }
        String[] array = new String[] {"1", "2", "3"};
        listGetValue = interp.getValue("['1','2','3']", PyObject.class);
        listBuiltin = builtins.list(array);
        if (!listBuiltin.equals(listGetValue)){
            failure = "list builtin does not return a list from array";
            return false;
        }
        listBuiltin = builtins.list(Arrays.asList(array));
        if (!listBuiltin.equals(listGetValue)){
            failure = "list builtin does not return a list from List";
            return false;
        }
        return true;
    }

    private boolean testObject() {
        PyObject o = builtins.object();
        if (!builtins.isinstance(object, objectType)){
            failure = "object builtin does not return an empty object";
            return false;
        }
        return true;
    }

    private boolean testSet() {
        PyObject setBuiltin = builtins.set();
        PyObject setGetValue = interp.getValue("set()", PyObject.class);
        if (!setBuiltin.equals(setGetValue)){
            failure = "set builtin does not return an empty set";
            return false;
        }
        String[] array = new String[] {"1", "2", "3"};
        setGetValue = interp.getValue("{'1','2','3'}", PyObject.class);
        setBuiltin = builtins.set(array);
        if (!setBuiltin.equals(setGetValue)){
            failure = "set builtin does not return a set from array";
            return false;
        }
        setBuiltin = builtins.set(Arrays.asList(array));
        if (!setBuiltin.equals(setGetValue)){
            failure = "set builtin does not return a set from List";
            return false;
        }
        return true;
    }

    private boolean testSetAttr() {
        builtins.setattr(subobject, "name", "value");
        if (!"value".equals(subobject.getAttr("name"))){
            failure = "setattr builtin does not set value";
            return false;
        }
        return true;
    }

    private boolean testTuple() {
        PyObject tupleBuiltin = builtins.tuple();
        PyObject tupleGetValue = interp.getValue("()", PyObject.class);
        if (!tupleBuiltin.equals(tupleGetValue)){
            failure = "tuple builtin does not return an empty tuple";
            return false;
        }
        String[] array = new String[] {"1", "2", "3"};
        tupleGetValue = interp.getValue("('1','2','3')", PyObject.class);
        tupleBuiltin = builtins.tuple(array);
        if (!tupleBuiltin.equals(tupleGetValue)){
            failure = "tuple builtin does not return a tuple from array";
            return false;
        }
        tupleBuiltin = builtins.tuple(Arrays.asList(array));
        if (!tupleBuiltin.equals(tupleGetValue)){
            failure = "tuple builtin does not return a tuple from List";
            return false;
        }
        return true;
    }

    private boolean testType() {
        if (!objectType.equals(builtins.type(object))){
            failure = "type builtin returns wrong type for an object";
            return false;
        }
        return true;
    }

    public void runTest() {
        try (Interpreter interp = new SharedInterpreter()) {
            this.interp = interp;
            builtins = PyBuiltins.get(interp);
            objectType = interp.getValue("object", PyCallable.class);
            object = interp.getValue("object()", PyObject.class);
            subtypeOne = interp.getValue("type('subtypeone', (object,), {})", PyCallable.class);
            subtypeTwo = interp.getValue("type('subtypetwo', (object,), {})", PyCallable.class);
            subobject = ((PyCallable) subtypeOne).callAs(PyObject.class);
            globals = interp.getValue("{}", PyObject.class);
            locals = interp.getValue("{}", PyObject.class);
            if (!testCallable()) {
                return;
            }
            if (!testDelAttr()) {
                return;
            }
            if (!testDict()) {
                return;
            }
            if (!testDir()) {
                return;
            }
            if (!testEval()) {
                return;
            }
            if (!testExec()) {
                return;
            }
            if (!testFrozenSet()) {
                return;
            }
            if (!testGetAttr()) {
                return;
            }
            if (!testHasAttr()) {
                return;
            }
            if (!testId()) {
                return;
            }
            if (!testIsInstance()) {
                return;
            }
            if (!testIsSubClass()) {
                return;
            }
            if (!testList()) {
                return;
            }
            if (!testObject()) {
                return;
            }
            if (!testSet()) {
                return;
            }
            if (!testSetAttr()) {
                return;
            }
            if (!testTuple()) {
                return;
            }
            if (!testType()) {
                return;
            }
        } catch (JepException e) {
            failure = e.getMessage();
        }
    }

    public static String test() throws InterruptedException{
        TestPyBuiltins test = new TestPyBuiltins();
        Thread t = new Thread(test::runTest);
        t.start();
        t.join();
        return test.failure;
    }
}
