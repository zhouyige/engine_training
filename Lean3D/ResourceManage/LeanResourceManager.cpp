#include "LeanResourceManager.h"
#include <map>

namespace Lean3D
{
	ResourceManager::ResourceManager()
	{
		_resources.reserve(100);
	}

	ResourceManager::~ResourceManager()
	{
		//释放资源对象
		resClear();
		//释放资源注册类型
		std::map< int, ResourceRegEntry >::const_iterator itr = _registry.begin();
		while (itr != _registry.end())
		{
			if (itr->second.releaseFunc != 0x0)
				(*itr->second.releaseFunc)();

			++itr;
		}
	}

	void ResourceManager::registerResType(int resType, const std::string &typeString, ResTypeInitializationFunc inf, ResTypeReleaseFunc rf, ResTypeFactoryFunc ff)
	{
		ResourceRegEntry entry;
		entry.typeString = typeString;
		entry.initializationFunc = inf;
		entry.releaseFunc = rf;
		entry.factoryFunc = ff;
		_registry[resType] = entry;
		//调用每个类型初始化函数
		if (inf != 0) (*inf)();
	}

	void ResourceManager::resClear()
	{

		for (uint32 i = 0; i < _resources.size(); ++i)
		{
			if (_resources[i] != 0x0) _resources[i]->release();
		}
		for (uint32 i = 0; i < _resources.size(); ++i)
		{
			if (_resources[i] != 0x0)
			{
				delete _resources[i]; _resources[i] = 0x0;
			}
		}
	}

	Lean3D::ResHandle ResourceManager::addResource(int type, std::string &name, int flags, bool userCall)
	{
		if (name == "")
		{
			LEAN_DEGUG_LOG("error 未定义的名字的资源，资源类型：%d!", type);
			return 0;
		}

		// 若资源已经在列表中，直接返回对应索引句柄
		for (uint32 i = 0; i < _resources.size(); ++i)
		{
			if (_resources[i] != 0x0 && _resources[i]->getName() == name)
			{
				if (_resources[i]->getType() == type)
				{
					if (userCall) _resources[i]->addUserRefCount();
					return i + 1;
				}
			}
		}

		// 创建资源对象
		Resource *resource = 0x0;
		std::map< int, ResourceRegEntry >::iterator itr = _registry.find(type);
		if (itr != _registry.end()) resource = (*itr->second.factoryFunc)(name, flags);
		if (resource == 0x0) return 0;

		if (userCall) resource->addUserRefCount();

		return addResource(*resource);

	}

	Lean3D::ResHandle ResourceManager::addResource(Resource &res)
	{
		// 向资源列表中空余的位置插入资源对象
		for (uint32 i = 0; i < _resources.size(); ++i)
		{
			if (_resources[i] == 0x0)
			{
				res.setHandle(i + 1);
				_resources[i] = &res;
				return i + 1;
			}
		}

		// 如果之前申请的空余列表被占满，向后push
		res.setHandle((ResHandle)_resources.size() + 1);
		_resources.push_back(&res);
		return res.getHandle();
	}

	int ResourceManager::removeResource(Resource &resource, bool userCall)
	{
		if (userCall && resource.getUserRefCount() > 0) resource.subUserRefCount();

		return (signed)resource.getUserRefCount();
	}

	Lean3D::ResHandle ResourceManager::addNonExistingResource(Resource &resource, bool userCall)
	{
		if (resource.getName() == "") return 0;

		for (uint32 i = 0; i < _resources.size(); ++i)
		{
			if (_resources[i] != 0x0 && _resources[i]->getName() == resource.getName())
				return 0;
		}

		if (userCall) resource.addUserRefCount();
		return addResource(resource);
	}

	Lean3D::Resource * ResourceManager::findResource(int type, const std::string &name)
	{
		for (size_t i = 0, s = _resources.size(); i < s; ++i)
		{
			if (_resources[i] != 0x0 && _resources[i]->getType() == type && _resources[i]->getName() == name)
			{
				return _resources[i];
			}
		}

		return 0x0;
	}

	//找到未被读取的资源
	Lean3D::ResHandle ResourceManager::queryUnloadedResource(int index)
	{
		int j = 0;

		for (uint32 i = 0; i < _resources.size(); ++i)
		{
			if (_resources[i] != 0x0 && !_resources[i]->isLoaded() && !_resources[i]->isNoQuery())
			{
				if (j == index) return _resources[i]->getHandle();
				else ++j;
			}
		}

		return 0;
	}

}