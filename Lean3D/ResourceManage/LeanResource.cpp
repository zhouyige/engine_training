#include "LeanResource.h"

namespace Lean3D
{
	Resource::Resource(int type, const std::string &name, int flag)
	{
		_type = type;
		_name = name;
		_handle = 0;
		_loaded = false;
		_refCount = 0;
		_userRefCount = 0;
		_flags = flag;

		if ((flag & ResourceFlags::NoQuery) == ResourceFlags::NoQuery) _noQuery = true;
		else _noQuery = false;
	}

	Resource::~Resource()
	{
	}

	void Resource::unload()
	{

	}

	Resource * Resource::clone()
	{
		LEAN_DEGUG_LOG("类型{%i}的资源克隆函数无实现！", _type);

		return 0x0;
	}

	void Resource::initDefault()
	{
		LEAN_DEGUG_LOG("Resource 基类： initDefault 被调用！", 0);
	}

	void Resource::release()
	{
		LEAN_DEGUG_LOG("Resource 基类： release 被调用！", 0);
	}

	bool Resource::load(const char *data, int size)
	{
		//一个资源只读取一次
		if (_loaded) return false;
		
		//文件读取错误
		if (data == 0x0 || size <= 0)
		{
			
			LEAN_DEGUG_LOG("Resource 基类：文件load错误！文件名 {%s}", _name.c_str());
			_noQuery = true;
			return false;
		}

		_loaded = true;

		return true;
	}

	int Resource::findElem(int elem, int param, const char *value)
	{
		for (int i = 0, count = getElemCount(elem); i < count; ++i)
		{
			if (strcmp(getElemParamStr(elem, i, param), value) == 0)
				return i;
		}

		return -1;
	}

	int Resource::getElemCount(int elem)
	{
		LEAN_DEGUG_LOG("Resource 基类： getElemCount 被调用！", 0);
		return 0;
	}

	int Resource::getElemParamI(int elem, int elemIdx, int param)
	{
		LEAN_DEGUG_LOG("Resource 基类： getElemParamI 被调用！", 0);

		return Math::MinInt32;
	}

	void Resource::setElemParamI(int elem, int elemIdx, int param, int value)
	{
		LEAN_DEGUG_LOG("Resource 基类： setElemParamI 被调用！", 0);
	}

	float Resource::getElemParamF(int elem, int elemIdx, int param, int compIdx)
	{
		LEAN_DEGUG_LOG("Resource 基类： getElemParamF 被调用！", 0);

		return Math::NaN;
	}

	void Resource::setElemParamF(int elem, int elemIdx, int param, int compIdx, float value)
	{
		LEAN_DEGUG_LOG("Resource 基类： setElemParamF 被调用！", 0);
	}

	const char * Resource::getElemParamStr(int elem, int elemIdx, int param)
	{
		LEAN_DEGUG_LOG("Resource 基类： getElemParamStr 被调用！", 0);

		return "";
	}

	void Resource::setElemParamStr(int elem, int elemIdx, int param, const char *value)
	{
		LEAN_DEGUG_LOG("Resource 基类： setElemParamStr 被调用！", 0);
	}

	void * Resource::mapStream(int elem, int elemIdx, int stream, bool read, bool write)
	{
		LEAN_DEGUG_LOG("Resource 基类： mapStream 被调用！", 0);

		return 0x0;
	}

	void Resource::unmapStream()
	{
		LEAN_DEGUG_LOG("Resource 基类： unmapStream 被调用！", 0);
	}

}