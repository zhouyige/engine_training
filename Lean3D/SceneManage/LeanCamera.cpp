#include "LeanCamera.h"
#include <map>
#include <string>

namespace Lean3D
{
	
	SceneNodeTpl * CameraNode::parsingFunc(std::map<std::string, std::string>& attribs)
	{
		//bool result = true;

		//std::map< std::string, std::string >::iterator itr;
		//CameraNodeTpl *cameraTpl = new CameraNodeTpl(""/*, 0x0*/);

		//itr = attribs.find("pipeline");
		//if (itr != attribs.end())
		//{
		//	uint32 res = Modules::resMan().addResource(ResourceTypes::Pipeline, itr->second, 0, false);
		//	cameraTpl->pipeRes = (PipelineResource *)Modules::resMan().resolveResHandle(res);
		//}
		//else result = false;
		//itr = attribs.find("outputTex");
		//if (itr != attribs.end())
		//{
		//	cameraTpl->outputTex = (TextureResource *)Modules::resMan().findResource(
		//		ResourceTypes::Texture, itr->second);
		//}
		//itr = attribs.find("outputBufferIndex");
		//if (itr != attribs.end()) cameraTpl->outputBufferIndex = atoi(itr->second.c_str());
		//itr = attribs.find("leftPlane");
		//if (itr != attribs.end()) cameraTpl->leftPlane = (float)atof(itr->second.c_str());
		//itr = attribs.find("rightPlane");
		//if (itr != attribs.end()) cameraTpl->rightPlane = (float)atof(itr->second.c_str());
		//itr = attribs.find("bottomPlane");
		//if (itr != attribs.end()) cameraTpl->bottomPlane = (float)atof(itr->second.c_str());
		//itr = attribs.find("topPlane");
		//if (itr != attribs.end()) cameraTpl->topPlane = (float)atof(itr->second.c_str());
		//itr = attribs.find("nearPlane");
		//if (itr != attribs.end()) cameraTpl->nearPlane = (float)atof(itr->second.c_str());
		//itr = attribs.find("farPlane");
		//if (itr != attribs.end()) cameraTpl->farPlane = (float)atof(itr->second.c_str());
		//itr = attribs.find("orthographic");
		//if (itr != attribs.end())
		//{
		//	if (_stricmp(itr->second.c_str(), "true") == 0 || _stricmp(itr->second.c_str(), "1") == 0)
		//		cameraTpl->orthographic = true;
		//	else
		//		cameraTpl->orthographic = false;
		//}
		//itr = attribs.find("occlusionCulling");
		//if (itr != attribs.end())
		//{
		//	if (_stricmp(itr->second.c_str(), "true") == 0 || _stricmp(itr->second.c_str(), "1") == 0)
		//		cameraTpl->occlusionCulling = true;
		//	else
		//		cameraTpl->occlusionCulling = false;
		//}

		//if (!result)
		//{
		//	delete cameraTpl; cameraTpl = 0x0;
		//}

		//return cameraTpl;

		return nullptr;
	}

	SceneNode * CameraNode::factoryFunc(const SceneNodeTpl & nodeTpl)
	{

		return nullptr;
	}

	CameraNode::CameraNode(const CameraNodeTpl &cameraTpl)
		: SceneNode(cameraTpl)
	{
	//	_pipelineRes = cameraTpl.pipeRes;
//		_outputTex = cameraTpl.outputTex;
		_outputBufferIndex = cameraTpl.outputBufferIndex;
		_vpX = 0; _vpY = 0; _vpWidth = 320; _vpHeight = 240;
		_frustLeft = cameraTpl.leftPlane;
		_frustRight = cameraTpl.rightPlane;
		_frustBottom = cameraTpl.bottomPlane;
		_frustTop = cameraTpl.topPlane;
		_frustNear = cameraTpl.nearPlane;
		_frustFar = cameraTpl.farPlane;
		_orthographic = cameraTpl.orthographic;
		//_occSet = cameraTpl.occlusionCulling ? Modules::renderer().registerOccSet() : -1;

	}

	CameraNode::~CameraNode()
	{
		// TODO
	}

	int CameraNode::getParamI(int param)
	{
		return 0;
	}

	void CameraNode::setParamI(int param, int value)
	{
	}

	float CameraNode::getParamF(int param, int compIdx)
	{
		return 0.0f;
	}

	void CameraNode::setParamF(int param, int compIdx, float value)
	{
	}

	void CameraNode::setupViewParams(float fovOrWidth, float aspect, float nearPlane, float farPlane)
	{
		_fovOrWidth		= fovOrWidth;
		_aspect			= aspect;
		_frustNear		= nearPlane;
		_frustFar		= farPlane;

		if (_orthographic)
		{
			float ymax = fovOrWidth / aspect * 0.5f;
			float xmax = fovOrWidth * 0.5f;
			_frustLeft = -xmax;
			_frustRight = xmax;
			_frustBottom = -ymax;
			_frustTop = ymax;
		}
		else
		{
			float ymax = nearPlane * tanf(degToRad(fovOrWidth / 2));
			float xmax = ymax * aspect;
			_frustLeft = -xmax;
			_frustRight = xmax;
			_frustBottom = -ymax;
			_frustTop = ymax;
		}
		onPostUpdate();
	}

	void CameraNode::onPostUpdate()
	{
		if (_orthographic)
		{
			_projMat = Matrix4::OrthoMat(_frustLeft, _frustRight, _frustBottom, _frustTop, _frustNear, _frustFar);
		}
		else
		{
			_projMat = Matrix4::PerspectiveMat(_frustLeft, _frustRight, _frustBottom, _frustTop, _frustNear, _frustFar);
		}

		//get absolute position
		_absPos = Vec3(_absTrans.c[3][0], _absTrans.c[3][1], _absTrans.c[3][2]);
		
		// update view matrix
		_viewMat = _absTrans.inverted();
		
		//update frustum
		_frustum.buildViewFrustum(_viewMat, _projMat);
	}

}

