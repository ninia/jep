
import sys

print "Hello world using Jep."
#print 'path: ', sys.path

import jep
from jep import *

#for i in (range(0, 10)):
if(__name__ == '__main__'):
    i = 0
    
    print """
    ##################################################
    # test parameters
    ##################################################
    """
    print 'dir test object: ', dir(testo)
    print 'test object:     ', testo
    print 'test parameter:  ', test
    print 'test int param:  ', testi
    print 'test bool param: ', testb
    print 'test long param: ', testl
    print 'double param:    ', testd
    print 'float param:     ', testf
    print 'test null obj:   ', testn

    print """
    ##################################################
    # test method calling
    ##################################################
    """
    print 'test callback:   ', testo.callback()
    print 'test toString:   ', testo.toString()
    print 'test getInt:     ', testo.getInt()
    print 'test getShort:   ', testo.getShort()
    print 'test getLong:    ', testo.getLong()
    print 'test getDouble:  ', testo.getDouble()
    print 'test getFloat:   ', testo.getFloat()
    integer = testo.getInteger()
    print 'dir getInteger:  ', dir(integer)
    l = testo.getClassLong()
    print 'dir getLong:     ', dir(l)
    print 'long value:      ', l.longValue()
    print 'integer value:   ', integer.intValue()
    print 'double value:    ', testo.getClassDouble()
    print 'float value:     ', testo.getClassFloat()
    
    jarrayList = testo.getObject()
    print 'dir(jarrayList): ', dir(jarrayList)
    print 'test getObject   ', jarrayList.get(0)
    jarrayList.add("list 1")
    jarrayList.add("list 2")
    it = jarrayList.iterator()
    print 'dir(it)          ', dir(it)
    print 'it.hasNext()     ', it.hasNext()
    while it.hasNext():
        next = it.next()
        if(next.equals('list 1')):
            print 'array parameter: ', next
    
    print 'test send stuff  ', testo.sendMeSomeStuff("an arraylist object", jarrayList)
    print 'test send stuff  ', testo.sendMeSomeStuff(None, jarrayList)

    print """
    ##################################################
    # test constructors
    ##################################################
    """
    Integer = findClass('java.lang.Integer')
    print 'new Integer:     ', Integer(12)
    h = findClass('java.util.HashMap')()
    h.put('test', 'w00t')
    print 'new HashMap:     ', h
    
    Boolean = findClass('java.lang.Boolean')
    print 'new Boolean:     ', Boolean(False)
    
    print repr(Boolean)
    
    String = findClass('java.lang.String')
    s = String('tested new string')
    print 'make jstring:    ', s

    Long = findClass('java.lang.Long')
    l = Long(123123123123123)
    print 'make long:       ', l

    Double = findClass('java.lang.Double')
    d = Double(132123.123123)
    print 'make double:     ', d

    Float = findClass('java.lang.Float')
    f = Float(123.123)
    print 'make float:      ', f
    
    # *shrugs*
    
    print """
    ##################################################
    # test fields
    ##################################################
    """
    print 'stringField:     ', testo.stringField
    print 'intField:        ', testo.intField
    print 'shortField:      ', testo.shortField
    print 'booleanField:    ', testo.booleanField
    print 'longField:       ', testo.longField
    print 'doubleField:     ', testo.doubleField
    print 'floatField:      ', testo.floatField

    testo.stringField = 'a new string for loop: %i' % (i)
    testo.intField = i
    testo.shortField = i
    testo.booleanField = False
    testo.longField = testo.longField * -1
    testo.doubleField = 1223123.123
    testo.floatField = 12312.122
    
    print 'stringField:     ', testo.stringField
    print 'intField:        ', testo.intField
    print 'shortField:      ', testo.shortField
    print 'booleanField:    ', testo.booleanField
    print 'longField:       ', testo.longField
    print 'doubleField:     ', testo.doubleField
    print 'floatField:      ', testo.floatField
    
    print """
    ##################################################
    # test static fields
    ##################################################
    """
    print 'staticString:    ', testo.staticString
    print 'staticInt:       ', testo.staticInt
    print 'staticShort:     ', testo.staticShort
    print 'staticBoolean:   ', testo.staticBoolean
    print 'staticLong:      ', testo.staticLong
    print 'staticDouble:    ', testo.staticDouble
    print 'staticFloat:     ', testo.staticFloat

    print ""
    testo.staticString = 'a new string for loop: %i' % (i)
    testo.staticInt = i
    testo.staticShort = i
    testo.staticBoolean = True
    testo.staticLong = testo.staticLong * -1
    testo.staticDouble = 123.12
    testo.staticFloat = 123.123
    
    Test = findClass('jep.Test')
    print 'staticShort:     ', Test.staticShort
    print 'staticString:    ', Test.staticString
    print 'staticInt:       ', Test.staticInt
    print 'staticBoolean:   ', Test.staticBoolean
    print 'staticLong:      ', Test.staticLong
    print 'staticDouble:    ', Test.staticDouble
    print 'staticFloat:     ', Test.staticFloat
    
    Test.staticString = 'a new string for loop: %i' % (i)
    Test.staticInt = i
    Test.staticShort = i
    Test.staticBoolean = True
    Test.staticLong = Test.staticLong * -1
    Test.staticDouble = 213239.990921221221
    Test.staticFloat = 3.2121231
    
    print 'staticString:    ', Test.staticString
    print 'staticInt:       ', Test.staticInt
    print 'staticShort:     ', Test.staticShort
    print 'staticBoolean:   ', Test.staticBoolean
    print 'staticLong:      ', Test.staticLong
    print 'staticDouble:    ', Test.staticDouble
    print 'staticFloat:     ', Test.staticFloat

    ii = Integer(12)
    print 'int min value:   ', ii.MIN_VALUE
    print 'int min value:   ', Integer.MIN_VALUE

    print 'ii type:         ', type(ii)
    print 'Integer type:    ', type(Integer)

    print """
    ##################################################
    # test static methods
    ##################################################
    """
    print 'staticString:    ', testo.getStaticString()
    print 'staticBoolean:   ', testo.getStaticBoolean()
    print 'staticInt:       ', testo.getStaticInt()
    print 'staticShort:     ', testo.getStaticShort()
    print 'staticObject:    ', testo.getStaticObject()
    print 'staticVoid:      ', testo.callStaticVoid()
    print 'staticLong:      ', testo.getStaticLong()
    print 'staticDouble:    ', testo.getStaticDouble()
    print 'staticFloat:     ', testo.getStaticFloat()

    print """
    ##################################################
    # test exceptions
    ##################################################
    """
    # exceptions are *slow*... you wouldn't want to do much of this
    # in a real environment.
    try:
        print '957106:          ', Integer(12).intValue()
        Integer.parseInt('asdf')
    except(jep.NumberFormatException):
        print 'parseInt:         caught NumberFormatException'

    try:
        FileInputStream = findClass('java.io.FileInputStream')
        fin = FileInputStream('asdf')
        fin.close()
    except(jep.FileNotFoundException):
        print 'inputStream:      caught FileNotFoundException'
    
    # broken
    try:
        print 'Integer = ', Integer.toString()
    except:
        print 'nope, still busted.'
    
    System = findClass('java/lang/System')
    System.out.println("regression test.")

    print 'python loop %i finished.' % (i)
