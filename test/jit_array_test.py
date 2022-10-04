import os,time
pid = os.getpid()

import pyjit

@pyjit.func(lang="cpp",Debug=True,
    impl=["extern_test.cpp","cpp_folder\\test2.cpp"])
def cpp_extern_impl_func(m:int,n:int)->str:
    pass

@pyjit.func(lang="cpp",Debug=True,
    impl=["extern_test.cpp","cpp_folder\\test2.cpp"])
def cpp_extern_impl_func2(m:int,n:int)->str:
    pass

@pyjit.func(lang="cpp",include=["math.h"])
def cpp_func(m:int,n:int):
    """
    unsigned long long dims[2] ={m,n};
    PyJit::Array<float> ary(2,dims);
    for(int i=0;i<m;i++)
    {
        for(int j=0;j<n;j++)
        {
            ary(i,j) = i*j/10.5;
            float x = ary(i,j);
            ary(i,j) = (float)sqrt(x); 
        }
    }

    return ary;
    """
def py_func(cnt):
    sum =0
    for k in range(cnt):
        for i in range(10000):
            sum += i*i
    return str(sum)

if __name__ == '__main__':
    #import cProfile
    #cProfile.run('cpp_func(200,200)')
    s = cpp_extern_impl_func(10,20)
    a = cpp_func(20,20)
    print(a)
    os.getcwd()