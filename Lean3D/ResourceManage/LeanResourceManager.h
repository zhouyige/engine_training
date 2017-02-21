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
		std::string                typeString;			//��Դ������
		ResTypeInitializationFunc  initializationFunc;  //��ʼ����Դ����
		ResTypeReleaseFunc         releaseFunc;			// ж����Դ����
		ResTypeFactoryFunc         factoryFunc;			// ������Դ����
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
		//�������Դ�б��в����ڵ���Դ
		ResHandle addNonExistingResource(Resource &resource, bool userCall);

		Resource *findResource(int type, const std::string &name);
		std::vector <Resource*> &getResourceList() { return _resources; }

		ResHandle queryUnloadedResource(int index);

	protected:
		ResHandle addResource(Resource &res);
	private:
		std::vector<Resource*> _resources;
		std::map<int, ResourceRegEntry> _registry; //֧�ֵ�������Դ����
	};


}
