from jep import *
import threading

System = findClass('java.lang.System')

class Test(threading.Thread):
    pre = None
    
    def __init__(self, pre):
        threading.Thread.__init__(self)
        self.pre = str(pre)

    def run(self):
        print 'printing:', self.pre
        System.out.println(self.pre)

        print '2nd printing:', self.pre
        System2 = findClass('java.lang.System')
        System2.out.println(self.pre)


if(__name__ == '__main__'):
    for i in range(100):
        t = Test(i)
        t.start()
