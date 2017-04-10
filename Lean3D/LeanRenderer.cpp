#include "LeanRenderer.h"

namespace Lean3D
{
	LeanRenderer::LeanRenderer()
	{
	}
	LeanRenderer::~LeanRenderer()
	{
	}
	void LeanRenderer::registerRenderFunc(int nodeType, RenderFunc rf)
	{
	}
	unsigned char * LeanRenderer::useScratchBuf(uint32 minSize)
	{
		return nullptr;
	}
	bool LeanRenderer::init()
	{
		return false;
	}
	void LeanRenderer::initStates()
	{
	}
	void LeanRenderer::drawAABB(const Vec3 & bbMin, const Vec3 & bbMax)
	{
	}
	void LeanRenderer::drawSphere(const Vec3 & pos, float radius)
	{
	}
	void LeanRenderer::drawCone(float height, float fov, const Matrix4 & transMat)
	{
	}
	bool LeanRenderer::createShaderComb(const char * vertexShader, const char * fragmentShader, ShaderPass & pass)
	{
		return false;
	}
	void LeanRenderer::releaseShaderComb(ShaderPass & pass)
	{
	}
	void LeanRenderer::setShaderComb(ShaderPass * pass)
	{
	}
	void LeanRenderer::commitGeneralUniforms()
	{
	}
	bool LeanRenderer::setMaterial(MaterialResource * materialRes, const std::string & shaderContext)
	{
		return false;
	}
	bool LeanRenderer::createShadowRB(uint32 width, uint32 height)
	{
		return false;
	}
	void LeanRenderer::releaseShadowRB()
	{
	}
	int LeanRenderer::registerOccSet()
	{
		return 0;
	}
	void LeanRenderer::unregisterOccSet(int occSet)
	{
	}
	void LeanRenderer::drawOccProxies(uint32 list)
	{
	}
	void LeanRenderer::showOverlays(const float * verts, uint32 vertCount, float * colRGBA, MaterialResource * matRes, int flags)
	{
	}
	void LeanRenderer::clearOverlays()
	{
	}
	void LeanRenderer::drawMeshes(uint32 firstItem, uint32 lastItem, const std::string & shaderContext, const std::string & theClass, bool debugView, const Frustum * frust1, const Frustum * frust2, RenderingOrder order, int occSet)
	{
	}
	void LeanRenderer::render(CameraNode * camNode)
	{
	}
	void LeanRenderer::finalizeFrame()
	{
	}
	void LeanRenderer::setupViewMatrices(const Matrix4 & viewMat, const Matrix4 & projMat)
	{
	}
	void LeanRenderer::createPrimitives()
	{
	}
	bool LeanRenderer::setMaterialRec(MaterialResource * materialRes, const std::string & shaderContext, ShaderResource * shaderRes)
	{
		return false;
	}
	void LeanRenderer::setupShadowMap(bool noShadows)
	{
	}
	Matrix4 LeanRenderer::calcCropMatrix(const Frustum & frustSlice, const Vec3 lightPos, const Matrix4 & lightViewProjMat)
	{
		return Matrix4();
	}
	void LeanRenderer::updateShadowMap()
	{
	}
	void LeanRenderer::drawOverlays(const std::string & shaderContext)
	{
	}
	void LeanRenderer::bindPipeBuffer(uint32 rbObj, const std::string & sampler, uint32 bufIndex)
	{
	}
	void LeanRenderer::clear(bool depth, bool buf0, bool buf1, bool buf2, bool buf3, float r, float g, float b, float a)
	{
	}
	void LeanRenderer::drawFSQuad(Resource * matRes, const std::string & shaderContext)
	{
	}
	void LeanRenderer::drawGeometry(const std::string & shaderContext, const std::string & theClass, RenderingOrder order, int occSet)
	{
	}
	void LeanRenderer::drawLightGeometry(const std::string & shaderContext, const std::string & theClass, bool noShadows, RenderingOrder order, int occSet)
	{
	}
	void LeanRenderer::drawLightShapes(const std::string & shaderContext, bool noShadows, int occSet)
	{
	}
	void LeanRenderer::drawRenderables(const std::string & shaderContext, const std::string & theClass, bool debugView, const Frustum * frust1, const Frustum * frust2, RenderingOrder order, int occSet)
	{
	}
	void LeanRenderer::renderDebugView()
	{
	}
	void LeanRenderer::finishRendering()
	{
	}
}

