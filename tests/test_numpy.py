import unittest
import sys
import jep


@unittest.skipIf(not jep.JEP_NUMPY_ENABLED, 'Jep library built without numpy support')
class TestNumpy(unittest.TestCase):

    def setUp(self, test=None):
        TestClass = jep.findClass('jep.test.numpy.TestNumpy')
        self.test = TestClass()
        self.printout = False

    @unittest.skipIf(sys.platform.startswith("win"), "subprocess complications on Windows")
    def testSetGet(self):
        """
        Tests using Jep.set(String, Object) for java NDArrays
        of the different primitive types.  Then uses
        Jep.get(String) to get a Java NDArray back and
        checks for equality/symmetry.

        Also checks that in Java, new NDArray(args) does not
        allow bad args through.
        """
        from tests.jep_pipe import build_java_process_cmd
        from tests.jep_pipe import jep_pipe

        jep_pipe(build_java_process_cmd('jep.test.numpy.TestNumpy'))

    def testArgReturn(self):
        """
        Tests making a python ndarray, sending it to Java,
        Java making a new one and adding 5 to the values,
        and then Java returning the new ndarray to python.
        """
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
        import numpy
        o = range(24)
        x = numpy.array(o, numpy.int32)
        y = x.reshape((3, 4, 2))
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

    def testIncompatibleConversion(self):
        import numpy
        fa = numpy.zeros((15, 5), numpy.float32)
        with self.assertRaises(TypeError):
            self.test.callDoubleMethod(fa)

    def testCharArrayCreation(self):
        NDArray = jep.findClass("jep.NDArray")
        with self.assertRaises(ValueError):
            NDArray(jep.jarray(4, jep.JCHAR_ID))
