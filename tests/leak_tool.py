# function for testing for memory leaks in unittests by repeatedly executing a callable and ensuring the 
# process size does not grow out of control. This function is not compatible with all platforms and will
# call testCase.skipTest() on platforms that aren't compatible.

import itertools

# This number is enough to prove small leaks on linux. Since the test measures process growth a system with a
# higher initial memory usage will hide a leak. Unfortunatly the units of ru.maxrss are platform dependent so
# it is impossible to accuratly measure memory usage in term of bytes. To make the test more robust the number
# of iterations can be increased but it takes longer so isn't worth it unless there is a reason to suspect a
# problem.
iterations = 2000000

# The value of 2 causes the test to fail if memory use doubles.
# the value of 1.2 causes the test to fail if memory increases more than 20%
# Values between 1.2 and 2 are considered reasonable.
failure_threshold=1.5


def test_leak(testCase, callable, msg):
    try:
        import resource
    except ImportError:
       testCase.skipTest('Cannot test for memory leaks on this platform')
    start_memory = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    for _ in itertools.repeat(None,iterations):
        callable()
    end_memory = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    percent = end_memory*100/start_memory - 100
    testCase.assertLess(end_memory, start_memory*failure_threshold, msg + ' resulted in %d%% increase over %d iterations' % (percent, iterations)) 
