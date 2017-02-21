// app_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#if defined( _DEBUG )
	#include <assert.h>
	#define  LEAN_LOG(formatPos,argPos) printf(formatPos"\n",argPos);
#else
#define LEAN_LOG(formatPos,argPos)
#endif
using namespace std;

void log_test()
{
	LEAN_LOG("ssss""sdf%d，%d", 0);
}

int global = 1;

void lambda_test(int para)
{
	struct A
	{
		int a;
	};
	vector< A > avec;
	A a1; a1.a = 2;
	A	a2; a2.a = 1;
	avec.push_back(a1);
	avec.push_back(a2);
	sort(avec.begin(), avec.end(), [](A aa, A ab){ return aa.a < ab.a; });
	for (int i = 0; i < avec.size(); ++i)
	{
		printf("%d\n", avec[i].a);
	}
	printf("%d,%d",para,global);
}

void bitOP_test()
{
	unsigned char pix[] = {1,2,3,4,5,6,7,8,9,0};
	unsigned int p;
	p =  pix[1] << 8;
	printf("%d,%0x", pix[0], p);
}

void bitOP_test1()
{
	struct SS
	{
		unsigned int a = 0;
		unsigned int b = 0;
	};

	SS ss;
	ss.a |= 1 << 30;
	printf("%d, %d", ss.a, ss.b);
}

void pp_test()
{
	char *str = "123456\0";
	char *p = str;
	printf("%c", *p++);
}

unsigned char * useScrathBuf(unsigned int minSize)
{
	unsigned _scratchBufSize = 0;
	unsigned char *_scratchBuf = 0x0;
	if (_scratchBufSize < minSize)
	{
		delete[] _scratchBuf;
		_scratchBuf = new unsigned char[minSize + 15];
		_scratchBufSize = minSize;
	}
	printf("%d", (size_t)_scratchBuf);
	return _scratchBuf + (size_t)_scratchBuf % 16;//16字节对齐
}

class A {
public:
	inline virtual int fun() { return 1; }
};
class B : public A {
public:
	inline virtual int fun() { return 2; }
};
void inlineVfun_test()
{
	A *a = new B;
	cout << a->fun();
}

struct X
{
	int a;
	char b;
	int c;

	virtual void set_value(int v) { a = v; }
	int get_value() { return a; }
	void increase_value() { a++; }
};

int _tmain(int argc, _TCHAR* argv[])
{

	int x = 10;
	lambda_test(x);
	printf("\n%d,%d", x, global);
	return 0;
}

