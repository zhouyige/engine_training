#pragma once
#include "tem.h"

namespace lean{
	class A
	{
	public:
		A();
		~A();

	private:
		ReferenceObjList<int> _xxx;
	};

	A::A()
	{
	}

	A::~A()
	{
	}
}