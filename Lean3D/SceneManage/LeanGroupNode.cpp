#include "LeanGroupNode.h"

namespace Lean3D
{
	GroupNode::GroupNode(const GroupNodeTpl & groupTpl) :
		SceneNode(groupTpl)
	{
	}

	SceneNodeTpl * GroupNode::parsingFunc(std::map<std::string, std::string>& attribs)
	{
		//i don't know the mean
		//std::map<std::string, std::string>::iterator it;
		GroupNodeTpl *groupTpl = new GroupNodeTpl("");

		return groupTpl;
	}

	SceneNode * GroupNode::factoryFunc(const SceneNodeTpl & nodeTpl)
	{
		if (nodeTpl.type != SceneNodeType::Group) return nullptr;

		return new GroupNode(*(GroupNodeTpl *)&nodeTpl);
	}
}