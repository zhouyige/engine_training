#pragma once
#include "tem.h"
namespace lean{
	class B
	{
	public:
		B();
		~B();

	private:
		ReferenceObjList<int> _xxx;
	};

	B::B()
	{
	}

	B::~B()
	{
	}
}