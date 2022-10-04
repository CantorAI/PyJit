import os,time
pid = os.getpid()
#todo: unordered_map will cause exceptions  3/6/2022
import pyjit
@pyjit.func(lang="cpp",Debug=True)
def cpp_func(cnt:int):
    """
    PyJit::Object k0;
    for(int k=0;k<cnt;k++)
    {
        //auto x = std::unordered_map<int, int>{{ 2, 2*2 }};
        auto x = std::unordered_map<std::string, int>{ { std::string("Riti"), 2 } };
        //auto x = std::unordered_map<std::string, int>{ { "Riti", 2 }, { "Jack", 4 } };
        PyJit::Object k1(x);
        //char buffer[50];
        for (int i = 0; i < 10; i++)
        {
            //sprintf(buffer, "key%d", i);
            //k1[buffer] =i*i;
            //k1[i] = i * i;
        }
        k0 = k1;
    }
    return k0;
    """
@pyjit.func(lang="cpp",Debug=True)
def cpp_func2(cnt:int):
    """
    //for(int k=0;k<cnt;k++)
    //{
        std::unordered_map<std::string, int> k0{ { "Riti", 2 }, { "Jack", 4 } };
        /*char buffer[50];
        for (int i = 0; i < 10; i++)
        {
            sprintf(buffer, "key%d", i*i);
        }*/
        PyJit::Object k1(k0);
    //}
    return k1;
    """    
def py_func(cnt):
    k0 = {}
    for k in range(cnt):
        k1 = {"Riti":2,"Jack":4}
        for i in range(10000):
            k1["key"+str(i)] =i*i
        k0 =k1
    return k0

def py_func2(cnt):
    k0 = {}
    for k in range(cnt):
        k1 = {"Riti":2,"Jack":4}
        for i in range(10000):
            key = "key"+str(i)
        k0 =k1
    return cnt

if __name__ == '__main__':
    x =cpp_func(100)
    print("call cpp_func",x,"\n")
    import cProfile
    """
    print("\nprofile for py_func----\n")
    cProfile.run('print("py_func=",len(py_func(100)))')
    print("\nprofile for cpp_func----\n")
    cProfile.run('print("cpp_func=",len(cpp_func(100)))')
    """
    print("\nprofile for py_func2----\n")
    cProfile.run('print("py_func2=",py_func2(10000))')
    print("\nprofile for cpp_func2----\n")
    cProfile.run('print("cpp_func2=",cpp_func2(10000))')

    os.getcwd()