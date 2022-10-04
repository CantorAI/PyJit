#include <string>

extern void func_from_test2();
std::string cpp_extern_impl_func(int m, int n)
{
	func_from_test2();
	char buf[100];
	sprintf(buf,"m=%d,n=%d", m, n);
	return buf;
}

std::string cpp_extern_impl_func2(int m, int n)
{
	func_from_test2();
	char buf[100];
	sprintf(buf, "m=%d,n=%d", m, n);
	return buf;
}