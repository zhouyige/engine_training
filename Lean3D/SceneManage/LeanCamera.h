#pragma once
#include "LeanSceneNode.h"
#include <map>

namespace Lean3D
{

	struct CameraNodeTpl : public SceneNodeTpl
	{
		//PPipelineResource   pipeRes;
		//PTextureResource    outputTex;
		float               leftPlane, rightPlane;
		float               bottomPlane, topPlane;
		float               nearPlane, farPlane;
		int                 outputBufferIndex;
		bool                orthographic;
		bool                occlusionCulling;

		CameraNodeTpl(const std::string &name/*, PipelineResource *pipelineRes*/) :
			SceneNodeTpl(SceneNodeType::Camera, name),
			/*pipeRes(pipelineRes),*/
			/*outputTex(0x0),*/
			// Default params: fov=45, aspect=4/3
			leftPlane(-0.055228457f), rightPlane(0.055228457f), bottomPlane(-0.041421354f),
			topPlane(0.041421354f), nearPlane(0.1f), farPlane(1000.0f), outputBufferIndex(0),
			orthographic(false), occlusionCulling(false)
		{
		}
	};

	class CameraNode : public SceneNode
	{
	public:
		CameraNode(const CameraNodeTpl &cameraTpl);
		~CameraNode();

		static SceneNodeTpl *parsingFunc(std::map< std::string, std::string > &attribs);
		static SceneNode *factoryFunc(const SceneNodeTpl &nodeTpl);


		int getParamI(int param);
		void setParamI(int param, int value);
		float getParamF(int param, int compIdx);
		void setParamF(int param, int compIdx, float value);

		void setupViewParams(float fovOrWidth, float aspect, float nearPlane, float farPlane);

		const Frustum &getFrustum() { return _frustum; }
		const Matrix4 &getViewMat() { return _viewMat; }
		const Matrix4 &getProjMat() { return _projMat; }
		const Vec3 &getAbsPos() { return _absPos; }

	private:
		//CameraNode(const CameraNodeTpl &cameraTpl);
		void onPostUpdate();

	private:
		//PPipelineResource   _pipelineRes;
		//PTextureResource    _outputTex;
		Matrix4             _viewMat, _projMat;
		Frustum             _frustum;
		Vec3                _absPos;
		int					_vpX, _vpY, _vpWidth, _vpHeight;
		float               _frustLeft, _frustRight, _frustBottom, _frustTop;
		float               _fovOrWidth;  // if is ortho this variable mean width
		float				_aspect;
		float               _frustNear, _frustFar;
		int                 _outputBufferIndex;
		int                 _occSet;
		bool                _orthographic;  // Perspective or orthographic frustum?

		friend class SceneManager;
		friend class Renderer;
	};	

}


