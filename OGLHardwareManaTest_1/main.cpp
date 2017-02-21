#include "windows.h"
#include "LeanUtil.h"
#include "OGLBufferManager.h"
#include "OGLOcclusionQuery.h"
#include "OGLRenderBufferManager.h"
#include "OGLTextureManager.h"
#include "OGLShaderManager.h"

using namespace Lean3D;

OGLBufferManager g_bufferManager;
OGLShaderManager g_shaderManager;
OGLTextureManager g_textureManager;
OGLRenderBufferManager *g_renderbufManager;

struct LightProperties
{
	GLfloat direction[3];
	GLfloat ambientColor[4];
	GLfloat diffuseColor[4];
	GLfloat specularColor[4];
};
struct MaterialProperties
{
	GLfloat ambientColor[4];
	GLfloat diffuseColor[4];
	GLfloat specularColor[4];
	GLfloat specularExponent;
};
struct LightLocations
{
	GLint directionLocation;
	GLint ambientColorLocation;
	GLint diffuseColorLocation;
	GLint specularColorLocation;
};
struct MaterialLocations
{
	GLint ambientColorLocation;
	GLint diffuseColorLocation;
	GLint specularColorLocation;
	GLint specularExponentLocation;

	GLint diffuseTextureLocation;
};

uint32 g_shaderHandle;
uint32 g_fboshaderHandle;
GLint g_fboprojectionMatrixLocation;
GLint g_fbomodelViewMatrixLocation;
GLint g_fbonormalMatrixLocation;
GLint g_fbovertexLocation;
GLint g_fbotextureLocation;
uint32 g_fbovertexVBOHandle;
uint32 g_fbotexcoordVBOhandle;
uint32 g_fboplaneIndexHandle;
uint32 g_fbotextureHandle;
uint32 g_fbodepthtexHandle;
uint32 g_fborenderbufferHandle;
uint32 g_fbovaoHandle;
int g_fboidxnum;


static GLint g_projectionMatrixLocation;
static GLint g_modelViewMatrixLocation;
static GLint g_normalMatrixLocation;
static GLint g_vertexLocation;
static GLint g_normalLocation;
static GLint g_texCoordLocation;
static GLint g_useTextureLocation;

static struct LightLocations g_light;
static struct MaterialLocations g_material;
static GLUSwavefront g_wavefront;
static GLfloat g_viewMatrix[16];

