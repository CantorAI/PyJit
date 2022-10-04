import os,time

pid = os.getpid()
import pyjit

import jit_array_test as ar
from jit_array_test import cpp_func as cf
a = cf(20,20)
print("a=",len(a),"\n")
b = ar.cpp_extern_impl_func(10,20)
print("b=",b,"\n")


