package jep.test.closeable;

import java.io.IOException;

/**
 * @author Nate Jensen
 */
public class TestAutoCloseable implements AutoCloseable {

    protected boolean closed = false;

    public void close() throws IOException {
        closed = true;
    }

    public boolean isClosed() {
        return closed;
    }

    public void write(String str) throws IOException {
        if (!closed) {
            // pretend it's writing
        } else {
            throw new IOException("Stream is closed");
        }
    }

}
