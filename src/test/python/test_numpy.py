import unittest
import sys
import jep


@unittest.skipIf(not jep.JEP_NUMPY_ENABLED, 'Jep library built without numpy support')
class TestNumpy(unittest.TestCase):

    def setUp(self, test=None):
        TestClass = jep.findClass('jep.test.numpy.TestNumpy')
        self.test = TestClass()
        self.printout = False

    def testSetGet(self):
        """
        Tests using Jep.set(String, Object) for java NDArrays
        of the different primitive types.  Then uses
        Jep.get(String) to get a Java NDArray back and
        checks for equality/symmetry.

        Also checks that in Java, new NDArray(args) does not
        allow bad args through.
        """
        from jep_pipe import build_java_process_cmd
        from jep_pipe import jep_pipe

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

    def createNdarrayFromBuffer(self, buffer):
        DirectNDArray = jep.findClass("jep.DirectNDArray")
        from java.util import ArrayList
        # Start off with a pyjobject which is a DirectNDArray
        dndarray = DirectNDArray(buffer)
        from java.util import ArrayList
        a = ArrayList()
        a.add(dndarray)
        # Getting the same object from a java method triggers the automatic
        # conversion to an ndarray.
        return a.get(0)

    def testDirectArgReturn(self):
        """
        Tests making a python ndarray from a java DirectNDArray.
        It should be possible to pass to java and back and still
        reference the same memory. It should also be possible to
        pass it to java methods that expect an NDArray or a primitive
        array in which case the memory is copied.
        """
        from java.nio import ByteBuffer, ByteOrder

        buffer = ByteBuffer.allocateDirect(48)
        ndarray = self.createNdarrayFromBuffer(buffer.asIntBuffer())
        ndarray2 = self.test.testDirectArgAndReturn(ndarray)
        ndarray[0] = 1
        self.assertEquals(1, ndarray2[0])
        ndarray2[0] = 2
        self.assertEquals(2, ndarray[0])

        ndarray2 = self.test.testArgAndReturn(ndarray)
        self.assertEquals(ndarray[0] + 5, ndarray2[0])

        self.assertTrue(self.test.callIntMethod(ndarray))

    def assertIntDirect(self, buffer): 
        """
        Tests that changes to a buffer are reflected in an ndarray created from
        the buffer. This works with integer types(byte, short, int long)
        """
        ndarray = self.createNdarrayFromBuffer(buffer)
        ndarray[0] = 1
        self.assertEquals(1, buffer.get(0))
        buffer.put(0,2)
        self.assertEquals(2, ndarray[0])

    def assertFloatDirect(self, buffer): 
        """
        Tests that changes to a buffer are reflected in an ndarray created from
        the buffer. This works with float types(float, double)
        """
        ndarray = self.createNdarrayFromBuffer(buffer)
        ndarray[0] = 1.5
        self.assertEquals(1.5, buffer.get(0))
        buffer.put(0,2.5)
        self.assertEquals(2.5, ndarray[0])

    def testDirect(self):
        from java.nio import ByteBuffer

        buffer = ByteBuffer.allocateDirect(48)
        self.assertIntDirect(buffer)
        self.assertIntDirect(buffer.asShortBuffer())
        self.assertIntDirect(buffer.asIntBuffer())
        self.assertIntDirect(buffer.asLongBuffer())
        self.assertFloatDirect(buffer.asFloatBuffer())
        self.assertFloatDirect(buffer.asDoubleBuffer())
        with self.assertRaises(ValueError):
            self.createNdarrayFromBuffer(buffer.asCharBuffer())

    def testDirectNative(self):
        from java.nio import ByteBuffer, ByteOrder

        buffer = ByteBuffer.allocateDirect(48).order(ByteOrder.nativeOrder())
        self.assertIntDirect(buffer)
        self.assertIntDirect(buffer.asShortBuffer())
        self.assertIntDirect(buffer.asIntBuffer())
        self.assertIntDirect(buffer.asLongBuffer())
        self.assertFloatDirect(buffer.asFloatBuffer())
        self.assertFloatDirect(buffer.asDoubleBuffer())

    def testPassingDirect(self):
        """
        Test that when a numpy ndarray created from a java direct buffer is
        passed to java that it still uses the exact same direct buffer. The
        simplest way to test that is to copy it back to python and ensure
        modifications are visible in both numpy ndarrays
        """
        from java.nio import ByteBuffer

        buffer = ByteBuffer.allocateDirect(48)
        ndarray = self.createNdarrayFromBuffer(buffer)

        from java.util import ArrayList
        a = ArrayList()
        a.add(ndarray)
        ndarray2 = a.get(0)
        ndarray[0] = 1
        self.assertEquals(1, ndarray2[0])
        ndarray2[0] = 2
        self.assertEquals(2, ndarray[0])

    def testScalarBoxing(self):
        import numpy
        getClass = self.test.getDefaultConversionClass
        TestClass = jep.findClass('jep.test.Test')
        test = TestClass()
        getConvert = test.testObjectPassThrough
        self.assertEquals('java.lang.Float', getClass(numpy.float32(1.5)).java_name)
        self.assertEquals(1.5, getConvert(numpy.float32(1.5)))
        self.assertEquals('java.lang.Double', getClass(numpy.float64(1.5)).java_name)
        self.assertEquals(1.5, getConvert(numpy.float64(1.5)))
        self.assertEquals('java.lang.Long', getClass(numpy.int64(7)).java_name)
        self.assertEquals(7, getConvert(numpy.int64(7)))
        self.assertEquals('java.lang.Integer', getClass(numpy.int32(7)).java_name)
        self.assertEquals(7, getConvert(numpy.int32(7)))
        self.assertEquals('java.lang.Short', getClass(numpy.int16(7)).java_name)
        self.assertEquals(7, getConvert(numpy.int16(7)))
        self.assertEquals('java.lang.Byte', getClass(numpy.int8(7)).java_name)
        self.assertEquals(7, getConvert(numpy.int8(7)))
          
