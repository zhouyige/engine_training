#include "LeanCamera.h"
#include <map>
#include <string>
#include "LeanModel.h"
#include "LeanResourceManager.h"

namespace Lean3D
{
	SceneNodeTpl * ModelNode::parsingFunc(std::map<std::string, std::string>& attribs)
	{
		bool result = true;

		ModelNodeTpl *modelTpl = new ModelNodeTpl("", nullptr);

		std::map< std::string, std::string >::iterator itr = attribs.find("geometry");
		if (itr != attribs.end())
		{
			uint32 res = LeanRoot::resMana().addResource(ResourceTypes::Geometry, itr->second, 0, false);
			if (res != 0)
				modelTpl->geoRes = (GeometryResource *)LeanRoot::resMana().resolveResHandle(res);
		}
		else result = false;
		itr = attribs.find("softwareSkinning");
		if (itr != attribs.end())
		{
			if (_stricmp(itr->second.c_str(), "true") == 0 || _stricmp(itr->second.c_str(), "1") == 0)
				modelTpl->softwareSkinning = true;
			else
				modelTpl->softwareSkinning = false;
		}

		itr = attribs.find("lodDist1");
		if (itr != attribs.end()) modelTpl->lodDist1 = (float)atof(itr->second.c_str());
		itr = attribs.find("lodDist2");
		if (itr != attribs.end()) modelTpl->lodDist2 = (float)atof(itr->second.c_str());
		itr = attribs.find("lodDist3");
		if (itr != attribs.end()) modelTpl->lodDist3 = (float)atof(itr->second.c_str());
		itr = attribs.find("lodDist4");
		if (itr != attribs.end()) modelTpl->lodDist4 = (float)atof(itr->second.c_str());

		if (!result)
		{
			SAFE_DELETE(modelTpl);
		}

		return modelTpl;
	}

	SceneNode * ModelNode::factoryFunc(const SceneNodeTpl & nodeTpl)
	{
		if (nodeTpl.type != SceneNodeType::Model) return nullptr;

		return new ModelNode(*(ModelNodeTpl *)&nodeTpl);
	}
	ModelNode::~ModelNode()
	{
		_geometryRes = nullptr;
		_baseGeoRes = nullptr;
	}
	void ModelNode::recreateNodeList()
	{
	}

	ModelNode::ModelNode(const ModelNodeTpl & modelTpl) :
		SceneNode(modelTpl), _geometryRes(modelTpl.geoRes), _baseGeoRes(0x0),
		_lodDist1(modelTpl.lodDist1), _lodDist2(modelTpl.lodDist2),
		_lodDist3(modelTpl.lodDist3), _lodDist4(modelTpl.lodDist4),
		_softwareSkinning(modelTpl.softwareSkinning), _skinningDirty(false),
		_nodeListDirty(false), _morpherUsed(false), _morpherDirty(false)
	{
		if (_geometryRes != 0x0)
		{
			res = LeanRoot::resMana().resolveResHandle(_geometryRes->getHandle());
			if (res != 0x0 && res->getType() == ResourceTypes::Geometry)
				setGeometryRes(*(GeometryResource *)res);
			else
				Modules::setError("Invalid handle in h3dSetNodeParamI for H3DModel::GeoResI");
			return;
		}
	}
	void ModelNode::setGeometryRes(GeometryResource & geoRes)
	{
		// Init joint data
		_skinMatRows.resize(geoRes._joints.size() * 3);
		for (uint32 i = 0; i < _skinMatRows.size() / 3; ++i)
		{
			_skinMatRows[i * 3 + 0] = Vec4f(1, 0, 0, 0);
			_skinMatRows[i * 3 + 1] = Vec4f(0, 1, 0, 0);
			_skinMatRows[i * 3 + 2] = Vec4f(0, 0, 1, 0);
		}

		// Copy morph targets
		_morphers.resize(geoRes._morphTargets.size());
		for (uint32 i = 0; i < _morphers.size(); ++i)
		{
			Morpher &morpher = _morphers[i];

			morpher.name = geoRes._morphTargets[i].name;
			morpher.index = i;
			morpher.weight = 0;
		}

		if (!_morphers.empty() || _softwareSkinning)
		{
			Resource *clonedRes = Modules::resMan().resolveResHandle(
				Modules::resMan().cloneResource(geoRes, ""));
			_geometryRes = (GeometryResource *)clonedRes;
			_baseGeoRes = &geoRes;
		}
		else
		{
			_geometryRes = &geoRes;
			_baseGeoRes = 0x0;
		}

		_skinningDirty = true;
		updateLocalMeshAABBs();
	}
}

