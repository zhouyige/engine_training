#pragma once
#include "LeanUtil.h"
#include "LeanSceneNode.h"
#include "LeanGroupNode.h"
#include "LeanCamera.h"
#include "LeanSpatialGraph.h"


namespace Lean3D
{
	class SceneGraphResource;

	typedef SceneNodeTpl *(*NodeTypeParsingFunc)(std::map< std::string, std::string > &attribs);
	typedef SceneNode *(*NodeTypeFactoryFunc)(const SceneNodeTpl &tpl);

	struct NodeRegEntry
	{
		std::string          typeString;
		NodeTypeParsingFunc  parsingFunc;
		NodeTypeFactoryFunc  factoryFunc;
	};

	struct CastRayResult
	{
		SceneNode  *node;
		float      distance;
		Vec3       intersection;
	};




	class SceneManager
	{
	public:
		SceneManager();
		~SceneManager();

		void registerNodeType(SceneNodeType nodeType, const std::string &typeString, NodeTypeParsingFunc pf,
			NodeTypeFactoryFunc ff);
		NodeRegEntry *findType(SceneNodeType type);
		NodeRegEntry *findType(const std::string &typeString);

		void updateNodes();
		void updateSpatialNode(uint32 sgHandle);
		void updateQueues(const Frustum &frustum1, const Frustum *frustum2,
			RenderingOrder order, uint32 filterIgnore, bool lightQueue, bool renderableQueue);

		NodeHandle addNode(SceneNode *node, SceneNode &parent);
		NodeHandle addNodes(SceneNode &parent, SceneGraphResource &sgRes);
		void removeNode(SceneNode &node);
		bool relocateNode(SceneNode &node, SceneNode &parent);

		int findNodes(SceneNode &startNode, const std::string &name, SceneNodeType type);
		void clearFindResults() { _findResults.resize(0); }
		SceneNode *getFindResult(int index) { return (unsigned)index < _findResults.size() ? _findResults[index] : 0x0; }

		int castRay(SceneNode &node, const Vec3 &rayOrig, const Vec3 &rayDir, int numNearest);
		bool getCastRayResult(int index, CastRayResult &crr);

		int checkNodeVisibility(SceneNode &node, CameraNode &cam, bool checkOcclusion, bool calcLod);

		SceneNode &getRootNode() { return *_nodes[0]; }
		SceneNode &getDefCamNode() { return *_nodes[1]; }
		std::vector< SceneNode * > &getLightQueue() { return _spatialGraph->getLightQueue(); }
		std::vector<RenderQueueItem> &getRenderQueue() { return _spatialGraph->getRenderQueue(); }

		SceneNode *resolveNodeHandle(NodeHandle handle)
		{
			return (handle != 0 && (unsigned)(handle - 1) < _nodes.size()) ? _nodes[handle - 1] : 0x0;
		}

	protected:
		NodeHandle parseNode(SceneNodeTpl &tpl, SceneNode *parent);
		void removeNodeRec(SceneNode &node);

		void castRayInternal(SceneNode &node);

	protected:
		std::vector< SceneNode *>      _nodes;				// _nodes[0] is root node
		std::vector< uint32 >          _freeList;			// List of free slots
		std::vector< SceneNode * >     _findResults;
		std::vector< CastRayResult >   _castRayResults;
		SpatialGraph                   *_spatialGraph;

		std::map< SceneNodeType, NodeRegEntry >  _registry;  // Registry of node types

		Vec3                           _rayOrigin;			// Don't put these values on the stack during recursive search
		Vec3                           _rayDirection;		// Ditto
		int                            _rayNum;				// Ditto

		friend class Renderer;
	};
}

