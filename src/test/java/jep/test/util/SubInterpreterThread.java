package jep.test.util;

import jep.Interpreter;
import jep.JepException;
import jep.SubInterpreter;

/**
 * This is just a thread that runs some Python statements in a subinterpreter.
 * It can be run from a Python unittest to verify SubInterpreter behavior
 * without writing Java code.
 */
public class SubInterpreterThread extends Thread {

    private final String[] pythonStatements;

    private JepException exception;

    public SubInterpreterThread(String... pythonStatements) {
        this.pythonStatements = pythonStatements;
    }

    @Override
    public void run() {
        try (Interpreter interpreter = new SubInterpreter()) {
            for (String statement : this.pythonStatements) {
                interpreter.exec(statement);
            }
        } catch (JepException e) {
            this.exception = e;
        }
    }

    public boolean hasException() {
        return exception != null;
    }

    public JepException getException() {
        return exception;
    }

}
