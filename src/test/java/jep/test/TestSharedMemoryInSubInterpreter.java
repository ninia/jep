package jep.test;

import jep.Interpreter;
import jep.JepConfig;
import jep.JepException;

import java.util.Random;

public class TestSharedMemoryInSubInterpreter {
    private JepConfig config;
    private Interpreter mainIterp;
    private Object pythonObject;

    public static void main(String[] args) {
        TestSharedMemoryInSubInterpreter app = new TestSharedMemoryInSubInterpreter();
        app.init();

        // test in single thread
        int v = app.callMethodAdd(100, 50);
        System.out.println("Add (100, 50) = " + v);

        v = app.callMethodAdd(10, 10);
        System.out.println("Sub (10, 10) = " + v);

        // test with separate threads
        Random random = new Random();

        for (int i = 0; i < 3; i++) {
            Thread thread = new Thread(() -> {
                System.out.println(Thread.currentThread().getName() + ": started");
                int a = random.nextInt(100);
                int b = random.nextInt(100);

                int r = app.callMethodAdd(a, b);
                String result = a + " + " + b + " = " + r;

                try {
                    Thread.sleep(1000L);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                System.out.println(Thread.currentThread().getName() + ": result(" + result + ")");
                System.out.println(Thread.currentThread().getName() + ": finish");
            });
            thread.start();
        }
    }

    private void init() throws JepException {
        String initScript =
                "class SimpleObject:\n" +
                        "  def add(self, a, b):\n" +
                        "    return a + b\n" +
                        "  def sub(self, a, b):\n" +
                        "    return a - b\n" +
                        "myobj = SimpleObject()";

        // config need for sharing memory
        config = new JepConfig();

        Interpreter interp = config.createSubInterpreterSharedMemory();
        interp.exec(initScript);
        pythonObject = interp.getValue("myobj", Object.class);
        // store main interpreter with shared memory
        mainIterp = interp;
    }

    private int callMethodAdd(int a, int b) throws JepException {
        // this interpreter accesses the object created in the main interpreter
        try (Interpreter interp = config.createSubInterpreterSharedMemory()) {
            interp.set("obj", pythonObject);
            interp.set("a", a);
            interp.set("b", b);
            interp.exec("result = obj.add(a, b)");
            return interp.getValue("result", Integer.class);
        }
    }

}
