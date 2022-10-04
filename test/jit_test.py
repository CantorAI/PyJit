
import os,time

pid = os.getpid()
import pyjit


#pyjit.EnableDebug()

import jit_array_test

@pyjit.func(lang="cpp",Debug=True)
def numpy_test(x:int,y:int):
    """
    auto np =PyJit::Object()["numpy"];
    int yy =1;
    auto a = np["arange"](x*y)["reshape"](x,y);
    auto b = np["random.rand"](x,y);
    auto c = np["multiply"](a,b);
    PyJit::Object k1(std::map<std::string,PyJit::Object>{ { "a", a }, { "b", b },{ "c", c } });
    return k1;
    """

dict1 = {'X': 2, 'Y': 3, 'Z': 4}

xx = dict1["X"]
dict1["X"] =200
xx =20
def pyfunc_to_cpp(var1):
    print("call from pyfunc_to_cpp,var1=",var1,"\n")
    return 10

@pyjit.func(lang="cpp",Debug=True)
def cpp_func(x,y:float,z,d):
    """
    auto datetime = PyJit::Object()["datetime.datetime"];
    for( int i=0;i<10;i++)
    {
        //auto now = datetime["now"]();
        std::string current_time = (std::string)datetime["now"]()["strftime"]("%Y-%m-%d %H:%M:%S.%f");
        printf("Current Time:%s\n",current_time.c_str());
        PyJit::Object tm = (PyJit::Object)PyJit::Object::Import("time");
        //tm["sleep"](1);
    }
    int pid = (int)PyJit::Object()["os.getpid"]();
    d["Y"] = 300;
    z(x);

    int sum =(int)d["X"];
    for(int i=0;i<10;i++)
    {
        sum+=int(i*y);
    }
    printf("sum=%d\n",sum);

    PyJit::Object k1(std::map<std::string, int>{ { "Riti", 2 }, { "Jack", 4 } });
    
    PyJit::Object k3(std::map<int, int>{});
    for(int i=0;i<10;i++)
    {
        k3[i] =i*i;
    }
    return k3;
    """

@pyjit.func(lang="cpp",Debug=True)
def cpp_inside_python_test_func(var1:int,y:float,z:str):
    """
    int sum =(int)y;
    for(int i=0;i<5;i++)
    {
        sum+=int(i*y);
        sum =sum*sum;
        printf("i=%d,sum=%d\n",i,sum);
    }
    printf("calc->sum=%d,z=%s\n",sum,z.c_str());
    PyJit::Object k2(std::map<std::string, int>{});
    k2["k1"] =100;
    k2["k2"] =200;
    k2["k3"] =-100;
    k2["k4"] =-400;
    return k2;
    """

startTS = time.time()
num =1

np_a = numpy_test(2,3)
print(np_a)

jit_array_test.cpp_extern_impl_func(1,2)
jit_array_test.cpp_extern_impl_func2(3,2)

np_a = numpy_test(2,3)
print(np_a)

for i in range(num):
    try:
        xc = cpp_func(1,3.14,pyfunc_to_cpp,dict1)
        print(xc)
    except Exception as inst:
        print(inst)
    #print("call cpp_func,i=\t",i)
    #rv = cpp_inside_python_test_func(100,1.23,"dddd")
    #print("rv=",rv,"\n")
elapsed = time.time() - startTS
elapsed =elapsed/num
print("call num=",num,"avg call time(ms):",elapsed*1000)
os.getcwd()