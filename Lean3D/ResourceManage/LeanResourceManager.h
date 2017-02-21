#pragma once

#include "LeanUtil.h"
#include "LeanResource.h"
#include <map>

namespace Lean3D
{
	typedef void(*ResTypeInitializationFunc)();
	typedef void(*ResTypeReleaseFunc)();
	typedef Resource *(*ResTypeFactoryFunc)(const std::string &name, int flags);

	struct ResourceRegEntry
	{
		std::string                typeString;			//资源类型名
		ResTypeInitializationFunc  initializationFunc;  //初始化资源对象
		ResTypeReleaseFunc         releaseFunc;			// 卸载资源对象
		ResTypeFactoryFunc         factoryFunc;			// 创建资源对象
	};

	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();
		void registerResType(int resType, const std::string &typeString, ResTypeInitializationFunc inf,
			ResTypeReleaseFunc rf, ResTypeFactoryFunc ff);
		void resClear();

		Resource *resolveResHandle(ResHandle handle)
		{
			return (handle != 0 && (unsigned)(handle - 1) < _resources.size()) ? _resources[handle - 1] : 0x0;
		}

		ResHandle addResource(int type, std::string &name, int flags, bool userCall);
		int removeResource(Resource &resource, bool userCall);
		//添加在资源列表中不存在的资源
		ResHandle addNonExistingResource(Resource &resource, bool userCall);

		Resource *findResource(int type, const std::string &name);
		std::vector <Resource*> &getResourceList() { return _resources; }

		ResHandle queryUnloadedResource(int index);

	protected:
		ResHandle addResource(Resource &res);
	private:
		std::vector<Resource*> _resources;
		std::map<int, ResourceRegEntry> _registry; //支持的所有资源类型
	};


}
