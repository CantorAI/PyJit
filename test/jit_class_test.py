import os,time
pid = os.getpid()

import pyjit



@pyjit.object(lang="cpp")
class cpp_class():
    def __init__(self,var1:int,var2:str,var3:float):
        self.m1 = var1
        self.m2 = var2
        self.m3 = var3
    def func1(self):
        """
        """
    def func2(self,var1:int)->int:
        var1 =10
        return var1


if __name__ == '__main__':
    a = cpp_class(100,"test",10.34)
    r = a.func2(10)
    print(a)
    os.getcwd()