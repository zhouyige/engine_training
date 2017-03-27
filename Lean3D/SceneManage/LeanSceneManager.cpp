#include "LeanSceneManager.h"
#include "Lean3DRoot.h"
#include <string>
#include <map>
#include "LeanSceneGraphRes.h"

namespace Lean3D
{


	//

	SceneManager::SceneManager()
	{
		SceneNode *rootNode = GroupNode::factoryFunc(GroupNodeTpl("RootNode"));
		rootNode->_handle = ROOTNODE;
		_nodes.push_back(rootNode);

		_spatialGraph = new SpatialGraph();
	}

	SceneManager::~SceneManager()
	{
		SAFE_DELETE(_spatialGraph);
		
		for(uint32 i=0; i < _nodes.size(); ++i)
		{
			SAFE_DELETE(_nodes[i]);
		}
	}

	void SceneManager::registerNodeType(SceneNodeType nodeType, const std::string & typeString, NodeTypeParsingFunc pf, NodeTypeFactoryFunc ff)
	{
		NodeRegEntry entry;
		entry.typeString = typeString;
		entry.parsingFunc = pf;
		entry.factoryFunc = ff;
		_registry[nodeType] = entry;
	}

	NodeRegEntry * SceneManager::findType(SceneNodeType type)
	{
		std::map< SceneNodeType, NodeRegEntry >::iterator itr = _registry.find(type);

		if (itr != _registry.end()) 
			return &itr->second;
		else 
			return nullptr;
	}

	NodeRegEntry * SceneManager::findType(const std::string & typeString)
	{
		std::map< SceneNodeType, NodeRegEntry >::iterator itr = _registry.begin();
		NodeRegEntry *result = nullptr;

		while (itr != _registry.end())
		{
			if (itr->second.typeString == typeString) 
				result = &itr->second;

			++itr;
		}

		return result;
	}

	void SceneManager::updateNodes()
	{
	}

	void SceneManager::updateSpatialNode(uint32 sgHandle)
	{
		_spatialGraph->updateNode(sgHandle);
	}

	void SceneManager::updateQueues(const Frustum & frustum1, const Frustum * frustum2, RenderingOrder order, uint32 filterIgnore, bool lightQueue, bool renderableQueue)
	{
		_spatialGraph->updateQueues(frustum1, frustum2, order, filterIgnore, lightQueue, renderableQueue);
	}

	NodeHandle SceneManager::addNode(SceneNode * node, SceneNode & parent)
	{
		if (node == nullptr) return 0;

		// Check if node can be attached to parent
		if (!node->canAttach(parent))
		{
			LEAN_DEGUG_LOG("Can't attach node '%s' to parent '%s'", node->_name.c_str(), parent._name.c_str());
			delete node; node = 0x0;
			return 0;
		}

		node->_parent = &parent;

		// Attach to parent
		parent._children.push_back(node);

		// Raise event
		node->onAttach(parent);

		// Mark tree as dirty
		node->markDirty();

		// Register node in spatial graph
		_spatialGraph->addNode(*node);

		// Insert node in free slot
		if (!_freeList.empty())
		{
			uint32 slot = _freeList.back();
			ASSERT(_nodes[slot] == 0x0);
			_freeList.pop_back();

			node->_handle = slot + 1;
			_nodes[slot] = node;
			return slot + 1;
		}
		else
		{
			_nodes.push_back(node);
			node->_handle = (NodeHandle)_nodes.size();
			return node->_handle;
		}
		return NodeHandle();
	}

	NodeHandle SceneManager::addNodes(SceneNode & parent, SceneGraphResource & sgRes)
	{
		return parseNode(*sgRes.getRootNode(), &parent);
	}

	void SceneManager::removeNode(SceneNode & node)
	{
		SceneNode *parent = node._parent;
		SceneNode *nodeAddr = &node;

		removeNodeRec(node);  //  delete node if it is not rootnode

		// Remove node from parent
		if (parent != nullptr)
		{
			for (uint32 i = 0; i < parent->_children.size(); ++i)
			{
				if (parent->_children[i] == nodeAddr)
				{
					parent->_children.erase(parent->_children.begin() + i);
					break;
				}
			}
			parent->markDirty();
		}
		else  // Rootnode
		{
			node._children.clear();
			node.markDirty();
		}
	}

