package jep.test;

import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;

import jep.Interpreter;
import jep.JepConfig;
import jep.SubInterpreter;

/**
 * Created: July 2021
 *
 * @author Nate Jensen
 * @since 4.0
 */
public class TestRedirectStreams {

    public static void main(String[] args) throws Exception {
        ByteArrayOutputStream stdout = new ByteArrayOutputStream();
        ByteArrayOutputStream stderr = new ByteArrayOutputStream();

        try (Interpreter interp = new SubInterpreter(new JepConfig()
                .redirectStdout(stdout).redirectStdErr(stderr))) {
            String testString = "This string came from Python's print function.";
            interp.set("testString", testString);
            interp.exec("print(testString)");
            String stdoutOutput = new String(stdout.toByteArray(),
                    StandardCharsets.UTF_8);
            // Python print function will add a \n on the end
            testString += "\n";
            if (!testString.equals(stdoutOutput)) {
                throw new IllegalStateException(
                        "testString did not match stdoutOutput! testString="
                                + testString + " stdoutOutput=" + stdoutOutput);
            }

            String line1 = "stderrLine1";
            String line2 = "stderrLine2";
            interp.set("line1", line1);
            interp.set("line2", line2);
            interp.exec("import sys");
            interp.exec("print(line1, file=sys.stderr)");
            interp.exec("sys.stderr.write(line2)");
            String stderrOutput = new String(stderr.toByteArray(),
                    StandardCharsets.UTF_8);
            String combined = line1 + "\n" + line2;
            if (!combined.equals(stderrOutput)) {
                throw new IllegalStateException(
                        "testString did not match stderrOutput! testString="
                                + combined + " stderrOutput=" + stderrOutput);
            }
        }
    }
}
