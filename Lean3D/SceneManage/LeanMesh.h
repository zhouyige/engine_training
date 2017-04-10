#pragma once
#include "LeanSceneNode.h"
#include <map>
#include "LeanMaterialRes.h"

namespace Lean3D
{
	class ModelNode;

	struct MeshNodeTpl : public SceneNodeTpl
	{
		ReferenceCountPtr<MaterialResource>  matRes;
		uint32								 batchStart, batchCount;
		uint32					             vertRStart, vertREnd;
		uint32								 lodLevel;

		MeshNodeTpl(const std::string &name, MaterialResource *materialRes, uint32 batchStart,
			uint32 batchCount, uint32 vertRStart, uint32 vertREnd) :
			SceneNodeTpl(SceneNodeType::Mesh, name), matRes(materialRes), batchStart(batchStart),
			batchCount(batchCount), vertRStart(vertRStart), vertREnd(vertREnd), lodLevel(0)
		{
		}
	};

	// =================================================================================================

	class MeshNode : public SceneNode/*, public IAnimatableNode*/
	{
	public:
		static SceneNodeTpl *parsingFunc(std::map< std::string, std::string > &attribs);
		static SceneNode *factoryFunc(const SceneNodeTpl &nodeTpl);

		// IAnimatableNode
		const std::string &getANName() { return _name; }
		Matrix4 &getANRelTransRef() { return _relTrans; }
		/*IAnimatableNode *getANParent();*/

		bool canAttach(SceneNode &parent);
		//int getParamI(int param);
		//void setParamI(int param, int value);
		bool checkIntersection(const Vec3 &rayOrig, const Vec3 &rayDir, Vec3 &intsPos) const;

		void onAttach(SceneNode &parentNode);
		void onDetach(SceneNode &parentNode);
		void onPostUpdate();

		//MaterialResource *getMaterialRes() { return _materialRes; }
		uint32 getBatchStart() { return _batchStart; }
		uint32 getBatchCount() { return _batchCount; }
		uint32 getVertRStart() { return _vertRStart; }
		uint32 getVertREnd() { return _vertREnd; }
		uint32 getLodLevel() { return _lodLevel; }
		ModelNode *getParentModel() { return _parentModel; }

	protected:
		MeshNode(const MeshNodeTpl &meshTpl);
		~MeshNode();

	protected:
		ReferenceCountPtr<MaterialResource>   _materialRes;
		uint32								  _batchStart, _batchCount;
		uint32								  _vertRStart, _vertREnd;
		uint32								  _lodLevel;

		ModelNode           *_parentModel;
		BoundingBox         _localBBox;

		std::vector< uint32 >  _occQueries;
		std::vector< uint32 >  _lastVisited;

		friend class SceneManager;
		friend class SceneNode;
		friend class ModelNode;
		friend class Renderer;
	};
}


