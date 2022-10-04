# PyJit
Embed c++/c#/Java code into Python directly and compile just in time 

Simple Example:

import pyjit
@pyjit.func(lang="cpp",Debug=True)
def numpy_test(x:int,y:int):
	"""
	auto np =JitObject()["numpy"];
	    
	auto a = np["arange"](x*y)["reshape"](x,y);
	auto b = np["random.rand"](x,y);
	auto c = np["multiply"](a,b);
	JitObject ary(std::map<std::string,JitObject>{ { "a", a }, { "b", b },{ "c", c } });
	return ary;
	"""
np_a = numpy_test(2,3)
print(np_a)
