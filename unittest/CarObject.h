#pragma once

#include <string>
#include "Jit_Object.h"

class CarObject:
	public PyJit::MetaObject
{
public:
	CarObject()
	{

	}
	CarObject(int x, double y, std::string z);
	int Move(float Speed, int targetDistance);
	bool repair(std::string dealer_name, std::string when);
	int test_func(int x);
	CarObject* test_func2(int x, CarObject* obj);

	void setcolor(std::string color)
	{
		m_color = color;
	}
	std::string getcolor()
	{
		return m_color;
	}
	void setbody(int v)
	{
		m_body = v;
	}
	int getbody()
	{
		return m_body;
	}
	void setother(PyJit::Object v)
	{
		m_other = v;
	}
	PyJit::Object getother()
	{
		return m_other;
	}
public:
	std::string m_make;
	int m_body;
protected:
	std::string m_color;
	PyJit::Object m_other;
};