	bool SceneManager::relocateNode(SceneNode & node, SceneNode & parent)
	{
		if (node._handle == ROOTNODE) return false;

		if (!node.canAttach(parent))
		{
			LEAN_DEGUG_LOG("Can't attach node to parent in h3dSetNodeParent", 0);
			return false;
		}

		// Detach from old parent
		node.onDetach(*node._parent);
		for (uint32 i = 0; i < node._parent->_children.size(); ++i)
		{
			if (node._parent->_children[i] == &node)
			{
				node._parent->_children.erase(node._parent->_children.begin() + i);
				break;
			}
		}

		// Attach to new parent
		parent._children.push_back(&node);
		node._parent = &parent;
		node.onAttach(parent);

		parent.markDirty();
		node._parent->markDirty();

		return true;
	}

	int SceneManager::findNodes(SceneNode & startNode, const std::string & name, SceneNodeType type)
	{
		int count = 0;

		if (type == SceneNodeType::Undefined || startNode._type == type)
		{
			if (name == "" || startNode._name == name)
			{
				_findResults.push_back(&startNode);
				++count;
			}
		}

		for (uint32 i = 0; i < startNode._children.size(); ++i)
		{
			count += findNodes(*startNode._children[i], name, type);
		}

		return count;
	}

	int SceneManager::castRay(SceneNode & node, const Vec3 & rayOrig, const Vec3 & rayDir, int numNearest)
	{
		_castRayResults.resize(0);

		if (node._flags & SceneNodeFlags::NoRayQuery) return 0;

		_rayOrigin = rayOrig;
		_rayDirection = rayDir;
		_rayNum = numNearest;

		castRayInternal(node);

		return (int)_castRayResults.size();
	}

	bool SceneManager::getCastRayResult(int index, CastRayResult & crr)
	{
		if ((uint32)index < _castRayResults.size())
		{
			crr = _castRayResults[index];

			return true;
		}

		return false;
	}

	int SceneManager::checkNodeVisibility(SceneNode & node, CameraNode & cam, bool checkOcclusion, bool calcLod)
	{
		ASSERT(false);
		//TODO
		return 0;
	}

	NodeHandle SceneManager::parseNode(SceneNodeTpl & tpl, SceneNode * parent)
	{
		if (parent == nullptr)  return 0;

		SceneNode *sn = nullptr;

		if (tpl.type == SceneNodeType::Undefined)
		{
			// Reference node
			//NodeHandle handle = parseNode(*((ReferenceNodeTpl *)&tpl)->sgRes->getRootNode(), parent);
			//sn = LeanRoot::sceneMana().resolveNodeHandle(handle);
			//if (sn != 0x0)
			//{
			//	sn->_name = tpl.name;
			//	sn->setTransform(tpl.trans, tpl.rot, tpl.scale);
			//	sn->_attachment = tpl.attachmentString;
			//}
		}
		else
		{
			std::map< SceneNodeType, NodeRegEntry >::iterator itr = _registry.find(tpl.type);
			if (itr != _registry.end()) sn = (*itr->second.factoryFunc)(tpl);
			if (sn != nullptr) addNode(sn, *parent);
		}

		if (sn == nullptr) return 0;

		// Parse children 解析孩子节点
		for (uint32 i = 0; i < tpl.children.size(); ++i)
		{
			parseNode(*tpl.children[i], sn);
		}

		return sn->getHandle();
	}

	void SceneManager::removeNodeRec(SceneNode & node)
	{
		NodeHandle handle = node._handle;

		// Raise event
		if (handle != ROOTNODE) node.onDetach(*node._parent);

		// Remove children
		for (uint32 i = 0; i < node._children.size(); ++i)
		{
			removeNodeRec(*node._children[i]);
		}

		// Delete node
		if (handle != ROOTNODE)
		{
			_spatialGraph->removeNode(node._sgHandle);
			delete _nodes[handle - 1]; _nodes[handle - 1] = 0x0;
			_freeList.push_back(handle - 1);
		}
	}

	void SceneManager::castRayInternal(SceneNode & node)
	{
		if (!(node._flags & SceneNodeFlags::NoRayQuery))
		{
			Vec3 intsPos;

			if (node.checkIntersection(_rayOrigin, _rayDirection, intsPos))
			{
				float dist = (intsPos - _rayOrigin).length();

				CastRayResult crr;
				crr.node = &node;
				crr.distance = dist;
				crr.intersection = intsPos;

				bool inserted = false;
				for (std::vector< CastRayResult >::iterator it = _castRayResults.begin(); it != _castRayResults.end(); ++it)
				{
					if (dist < it->distance)
					{
						_castRayResults.insert(it, crr);
						inserted = true;
						break;
					}
				}

				if (!inserted)
				{
					_castRayResults.push_back(crr);
				}

				if (_rayNum > 0 && (int)_castRayResults.size() > _rayNum)
				{
					_castRayResults.pop_back();
				}
			}

			for (size_t i = 0, s = node._children.size(); i < s; ++i)
			{
				castRayInternal(*node._children[i]);
			}
		}
	}
}


