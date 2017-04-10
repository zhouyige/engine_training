#pragma once
#include "LeanSceneNode.h"
#include <map>

namespace Lean3D
{
	class ModelNode;

	struct JointNodeTpl : public SceneNodeTpl
	{
		uint32  jointIndex;

		JointNodeTpl(const std::string &name, uint32 jointIndex) :
			SceneNodeTpl(SceneNodeType::Joint, name), jointIndex(jointIndex)
		{
		}
	};

	class JointNode : public SceneNode/*, public IAnimatableNode*/
	{
	public:
		static SceneNodeTpl *parsingFunc(std::map< std::string, std::string > &attribs);
		static SceneNode *factoryFunc(const SceneNodeTpl &nodeTpl);

		// IAnimatableNode
		const std::string &getANName() { return _name; }
		Matrix4 &getANRelTransRef() { return _relTrans; }
		/*IAnimatableNode *getANParent();*/

		bool canAttach(SceneNode &parent);
		int getParamI(int param);

		void onPostUpdate();
		void onAttach(SceneNode &parentNode);
		void onDetach(SceneNode &parentNode);

	protected:
		JointNode(const JointNodeTpl &jointTpl);

	protected:
		uint32     _jointIndex;

		ModelNode  *_parentModel;
		Matrix4   _relModelMat;  // Transformation relative to parent model

		friend class SceneNode;
		friend class ModelNode;
	};
}


