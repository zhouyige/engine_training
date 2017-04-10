#include "LeanMesh.h"
#include <map>
#include <string>
#include "LeanResourceManager.h"
#include "LeanModel.h"

namespace Lean3D
{
	SceneNodeTpl * MeshNode::parsingFunc(std::map<std::string, std::string>& attribs)
	{
		bool result = true;

		std::map< std::string, std::string >::iterator itr;
		MeshNodeTpl *meshTpl = new MeshNodeTpl("", nullptr, 0, 0, 0, 0);

		itr = attribs.find("material");
		if (itr != attribs.end())
		{
			uint32 res = LeanRoot::resMana().addResource(ResourceTypes::Material, itr->second, 0, false);
			if (res != 0)
				meshTpl->matRes = (MaterialResource *)LeanRoot::resMana().resolveResHandle(res);
		}
		else result = false;
		itr = attribs.find("batchStart");
		if (itr != attribs.end()) meshTpl->batchStart = atoi(itr->second.c_str());
		else result = false;
		itr = attribs.find("batchCount");
		if (itr != attribs.end()) meshTpl->batchCount = atoi(itr->second.c_str());
		else result = false;
		itr = attribs.find("vertRStart");
		if (itr != attribs.end()) meshTpl->vertRStart = atoi(itr->second.c_str());
		else result = false;
		itr = attribs.find("vertREnd");
		if (itr != attribs.end()) meshTpl->vertREnd = atoi(itr->second.c_str());
		else result = false;

		itr = attribs.find("lodLevel");
		if (itr != attribs.end()) meshTpl->lodLevel = atoi(itr->second.c_str());

		if (!result)
		{
			delete meshTpl; meshTpl = nullptr;
		}

		return meshTpl;
	}

	SceneNode * MeshNode::factoryFunc(const SceneNodeTpl & nodeTpl)
	{
		if (nodeTpl.type != SceneNodeType::Mesh) return nullptr;

		return new MeshNode(*(MeshNodeTpl *)&nodeTpl);
	}

	bool MeshNode::canAttach(SceneNode & parent)
	{
		return (parent.getType() == SceneNodeType::Model) ||
			(parent.getType() == SceneNodeType::Mesh) ||
			(parent.getType() == SceneNodeType::Joint);
	}

	bool MeshNode::checkIntersection(const Vec3 & rayOrig, const Vec3 & rayDir, Vec3 & intsPos) const
	{
		if (_lodLevel != 0) return false;

		if (!rayAABBIntersection(rayOrig, rayDir, _bBox.min, _bBox.max)) return false;

		GeometryResource *geoRes = _parentModel->getGeometryResource();
		if (geoRes == 0x0 || geoRes->getIndexData() == 0x0 || geoRes->getVertPosData() == 0x0) return false;

		// Transform ray to local space
		Matrix4 m = _absTrans.inverted();
		Vec3 orig = m * rayOrig;
		Vec3 dir = m * (rayOrig + rayDir) - orig;

		Vec3 nearestIntsPos = Vec3(Math::MaxFloat, Math::MaxFloat, Math::MaxFloat);
		bool intersection = false;

		// Check triangles
		for (uint32 i = _batchStart; i < _batchStart + _batchCount; i += 3)
		{
			Vec3 *vert0, *vert1, *vert2;

			if (geoRes->is16BitIndex())
			{
				vert0 = &geoRes->getVertPosData()[((uint16 *)geoRes->getIndexData())[i + 0]];
				vert1 = &geoRes->getVertPosData()[((uint16 *)geoRes->getIndexData())[i + 1]];
				vert2 = &geoRes->getVertPosData()[((uint16 *)geoRes->getIndexData())[i + 2]];
			}
			else
			{
				vert0 = &geoRes->getVertPosData()[((uint32 *)geoRes->getIndexData())[i + 0]];
				vert1 = &geoRes->getVertPosData()[((uint32 *)geoRes->getIndexData())[i + 1]];
				vert2 = &geoRes->getVertPosData()[((uint32 *)geoRes->getIndexData())[i + 2]];
			}

			if (rayTriangleIntersection(orig, dir, *vert0, *vert1, *vert2, intsPos))
			{
				intersection = true;
				if ((intsPos - orig).length() < (nearestIntsPos - orig).length())
					nearestIntsPos = intsPos;
			}
		}

		intsPos = _absTrans * nearestIntsPos;

		return intersection;
	}

	void MeshNode::onAttach(SceneNode & parentNode)
	{
		SceneNode *node = &parentNode;
		while (node->getType() != SceneNodeType::Model) node = node->getParent();
		_parentModel = (ModelNode *)node;
		_parentModel->markNodeListDirty();
	}

	void MeshNode::onDetach(SceneNode & parentNode)
	{
		if (_parentModel != nullptr) _parentModel->markNodeListDirty();
	}

	void MeshNode::onPostUpdate()
	{
		_bBox = _localBBox;
		_bBox.transform(_absTrans);
	}

	MeshNode::MeshNode(const MeshNodeTpl & meshTpl) :
		SceneNode(meshTpl),
		_materialRes(meshTpl.matRes), _batchStart(meshTpl.batchStart), _batchCount(meshTpl.batchCount),
		_vertRStart(meshTpl.vertRStart), _vertREnd(meshTpl.vertREnd), _lodLevel(meshTpl.lodLevel),
		_parentModel(0x0)
	{
		_renderable = true;

		if (_materialRes != 0x0)
			_sortKey = (float)_materialRes->getHandle();
	}

	MeshNode::~MeshNode()
	{
		_materialRes = nullptr;
		for (uint32 i = 0; i < _occQueries.size(); ++i)
		{
			if (_occQueries[i] != 0)
				g_OGLDiv->oclQueryRef()->destroyQuery(_occQueries[i]);
		}
	}
}

