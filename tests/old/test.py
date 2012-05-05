#asd
import sys

print "Hello world using Jep."

#import os
#raw_input('gdb --pid=%s' % os.getpid())

from jep import *
__builtins__.__import__ = jep.jimport

from java.lang import *
from java.util import HashMap
from java.io import FileInputStream

def console(quit=True):
    import console
    console.prompt(testo.getJep())
    if(quit):
        raise RuntimeError, 'blah'

def testMethod(str):
    print 'testMethod:      ', str
    return Integer(12)

#for i in (range(0, 10)):
if(__name__ == '__main__'):
    
    # turn on stack traces
    printStack(True)
    
    i = 0

    print 'sys.path:        ', sys.path
    print 'dir(jep):        ', dir(jep)
    
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
    print 'test byte:       ', testy
    print 'test char:       ', testc
    print 'test null obj:   ', testn
    print 'test class obj:  ', testz().toString()

    print 'test b array:    ', testab[1]
    print 'test d array:    ', testad[1]

    print 'test b array:    ', amod.testab[1]
    print 'test d array:    ', amod.testad[1]

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
    print 'test getByte:    ', testo.getByte()
    print 'test getChar:    ', testo.getChar()
    print 'test getClass:   ', testo.getClass()
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
    
    print 'test send stuff  ', testo.sendMeSomeStuff("an arraylist object",
                                                     jarrayList)
    print 'test send stuff  ', testo.sendMeSomeStuff(None, jarrayList)

    print """
    ##################################################
    # test constructors
    ##################################################
    """
    print 'new Integer:     ', Integer(12)
    h = HashMap()
    h.put('test', 'w00t')
    print 'new HashMap:     ', h
    
    print 'new Boolean:     ', Boolean(False)
    print repr(Boolean)
    
    s = String('tested new string')
    print 'make jstring:    ', s
    #s2 = String(s.getBytes())
    #print 'another string:  ', s2

    l = Long(123123123123123)
    print 'make long:       ', l

    d = Double(132123.123123)
    print 'make double:     ', d

    f = Float(123.123)
    print 'make float:      ', f

    c = Character('j')
    print 'make char:       ', c

    b = Byte(123)
    print 'make byte:       ', b
    
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
    print 'byteField:       ', testo.byteField
    print 'charField:       ', testo.charField
    print 'classField:      ', testo.classField

    testo.stringField = 'a new string for loop: %i' % (i)
    testo.intField = i
    testo.shortField = i
    testo.booleanField = False
    testo.longField = testo.longField * -1
    testo.doubleField = 1223123.123
    testo.floatField = 12312.122
    testo.charField = 'j'
    testo.byteField = 2
    testo.classField = Integer
    
    print 'stringField:     ', testo.stringField
    print 'intField:        ', testo.intField
    print 'shortField:      ', testo.shortField
    print 'booleanField:    ', testo.booleanField
    print 'longField:       ', testo.longField
    print 'doubleField:     ', testo.doubleField
    print 'floatField:      ', testo.floatField
    print 'byteField:       ', testo.byteField
    print 'charField:       ', testo.charField
    print 'classField:      ', testo.classField
    
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
    print 'staticChar:      ', testo.staticChar
    print 'staticByte:      ', testo.staticByte
    print 'staticClass:     ', testo.staticClass

    print ""
    testo.staticString = 'a new string for loop: %i' % (i)
    testo.staticInt = i
    testo.staticShort = i
    testo.staticBoolean = True
    testo.staticLong = testo.staticLong * -1
    testo.staticDouble = 123.12
    testo.staticFloat = 123.123
    testo.staticByte = testo.staticByte * 10
    testo.staticChar = 'z'
    testo.staticClass = Integer
    
    Test = findClass('jep.Test')
    print 'staticShort:     ', Test.staticShort
    print 'staticString:    ', Test.staticString
    print 'staticInt:       ', Test.staticInt
    print 'staticBoolean:   ', Test.staticBoolean
    print 'staticLong:      ', Test.staticLong
    print 'staticDouble:    ', Test.staticDouble
    print 'staticFloat:     ', Test.staticFloat
    print 'staticChar:      ', Test.staticChar
    print 'staticByte:      ', Test.staticByte
    print 'staticClass:     ', Test.staticClass
    
    Test.staticString = 'a new string for loop: %i' % (i)
    Test.staticInt = i
    Test.staticShort = i
    Test.staticBoolean = True
    Test.staticLong = Test.staticLong * -1
    Test.staticDouble = 213239.990921221221
    Test.staticFloat = 3.2121231
    testo.staticByte = testo.staticByte * 10
    testo.staticChar = 'a'
    
    print 'staticString:    ', Test.staticString
    print 'staticInt:       ', Test.staticInt
    print 'staticShort:     ', Test.staticShort
    print 'staticBoolean:   ', Test.staticBoolean
    print 'staticLong:      ', Test.staticLong
    print 'staticDouble:    ', Test.staticDouble
    print 'staticFloat:     ', Test.staticFloat
    print 'staticChar:      ', Test.staticChar
    print 'staticByte:      ', Test.staticByte

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
    print 'staticByte:      ', testo.getStaticByte()
    print 'staticChar:      ', testo.getStaticChar()
    print 'staticClass:     ', testo.getStaticClass()

    print """
    ##################################################
    # test exceptions
    ##################################################
    """
    # exceptions are *slow*... you wouldn't want to do much of this
    # in a real environment.
    try:
        print '12:              ', Integer(12).intValue()
        Integer.parseInt('asdf')
    except(jep.NumberFormatException):
        print 'parseInt:         caught NumberFormatException'

    try:
        fin = FileInputStream('asdf')
        fin.close()
    except(jep.FileNotFoundException):
        print 'inputStream:      caught FileNotFoundException'

    try:
        print 'Integer =        ', Integer.toString()
    except:
        print 'fixed'
    
    System.out.println("regression test.")

    try:
        Integer.byteValue()
    except:
        print 'regression:       no crash yet'
    #raise TypeError, 'test'

    print """
    ##################################################
    # array handling
    ##################################################
    """

    # just for fun
    fin = FileInputStream("configure")
    ar = jarray(20, JBYTE_ID)
    count = fin.read(ar)

    # strip any other lines, just want first
    if(10 in ar):
        count = ar.index(10)
        ar = ar[0:count]
    print 'configure starts ', String(ar, 0, count)
    fin.close()
    
    # array handling
    ar = testo.getStringArray()
    print 'string[] len:    ', len(ar)
    print '[0], [1], [2]:   ', ar[0], ar[1], ar[2]
    ar[0] = "new"
    ar[1] = None
    print '[0], [1]:        ', ar[0], ar[1]

    print 'sending:         '
    testo.sendObjectArray(ar)

    ar = testo.getStringStringArray()
    print 'string[][]:      ', len(ar)
    print '[0][1]:          ', ar[0][1]

    ar = testo.getObjectArray()
    print 'object array:    ', len(ar)
    print '[0], [1]:        ', ar[0], ar[1]
    ar[1] = None
    print '[0], [1]:        ', ar[0], ar[1]
    
    ar = testo.getIntArray()
    print 'int array[0]:    ', ar[0]
    ar[0] -= 100
    print 'sending:         ', repr(ar)
    testo.sendIntArray(ar)

    ar = testo.getBooleanArray()
    print 'bool array[1]:   ', ar[1]

    ar = testo.getShortArray()
    print 'short [1]:       ', ar[1]

    ar = testo.getFloatArray()
    print 'float[0]:        ', ar[0]

    print 'Float max: ', Float.MAX_VALUE
    testb = True

    print 'python loop %i finished.' % (i)
