#PyJit
using pyjit, you can embed c++,c# and Java code into 
python, and compile just in time.

#examples

import pyjit

@pyjit.func(lang="cpp",Debug=True)
def numpy_test(x:int,y:int):
    """
    auto np =PyJit::Object()["numpy"];
    
    auto a = np["arange"](x*y)["reshape"](x,y);
    auto b = np["random.rand"](x,y);
    auto c = np["multiply"](a,b);
    PyJit::Object ary(std::map<std::string,PyJit::Object>{ { "a", a }, { "b", b },{ "c", c } });
    return ary;
    """

np_a = numpy_test(2,3)
print(np_a)


