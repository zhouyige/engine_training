#include "LeanUtil.h"
#include "OGLDeviceManager.h"
#include "LeanResourceManager.h"
#include "LeanGeometryRes.h"
#include "LeanMaterialRes.h"
#include "LeanGeometryRes.h"
#include "Lean3DRoot.h"
#include <map>
#include "OGLShaderManager.h"
using namespace std;
using namespace Lean3D;



ResHandle knightGeoRes = 0;
ResHandle knightAnim1Res = 0;
ResHandle knightAnim2Res = 0;
ResHandle knightTexRes = 0;
ResHandle knightMatriRes = 0;
MaterialResource *matres = 0x0;
GeometryResource *geores = 0x0;

static GLfloat g_viewMatrix[16];

GLUSboolean init(GLUSvoid)
{
	LeanRoot::init();
	//LeanRoot::resMana().addResource(ResourceTypes::Material, string("overlays/font.material.xml") , 0, true);
	//LeanRoot::resMana().addResource(ResourceTypes::Material, string("overlays/panel.material.xml"), 0, true);
	//LeanRoot::resMana().addResource(ResourceTypes::Material, string("overlays/panel.material.xml"), 0, true);

	//ResHandle hnightres =  LeanRoot::resMana().addResource(ResourceTypes::SceneGraph, string("models/knight/knight.scene.xml"), 0, true);
	knightGeoRes = LeanRoot::resMana().addResource(ResourceTypes::Geometry, string("models/knight/knight.geo"), 0, true);
	knightAnim1Res = LeanRoot::resMana().addResource(ResourceTypes::Animation, string("animations/knight_order.anim"), 0, true);
	knightAnim2Res = LeanRoot::resMana().addResource(ResourceTypes::Animation, string("animations/knight_attack.anim"), 0, true);
	knightTexRes = LeanRoot::resMana().addResource(ResourceTypes::Texture, string("models/knight/knight.jpg"), 0, true);
	knightMatriRes = LeanRoot::resMana().addResource(ResourceTypes::Material, string("models/knight/knight.material.xml"), 0, true);

	LeanRoot::loadResourceFromPath("../Binaries/Content");
	geores = (GeometryResource*)LeanRoot::resMana().resolveResHandle(knightGeoRes);
	matres = (MaterialResource*)LeanRoot::resMana().resolveResHandle(knightMatriRes);
	uint32 shaderResHandle = matres->getElemParamI(MaterialElemType::MaterialElem, 0, MaterialElemType::MatShaderI);
	ShaderResource* shaderRes = (ShaderResource*)LeanRoot::resMana().resolveResHandle(shaderResHandle);
	ShaderPass* sc = shaderRes->findPass("AMBIENT");
	geores->genGeomResVAO(sc->shaderHandle);
	int i = 0;
	g_OGLDiv->clearColor(0.5f, 0.1f, 0.8f, 1.0f);


	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{

	g_OGLDiv->setViewport(0, 0, width, height);
	GLfloat projectionMatrix[16];
	glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat)width / (GLfloat)height, 1.0f, 100.0f);
	uint32 shaderResHandle = matres->getElemParamI(MaterialElemType::MaterialElem, 0, MaterialElemType::MatShaderI);
	ShaderResource* shaderRes = (ShaderResource*)LeanRoot::resMana().resolveResHandle(shaderResHandle);
	ShaderPass* sc = shaderRes->findPass("AMBIENT");
	GLint location = 0;
	for (int i = 0; i < sc->uniforms.size(); ++i)
	{
		if (sc->uniforms[i].getName() == "viewProjMat")
		{
			location = sc->uniforms[i]._location;
		}
	}
	g_OGLDiv->shaderManaRef()->setShaderUniform(location, UNIFORM_FLOAT44, projectionMatrix);
	
}
bool setMaterialRes(MaterialResource *matrialRes, const string &shaderContext, ShaderResource *shaderRes)
{
	if (matrialRes == 0x0)
		return false;
	bool firstRes = 1;
	bool result = true;

	if (firstRes)
	{
		shaderRes = matrialRes->getShaderRes();
	}
}
bool setMaterial(MaterialResource *matrialRes, const string &shaderContext)
{
	if (matrialRes == 0x0)
	{
		g_OGLDiv->setBlendMode(false);
		g_OGLDiv->setAlphaToCoverage(false);
		g_OGLDiv->setDepthTest(true);
		g_OGLDiv->setDepthFunc(DSS_DEPTHFUNC_LESS_EQUAL);
		g_OGLDiv->setDepthMask(true);
		return false;
	}
	setMaterialRes(matrialRes, shaderContext,0x0);
	return true;
}

GLUSboolean update(GLUSfloat time)
{
	g_OGLDiv->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	g_OGLDiv->buffManaRef()->setCurrentVAO(geores->getVAOHandle());
	uint32 shaderResHandle = matres->getElemParamI(MaterialElemType::MaterialElem, 0, MaterialElemType::MatShaderI);
	ShaderResource* shaderRes = (ShaderResource*)LeanRoot::resMana().resolveResHandle(shaderResHandle);
	ShaderPass* sc = shaderRes->findPass("AMBIENT");
	g_OGLDiv->shaderManaRef()->setCurrentShader(sc->shaderHandle);
	//g_OGLDiv->shaderManaRef()->setShaderSampler(sc->shaderCombs[0].)
	g_OGLDiv->drawIndexed(GL_TRIANGLES, geores->getIndexCount(), GL_UNSIGNED_SHORT);
	return GLUS_TRUE;
}


GLUSvoid terminate(GLUSvoid)
{

}

int main(int argc, char* argv[])
{
	EGLint eglConfigAttributes[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};

	EGLint eglContextAttributes[] = {
		EGL_CONTEXT_MAJOR_VERSION, 3,
		EGL_CONTEXT_MINOR_VERSION, 2,
		EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
		EGL_NONE
	};
	glusWindowSetInitFunc(init);

	glusWindowSetReshapeFunc(reshape);

	glusWindowSetUpdateFunc(update);

	glusWindowSetTerminateFunc(terminate);

	if (!glusWindowCreate("GLUS Example Window", 640, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes, 0))
	{
		printf("Could not create window!\n");
		return -1;
	}

	glusWindowRun();

	return 0;
}