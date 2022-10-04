#include "CarObject.h"

CarObject::CarObject(int x, double y, std::string z)
{
}

int CarObject::Move(float Speed, int targetDistance)
{
	return 0;
}

bool CarObject::repair(std::string dealer_name, std::string when)
{
	printf("call from CarObject::repair,%s,%s\n", dealer_name.c_str(), when.c_str());
	return true;
}
int CarObject::test_func(int x)
{
	return x*x;
}
CarObject* CarObject::test_func2(int x, CarObject* obj)
{
	return obj;
}