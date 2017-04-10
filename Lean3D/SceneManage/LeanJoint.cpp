#include "LeanJoint.h"
#include <map>
#include <string>
#include "LeanModel.h"

namespace Lean3D
{	
	JointNode::JointNode(const JointNodeTpl & jointTpl) :
		SceneNode(jointTpl), _jointIndex(jointTpl.jointIndex), _parentModel(nullptr)
	{

	}
	SceneNodeTpl * JointNode::parsingFunc(std::map<std::string, std::string>& attribs)
	{
		bool result = true;

		std::map< std::string, std::string >::iterator itr;
		JointNodeTpl *jointTpl = new JointNodeTpl("", 0);

		itr = attribs.find("jointIndex");
		if (itr != attribs.end()) jointTpl->jointIndex = atoi(itr->second.c_str());
		else result = false;

		if (!result)
		{
			SAFE_DELETE(jointTpl);
		}

		return jointTpl;
	}
	SceneNode * JointNode::factoryFunc(const SceneNodeTpl & nodeTpl)
	{
		if (nodeTpl.type != SceneNodeType::Joint) return nullptr;

		return new JointNode(*(JointNodeTpl *)&nodeTpl);
	}

	bool JointNode::canAttach(SceneNode & parent)
	{
		return (parent.getType() == SceneNodeType::Model) ||
			(parent.getType() == SceneNodeType::Joint);
	}

	int JointNode::getParamI(int param)
	{
		return 0;
	}

	void JointNode::onPostUpdate()
	{
		if (_parentModel->getGeometryResource() == 0x0) return;

		if (_parent->getType() != SceneNodeType::Joint)
			_relModelMat = _relTrans;
		else
			Matrix4::fastMult43(_relModelMat, ((JointNode *)_parent)->_relModelMat, _relTrans);

		if (_parentModel->jointExists(_jointIndex))
		{
			Matrix4 mat(Math::NO_INIT);
			Matrix4::fastMult43(mat, _relModelMat, _parentModel->getGeometryResource()->getInvBindMat(_jointIndex));

			_parentModel->setSkinningMat(_jointIndex, mat);
		}
	}

	void JointNode::onAttach(SceneNode & parentNode)
	{
		SceneNode *node = &parentNode;
		while (node->getType() != SceneNodeType::Model) node = node->getParent();
		_parentModel = (ModelNode *)node;

		_parentModel->markNodeListDirty();
	}

	void JointNode::onDetach(SceneNode & parentNode)
	{
		if (_parentModel != nullptr) _parentModel->markNodeListDirty();
	}
	

}

