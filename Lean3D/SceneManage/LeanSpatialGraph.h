#pragma once
#include "LeanUtil.h"
#include "LeanSceneNode.h"

namespace Lean3D
{
	struct RenderQueueItem
	{
		SceneNode				*node;
		SceneNodeType			type; 
		float					sortKey;
		RenderQueueItem() { }
		RenderQueueItem(SceneNodeType type, float sortKey, SceneNode *node)
			: node(node), type(type), sortKey(sortKey) {}
	};

	class SpatialGraph
	{
	public:
		SpatialGraph();
		void addNode(SceneNode &sceneNode);
		void removeNode(uint32 sgHandle);
		void updateNode(uint32 sgHandle);

		void updateQueues(const Frustum &frustum1, const Frustum *frustum2,
			RenderingOrder order, uint32 filterIgnore, bool lightQueue, bool renderQueue);

		std::vector< SceneNode * > &getLightQueue() { return _lightQueue; }
		std::vector<RenderQueueItem> &getRenderQueue() { return _renderQueue; }

	protected:
		std::vector< SceneNode * >     _nodes;		// Renderable nodes and lights
		std::vector< uint32 >          _freeList;
		std::vector< SceneNode * >     _lightQueue;
		std::vector<RenderQueueItem>   _renderQueue;
	};
}


