# c# test 

import os,time

pid = os.getpid()
import pyjit

#pyjit.EnableDebug()

@pyjit.func(lang="cs",Debug=True)
def embed_func(x:int,y:int):
    """
    String s = String.Format("x:{0},y:{1}.",x,y);
    Console.WriteLine(s);
    """

@pyjit.func(lang="cs",Debug=True,
    impl=["extern_test.cs"])
def extern_impl_func(m:int,n:int)->str:
    pass

np_a = embed_func(2,3)
print(np_a)
s = extern_impl_func(100,1000);
print(s)

os.getcwd()