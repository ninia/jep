package jep.test;

import java.util.Arrays;
import java.util.Iterator;

/**
 * A class implementing both iterator and iterable to test that python wrapped
 * java objects can represent both interfaces at once.
 *
 * @author Ben Steffensmeier
 * @since 4.0
 */
public class TestIteratorable implements Iterator<String>, Iterable<String>{

    private final String[] arr = {"One", "Two", "Three"};

    private int index = 0;

    @Override
    public boolean hasNext() {
        return index < arr.length;
    }

    @Override
    public String next() {
        String next = arr[index];
	index += 1;
	return next;
    }

    @Override
    public Iterator<String> iterator() {
        return new TestIteratorable();
    }

}