GLUSboolean init(GLUSvoid)
{
	g_renderbufManager = new OGLRenderBufferManager(&g_textureManager);
	// 白色光源
	struct LightProperties light = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
	GLfloat points[16 * 16 * 2] = {};


	GLUStextfile vertexSource;
	GLUStextfile fragmentSource;

	GLUStgaimage image;

	GLUSgroupList* groupWalker;
	GLUSmaterialList* materialWalker;

	//GLUSprogram pro;
	//创建shader1
	glusFileLoadText("shader/phong_textured.vert.glsl", &vertexSource);
	glusFileLoadText("shader/phong_textured.frag.glsl", &fragmentSource);
	//glusProgramBuildFromSource(&pro.program, vertexSource.text, 0, 0, 0, vertexSource.text);
	g_shaderHandle = g_shaderManager.createShader(vertexSource.text, 0, 0, 0, fragmentSource.text);
	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);
	glusFileLoadText("shader/particle.vert.glsl", &vertexSource);
	glusFileLoadText("shader/particle.frag.glsl", &fragmentSource);
	g_fboshaderHandle = g_shaderManager.createShader(vertexSource.text, 0, 0, 0, fragmentSource.text);
	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);
	g_shaderManager.setCurrentShader(g_fboshaderHandle);
	g_fboprojectionMatrixLocation = g_shaderManager.getShaderUniformLoc(g_fboshaderHandle, "u_projectionMatrix");;
	//g_fbomodelViewMatrixLocation = g_shaderManager.getShaderUniformLoc(g_fboshaderHandle, "u_modelViewMatrix");;
	//g_fbonormalMatrixLocation = g_shaderManager.getShaderUniformLoc(g_fboshaderHandle, "u_normalMatrix");;
	g_fbotextureLocation = g_shaderManager.getShaderSamplerLoc(g_fboshaderHandle, "u_texture");
	GLUSshape plane;
	glusShapeCreateRectangularPlanef(&plane, (GLfloat)200.0f, (GLfloat)200.0f); g_fboidxnum = plane.numberIndices;
	g_fbovertexVBOHandle = g_bufferManager.createVertexBuffer(plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)plane.vertices, GL_STATIC_DRAW);	
	g_fbotexcoordVBOhandle = g_bufferManager.createVertexBuffer(plane.numberVertices * 2 * sizeof(GLfloat), (GLfloat*)plane.texCoords, GL_STATIC_DRAW);
	g_fboplaneIndexHandle = g_bufferManager.createIndexBuffer(plane.numberIndices * sizeof(GLuint), (GLuint*)plane.indices, GL_STATIC_DRAW);
	VertexAttribLayoutList fboattributelist;	fboattributelist.numAttribs = 3;
	fboattributelist.attribs[0].locationName = {"a_vertex"};
	fboattributelist.attribs[0].unitSize = 4;
	fboattributelist.attribs[0].vboHandle = g_fbovertexVBOHandle;
	fboattributelist.attribs[0].vboType = GL_ARRAY_BUFFER;
	fboattributelist.attribs[1].locationName = { "a_texcoord" };
	fboattributelist.attribs[1].unitSize = 2;
	fboattributelist.attribs[1].vboHandle = g_fbotexcoordVBOhandle;
	fboattributelist.attribs[1].vboType = GL_ARRAY_BUFFER;
	fboattributelist.attribs[2].vboHandle = g_fboplaneIndexHandle;
	fboattributelist.attribs[2].vboType = GL_ELEMENT_ARRAY_BUFFER;
	g_shaderManager.setPragramAttribLoc(g_fboshaderHandle, fboattributelist);
	g_fbovaoHandle = g_bufferManager.createVAO(fboattributelist);
	
	//
	g_projectionMatrixLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_projectionMatrix");
	g_modelViewMatrixLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_modelViewMatrix");
	g_normalMatrixLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_normalMatrix");


	g_light.directionLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_light.direction");
	g_light.ambientColorLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_light.ambientColor");
	g_light.diffuseColorLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_light.diffuseColor");
	g_light.specularColorLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_light.specularColor");

	g_material.ambientColorLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_material.ambientColor");
	g_material.diffuseColorLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_material.diffuseColor");
	g_material.specularColorLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_material.specularColor");
	g_material.specularExponentLocation = g_shaderManager.getShaderUniformLoc(g_shaderHandle, "u_material.specularExponent"); 
	g_material.diffuseTextureLocation = g_shaderManager.getShaderSamplerLoc(g_shaderHandle, "u_material.diffuseTexture"); 

	g_useTextureLocation = g_shaderManager.getShaderSamplerLoc(g_shaderHandle, "u_useTexture");

	VertexAttribLayoutList attributelist;
	attributelist.attribs[0].locationName = { "a_vertex"};
	attributelist.attribs[1].locationName = { "a_normal" };
	attributelist.attribs[2].locationName = { "a_texCoord" };
	attributelist.numAttribs = 4;
	g_shaderManager.setPragramAttribLoc(g_shaderHandle, attributelist);

	//g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
	//g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");
	//g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");

	glusWavefrontLoad("ChessKing.obj", &g_wavefront);
	g_wavefront.verticesVBO = g_bufferManager.createVertexBuffer(g_wavefront.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)g_wavefront.vertices, GL_STATIC_DRAW);
	g_wavefront.normalsVBO = g_bufferManager.createVertexBuffer(g_wavefront.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)g_wavefront.normals, GL_STATIC_DRAW);
	g_wavefront.texCoordsVBO = g_bufferManager.createVertexBuffer(g_wavefront.numberVertices * 2 * sizeof(GLfloat), (GLfloat*)g_wavefront.texCoords, GL_STATIC_DRAW);
	g_bufferManager.setCurrentVertexBuffer(0);
	/*glGenBuffers(1, &g_wavefront.verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, g_wavefront.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)g_wavefront.vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &g_wavefront.normalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.normalsVBO);
	glBufferData(GL_ARRAY_BUFFER, g_wavefront.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)g_wavefront.normals, GL_STATIC_DRAW);

	glGenBuffers(1, &g_wavefront.texCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.texCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, g_wavefront.numberVertices * 2 * sizeof(GLfloat), (GLfloat*)g_wavefront.texCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);*/


	g_shaderManager.setCurrentShader(g_shaderHandle);

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
		groupWalker->group.indicesVBO = g_bufferManager.createIndexBuffer(groupWalker->group.numberIndices * sizeof(GLuint), (GLuint*)groupWalker->group.indices, GL_STATIC_DRAW);
		//glGenBuffers(1, &groupWalker->group.indicesVBO);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.numberIndices * sizeof(GLuint), (GLuint*)groupWalker->group.indices, GL_STATIC_DRAW);
		g_bufferManager.setCurrentIndexBuffer(0);

		attributelist.attribs[0].unitSize = 4;
		attributelist.attribs[0].vboHandle = g_wavefront.verticesVBO;
		attributelist.attribs[1].unitSize = 3;
		attributelist.attribs[1].vboHandle = g_wavefront.normalsVBO;
		attributelist.attribs[2].unitSize = 2;
		attributelist.attribs[2].vboHandle = g_wavefront.texCoordsVBO;
		attributelist.attribs[0].vboType = GL_ARRAY_BUFFER;
		attributelist.attribs[1].vboType = GL_ARRAY_BUFFER;
		attributelist.attribs[2].vboType = GL_ARRAY_BUFFER;
		attributelist.attribs[3].vboHandle = groupWalker->group.indicesVBO;
		attributelist.attribs[3].vboType = GL_ELEMENT_ARRAY_BUFFER;
		groupWalker->group.vao =  g_bufferManager.createVAO(attributelist);
		//glGenVertexArrays(1, &groupWalker->group.vao);
		//glBindVertexArray(groupWalker->group.vao);
		//glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.verticesVBO);
		//glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
		//glEnableVertexAttribArray(g_vertexLocation);
		//glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.normalsVBO);
		//glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//glEnableVertexAttribArray(g_normalLocation);
		//glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.texCoordsVBO);
		//glVertexAttribPointer(g_texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
		//glEnableVertexAttribArray(g_texCoordLocation);
		g_bufferManager.setCurrentIndexBuffer(0);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);
		//glBindVertexArray(0);

		groupWalker = groupWalker->next;
	}

	materialWalker = g_wavefront.materials;
	uint32 count = 0;
	while (materialWalker)
	{
		if (materialWalker->material.diffuseTextureFilename[0] != '\0')
		{
			glusImageLoadTga(materialWalker->material.diffuseTextureFilename, &image); 
			TextureFormats::List format;
			if (image.format == GL_RGBA)
			{
				format = TextureFormats::RGBA;
			}
			else
			{
				format = TextureFormats::Unknown;
			}
			materialWalker->material.diffuseTextureName = g_textureManager.createTexture(TextureType::Tex2D, image.width, image.height, 1, format, true, true, false, false);
			g_textureManager.uploadTextureData(materialWalker->material.diffuseTextureName, 0, 0, image.data); 
			//glActiveTexture(GL_TEXTURE0 + count);
			//count++;
			//g_textureManager.setCurrentTexture(materialWalker->material.diffuseTextureName);
			g_textureManager.addActiveTexture(count, materialWalker->material.diffuseTextureName, 0); count++;
			//g_textureManager.setCurrentTexture(0);
			//glGenTextures(1, &materialWalker->material.diffuseTextureName);
			//glBindTexture(GL_TEXTURE_2D, materialWalker->material.diffuseTextureName);
			//glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			//glGenerateMipmap(GL_TEXTURE_2D);
			//glBindTexture(GL_TEXTURE_2D, 0);
		}

		materialWalker = materialWalker->next;
	}
	g_textureManager.bindAllActiveTex();

	glusMatrix4x4LookAtf(g_viewMatrix, 0.0f, 0.75f, 3.0f, 0.0f, 0.75f, 0.0f, 0.0f, 1.0f, 0.0f);
	glusVector3Normalizef(light.direction);
	glusMatrix4x4MultiplyVector3f(light.direction, g_viewMatrix, light.direction);

	g_shaderManager.setShaderUniform(g_light.directionLocation, UNIFORM_FLOAT3, light.direction);
	g_shaderManager.setShaderUniform(g_light.ambientColorLocation, UNIFORM_FLOAT4, light.ambientColor);
	g_shaderManager.setShaderUniform(g_light.diffuseColorLocation, UNIFORM_FLOAT4, light.diffuseColor);
	g_shaderManager.setShaderUniform(g_light.specularColorLocation, UNIFORM_FLOAT4, light.specularColor);
	//glUniform3fv(g_light.directionLocation, 1, light.direction);
	//glUniform4fv(g_light.ambientColorLocation, 1, light.ambientColor);
	//glUniform4fv(g_light.diffuseColorLocation, 1, light.diffuseColor);
	//glUniform4fv(g_light.specularColorLocation, 1, light.specularColor);

	g_fborenderbufferHandle = g_renderbufManager->createRenderBuffer(512, 512, TextureFormats::RGBA8, true, 1, 0);
	g_fbotextureHandle = g_renderbufManager->getRenderBufferTex(g_fborenderbufferHandle, 0);
	g_fbodepthtexHandle = g_renderbufManager->getRenderBufferTex(g_fborenderbufferHandle, 32);


	

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	GLfloat projectionMatrix[16];

	glViewport(0, 0, width, height);
	g_shaderManager.setCurrentShader(g_shaderHandle);
	glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat)width / (GLfloat)height, 1.0f, 100.0f);
	g_shaderManager.setShaderUniform(g_projectionMatrixLocation, UNIFORM_FLOAT44, projectionMatrix);
	//glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);

	//fbo平面显示变换
	g_shaderManager.setCurrentShader(g_fboshaderHandle);
	GLfloat viewMatrix[16];
	GLfloat modelViewProjectionMatrix[16];
	glusMatrix4x4LookAtf(viewMatrix, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	glusMatrix4x4Orthof(modelViewProjectionMatrix, -(GLfloat)width / 2.0f, (GLfloat)width / 2.0f, -(GLfloat)height / 2.0f, (GLfloat)height / 2.0f, 1.0f, 100.0f);
	glusMatrix4x4Multiplyf(modelViewProjectionMatrix, modelViewProjectionMatrix, viewMatrix);
	g_shaderManager.setShaderUniform(g_fboprojectionMatrixLocation, UNIFORM_FLOAT44, modelViewProjectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
	static GLfloat angle = 0.0f;

	GLfloat modelViewMatrix[16];
	GLfloat normalMatrix[9];
	GLUSgroupList* groupWalker;
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_renderbufManager->setCurRenderBuffer(0);
	g_shaderManager.setCurrentShader(g_fboshaderHandle);
	g_fbotextureHandle = g_renderbufManager->getRenderBufferTex(g_fborenderbufferHandle, 0);
	g_renderbufManager->setCurRenderTexture(g_fbotextureHandle);
	
	g_shaderManager.setShaderSampler(g_fbotextureLocation, 15);

	g_bufferManager.setCurrentVAO(g_fbovaoHandle);
	glDrawElements(GL_TRIANGLES, g_fboidxnum, GL_UNSIGNED_INT, 0);
	//模型绘制到fbotexture
	
	g_renderbufManager->setCurRenderBuffer(g_fborenderbufferHandle);
	glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_shaderManager.setCurrentShader(g_shaderHandle);
	glusMatrix4x4Identityf(modelViewMatrix);
	glusMatrix4x4RotateRyf(modelViewMatrix, angle);
	glusMatrix4x4Scalef(modelViewMatrix, 10.0f, 10.0f, 10.0f);
	glusMatrix4x4Multiplyf(modelViewMatrix, g_viewMatrix, modelViewMatrix);
	glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

	g_shaderManager.setShaderUniform(g_modelViewMatrixLocation, UNIFORM_FLOAT44, modelViewMatrix);
	g_shaderManager.setShaderUniform(g_normalMatrixLocation, UNIFORM_FLOAT33, normalMatrix);
	//glUniformMatrix4fv(g_modelViewMatrixLocation, 1, GL_FALSE, modelViewMatrix);
	//glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);
	
	
	
	uint32 count = 0;
	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
		g_shaderManager.setShaderUniform(g_material.ambientColorLocation, UNIFORM_FLOAT4, groupWalker->group.material->ambient);
		g_shaderManager.setShaderUniform(g_material.diffuseColorLocation, UNIFORM_FLOAT4, groupWalker->group.material->diffuse);
		g_shaderManager.setShaderUniform(g_material.specularColorLocation, UNIFORM_FLOAT4, groupWalker->group.material->specular);
		g_shaderManager.setShaderUniform(g_material.specularExponentLocation, UNIFORM_FLOAT, &groupWalker->group.material->shininess);
		//glUniform4fv(g_material.ambientColorLocation, 1, groupWalker->group.material->ambient);
		//glUniform4fv(g_material.diffuseColorLocation, 1, groupWalker->group.material->diffuse);
		//glUniform4fv(g_material.specularColorLocation, 1, groupWalker->group.material->specular);
		//glUniform1f(g_material.specularExponentLocation, groupWalker->group.material->shininess);

		//glActiveTexture(GL_TEXTURE0 + count);
		//g_textureManager.setCurrentTexture(groupWalker->group.material->diffuseTextureName);

		if (groupWalker->group.material->diffuseTextureName)
		{
			g_shaderManager.setShaderSampler(g_useTextureLocation, 10);
			g_shaderManager.setShaderSampler(g_material.diffuseTextureLocation, count);
			/*glUniform1i(g_useTextureLocation, 1);
			glUniform1i(g_material.diffuseTextureLocation, 0);*/
			//glBindTexture(GL_TEXTURE_2D, groupWalker->group.material->diffuseTextureName);
			//g_textureManager.setCurrentTexture(groupWalker->group.material->diffuseTextureName);
		}count++;
		//else
		//{
		//	g_shaderManager.setShaderSampler(g_useTextureLocation, 0);
		//	g_shaderManager.setShaderSampler(g_material.diffuseTextureLocation, 0);
		//	//glUniform1i(g_useTextureLocation, 0);
		//	//glUniform1i(g_material.diffuseTextureLocation, 0);
		//	glBindTexture(GL_TEXTURE_2D, 0);
		//}

		//glBindVertexArray(groupWalker->group.vao);
		g_bufferManager.setCurrentVAO(groupWalker->group.vao);
		glDrawElements(GL_TRIANGLES, groupWalker->group.numberIndices, GL_UNSIGNED_INT, 0);

		groupWalker = groupWalker->next;
	}
	g_renderbufManager->setCurRenderBuffer(0);


	angle += 30.0f * time;

	return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
	/*GLUSgroupList* groupWalker;
	GLUSmaterialList* materialWalker;

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (g_wavefront.verticesVBO)
	{
	glDeleteBuffers(1, &g_wavefront.verticesVBO);

	g_wavefront.verticesVBO = 0;
	}

	if (g_wavefront.normalsVBO)
	{
	glDeleteBuffers(1, &g_wavefront.normalsVBO);

	g_wavefront.normalsVBO = 0;
	}

	if (g_wavefront.texCoordsVBO)
	{
	glDeleteBuffers(1, &g_wavefront.texCoordsVBO);

	g_wavefront.texCoordsVBO = 0;
	}

	glBindVertexArray(0);

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
	if (groupWalker->group.indicesVBO)
	{
	glDeleteBuffers(1, &groupWalker->group.indicesVBO);

	groupWalker->group.indicesVBO = 0;
	}

	if (groupWalker->group.vao)
	{
	glDeleteVertexArrays(1, &groupWalker->group.vao);

	groupWalker->group.vao = 0;
	}

	groupWalker = groupWalker->next;
	}

	materialWalker = g_wavefront.materials;
	while (materialWalker)
	{
	if (materialWalker->material.diffuseTextureName)
	{
	glDeleteTextures(1, &materialWalker->material.diffuseTextureName);

	materialWalker->material.diffuseTextureName = 0;
	}

	materialWalker = materialWalker->next;
	}

	glUseProgram(0);

	glusProgramDestroy(&g_program);

	glusWavefrontDestroy(&g_wavefront);*/
}

int main(int argc, char* argv[])
{
	EGLint eglConfigAttributes[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 0,
		EGL_SAMPLE_BUFFERS, 1,
		EGL_SAMPLES, 8,
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