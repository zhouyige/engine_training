#pragma once
#include "LeanSceneNode.h"
#include <map>

namespace Lean3D
{
	struct GroupNodeTpl : public SceneNodeTpl
	{
		GroupNodeTpl(const std::string &name) :
			SceneNodeTpl(SceneNodeType::Group, name)
		{
		}
	};

	class GroupNode : public SceneNode
	{
	public:
		static SceneNodeTpl *parsingFunc(std::map< std::string, std::string > &attribs);
		static SceneNode *factoryFunc(const SceneNodeTpl &nodeTpl);

		friend class Renderer;
		friend class SceneManager;

	protected:
		GroupNode(const GroupNodeTpl &groupTpl);
	};
}
