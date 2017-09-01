package jep.test.closeable;

import java.io.IOException;

/**
 * Test class used to support test_autocloseable.py.
 * 
 * Created: February 2017
 * 
 * @author Nate Jensen
 * @since 3.7
 */
public class TestAutoCloseable implements AutoCloseable {

    protected boolean closed = false;

    @Override
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
