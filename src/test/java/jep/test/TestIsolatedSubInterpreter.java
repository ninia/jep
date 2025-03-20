package jep.test;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import jep.Interpreter;
import jep.JepConfig;
import jep.SubInterpreter;
import jep.SubInterpreterOptions;

/**
 * This test case runs some simple Python in many threads using isolated
 * sub-interpreters to ensure that there aren't any concurrency issues that
 * cause it to crash.
 */
public class TestIsolatedSubInterpreter {

    private static final JepConfig JEP_CONFIG = new JepConfig()
            .setSubInterpreterOptions(SubInterpreterOptions.isolated());

    private static final ThreadLocal<Interpreter> subInterpreter = ThreadLocal
            .withInitial(() -> new SubInterpreter(JEP_CONFIG));

    public static void main(String... args) {
        ExecutorService executor = Executors.newFixedThreadPool(12);
        try {
            List<CompletableFuture<?>> futures = new ArrayList<>();
            for (int i = 0; i < 1000; i++) {
                futures.add(CompletableFuture.supplyAsync(() -> {
                    return subInterpreter.get().getValue("1 + 2");
                }, executor));
            }
            CompletableFuture.allOf(futures.toArray(new CompletableFuture[0]))
                    .join();
        } finally {
            executor.shutdown();
        }

    }

}
