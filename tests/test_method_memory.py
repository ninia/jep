# This test case attempts to verify that pyjmethods are correctly managing their refcounts.

from __future__ import print_function
import unittest
# TODO resource may not be available under windows, I need someone with windows to test and possibly skip the memory leak testing.
import resource

from java.lang import Object

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

class TestMethodMemory(unittest.TestCase):
    #def test_repeated_call(self):
    #    obj = Object()
    #    method = obj.hashCode
    #    for i in range(0,iterations,1):
    #        method()

    def test_access_call(self):
        from java.util import ArrayList
        start_memory = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
        obj = Object()
        for i in xrange(0,iterations,1):
            b = obj.hashCode()
        end_memory = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
        if start_memory*failure_threshold < end_memory:
            percent = end_memory*100/start_memory - 100
            raise Exception('Called methods are leaking memory, process size increased %d%% over %d iterations.' % (percent, iterations)) 

    def test_access_no_call(self):
        start_memory = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
        obj = Object()
        for i in xrange(0,iterations,1):
            b = obj.hashCode
        end_memory = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
        if start_memory*failure_threshold < end_memory:
            percent = end_memory*100/start_memory - 100
            raise Exception('Uncalled methods are leaking memory, process size increased %d%% over %d iterations.' % (percent, iterations)) 
 
