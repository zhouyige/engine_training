#include "LeanSpatialGraph.h"
#include "Lean3DRoot.h"

namespace Lean3D
{
	SpatialGraph::SpatialGraph()
	{
		_lightQueue.reserve(20);
		_renderQueue.reserve(500);
	}
	void SpatialGraph::addNode(SceneNode & sceneNode)
	{
		if (!sceneNode._renderable && sceneNode._type != SceneNodeType::Light)
			return;

		if (!_freeList.empty())
		{
			uint32 index = _freeList.back();
			ASSERT(_nodes[index] == nullptr);
			_freeList.pop_back();

			sceneNode._sgHandle = index + 1;
			_nodes[index] = &sceneNode;
		}
		else
		{
			_nodes.push_back(&sceneNode);
			sceneNode._sgHandle = (uint32)_nodes.size();
		}
	}

	void SpatialGraph::removeNode(uint32 sgHandle)
	{
		if (sgHandle == 0 || _nodes[sgHandle - 1] == nullptr) return;

		// Reset queues
		_lightQueue.resize(0);
		_renderQueue.resize(0);

		_nodes[sgHandle - 1]->_sgHandle = 0;
		_nodes[sgHandle - 1] = nullptr;
		_freeList.push_back(sgHandle - 1);
	}

	void SpatialGraph::updateNode(uint32 sgHandle)
	{
	}

	void SpatialGraph::updateQueues(const Frustum & frustum1, const Frustum * frustum2,
		RenderingOrder order, uint32 filterIgnore,
		bool lightQueue, bool renderQueue)
	{
		ASSERT(false);
		//LeanRoot::sceneMana()  TODO**Modules::sceneMan().updateNodes(); 
		Vec3 camPos(frustum1.getOrigin());

		ASSERT(false);
		//TODO
		//if (Modules::renderer().getCurCamera() != 0x0)
		//	camPos = Modules::renderer().getCurCamera()->getAbsPos();

		//clear Queue
		if (lightQueue) _lightQueue.resize(0);
		if (renderQueue) _renderQueue.resize(0);

		//culling

		for (size_t i = 0, s = _nodes.size(); i < s; ++i)
		{
			SceneNode *node = _nodes[i];
			if (node == nullptr || (node->_flags & filterIgnore)) continue;

			if (renderQueue && node->_renderable)
			{
				if (!frustum1.cullBox(node->_bBox) &&
					(frustum2 == 0x0 || !frustum2->cullBox(node->_bBox)))
				{
					//if (node->_type == SceneNodeType::Mesh)  // TODO: Generalize and optimize this
					//{
					//	uint32 curLod = ((MeshNode *)node)->getParentModel()->calcLodLevel(camPos);
					//	if (((MeshNode *)node)->getLodLevel() != curLod) continue;
					//}

					float sortKey = 0;

					switch (order)
					{
					case RenderingOrder::StateChanges:
						sortKey = node->_sortKey;
						break;
					case RenderingOrder::FrontToBack:
						sortKey = nearestDistToAABB(frustum1.getOrigin(), node->_bBox.min, node->_bBox.max);
						break;
					case RenderingOrder::BackToFront:
						sortKey = -nearestDistToAABB(frustum1.getOrigin(), node->_bBox.min, node->_bBox.max);
						break;
					}

					_renderQueue.push_back(RenderQueueItem(node->_type, sortKey, node));
				}
			}
			else if (lightQueue && node->_type == SceneNodeType::Light)
			{
				_lightQueue.push_back(node);
			}
		}

		// Sort
		if (order != RenderingOrder::None)
			std::sort(_renderQueue.begin(), _renderQueue.end(),
				[](const RenderQueueItem &a, const RenderQueueItem &b)
		{
			return a.sortKey < b.sortKey;
		});
	}
}

