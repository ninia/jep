import unittest

import jep
Test = jep.findClass('jep.test.numpy.TestNumpy')


class TestNumpy(unittest.TestCase):
    def setUp(self, test=None):
        self.test = Test()
        self.printout = False
    
    def testSetGet(self):
        """
        Tests using Jep.set(String, Object) for java NDArrays
        of the different primitive types.  Then uses
        Jep.get(String) to get a Java NDArray back and
        checks for equality/symmetry.
        """
        if jep.JEP_NUMPY_ENABLED:
            self.test.testSetAndGet()
                
    def testArgReturn(self):
        """
        Tests making a python ndarray, sending it to Java,
        Java making a new one and adding 5 to the values,
        and then Java returning the new ndarray to python.
        """
        if jep.JEP_NUMPY_ENABLED:
            import numpy
            x = numpy.array(range(10), numpy.int32)
            y = self.test.testArgAndReturn(x)
            self.assertEqual(x.shape, y.shape)
            self.assertEqual(x.dtype, y.dtype)
            for i in range(10):
                self.assertEqual(x[i] + 5, y[i])
            
            
    def testMultiDimensional(self):
        """
        Tests sending a 3-dimensional ndarray to Java,
        Java making a new one and adding 5 to the values,
        and then Java returning the new ndarray to python.
        """
        if jep.JEP_NUMPY_ENABLED:
            import numpy
            o = range(24)
            x = numpy.array(o, numpy.int32)
            y = x.reshape((3,4,2))
            z = self.test.testArgAndReturn(y)
            self.assertEqual(y.shape, z.shape)
            self.assertEqual(y.dtype, z.dtype)
            y += 5
            self.assertEqual(repr(y), repr(z))
            if self.printout:
                print("")
                print(repr(y))
                print(repr(z))

    
    def testArrayParams(self):
        """
        Tests passing an ndarray to a Java method that is expecting
        a primitive array.  This is useful for 1 dimensional arrays,
        or if you already know the dimensions in Java, or you don't
        want to depend on the java class NDArray.
        """
        if jep.JEP_NUMPY_ENABLED:
            import numpy
            
            za = numpy.zeros((15, 5), numpy.bool)
            za.fill(True)
            self.assertTrue(self.test.callBooleanMethod(za))
            
            ba = numpy.zeros((15, 5), numpy.byte)
            ba.fill(1)
            self.assertTrue(self.test.callByteMethod(ba))
            
            sa = numpy.zeros((15, 5), numpy.short)
            sa.fill(2)
            self.assertTrue(self.test.callShortMethod(sa))
            
            ia = numpy.zeros((15, 5), numpy.int32)
            ia.fill(True)
            self.assertTrue(self.test.callIntMethod(ia))
            
            la = numpy.zeros((15, 5), numpy.int64)
            la.fill(True)
            self.assertTrue(self.test.callLongMethod(la))
            
            fa = numpy.zeros((15, 5), numpy.float32)
            fa.fill(True)
            self.assertTrue(self.test.callFloatMethod(fa))
            
            da = numpy.zeros((15, 5), numpy.float64)
            da.fill(True)
            self.assertTrue(self.test.callDoubleMethod(da))

