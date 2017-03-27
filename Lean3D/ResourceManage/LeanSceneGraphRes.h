#pragma once

#include "LeanUtil.h"
#include "LeanResource.h"
#include  "SceneManage/LeanSceneManager.h"

namespace Lean3D
{
	class XMLNode;

	class SceneGraphResource : Resource
	{
	public:
		SceneGraphResource(const std::string &name, int flags);
		~SceneGraphResource();

		void initDefault() override;
		void release() override;
		bool load(const char *data, int size) override;
		static Resource *factoryFunc(const std::string &name, int flags)
		{
			return new SceneGraphResource(name, flags);
		}

		SceneNodeTpl *getRootNode() const { return _rootNode; }
	private:
		void parseBaseAttributes(XMLNode &xmlNode, SceneNodeTpl &nodeTpl);
		void parseNode(XMLNode &xmlNode, SceneNodeTpl *parentTpl);
	private:
		SceneNodeTpl	*_rootNode;
	};
}