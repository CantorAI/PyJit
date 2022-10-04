import testclass
import cProfile

def test(r):
    x1 =[]
    for i in range(r):
        x = testclass.CarObject()
        x.color ="white:"+str(i)
        #print(x.color)
        x1.append(x)
    x1 = None
cProfile.run('print("test=",test(1000))')

x = testclass.CarObject()
cProfile.run('print("x.Move=",x.Move(1.0,1))')
print("end")