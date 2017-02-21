#pragma once

#include "LeanUtil.h"
#include "LeanResource.h"

namespace Lean3D
{
	class SceneGraphResource : Resource
	{
	public:
		SceneGraphResource(const std::string &name, int flags);
		~SceneGraphResource();

		void initDefault();
		void release();
		static Resource *factoryFunc(const std::string &name, int flags)
		{
			return new SceneGraphResource(name, flags);
		}

	private:
	};
}