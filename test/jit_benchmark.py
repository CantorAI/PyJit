import os,time

pid = os.getpid()

import pyjit

@pyjit.func(lang="cpp")
def cpp_func(cnt:int):
    """
    unsigned long long sum =0;
    for(int k=0;k<cnt;k++)
    {
        for(int i=0;i<10000;i++)
        {
            sum +=i*i;
        }
    }
    char buf[50];
    sprintf (buf, "%llu",sum);
    return std::string(buf);
    """
def py_func(cnt):
    sum =0
    for k in range(cnt):
        for i in range(10000):
            sum += i*i
    return str(sum)

if __name__ == '__main__':
    print("call cpp_func",cpp_func(1),"\n")
    import cProfile
    print("\nprofile for cpp_func----\n")
    cProfile.run('print("cpp_func=",cpp_func(100000))')
    print("\nprofile for py_func----\n")
    cProfile.run('print("py_func=",py_func(1000))')
    
    os.getcwd()