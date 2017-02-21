#include "LeanUtil.h"
#include "OGLDeviceManager.h"
#include <map>
using namespace std;
using namespace Lean3D;


static GLUSfloat g_cameraPosition[3] = { -5.0f, 5.0f, 10.0f };

static GLUSfloat g_lightDirection[3] = { 0.0f, 0.0f, 10.0f };

static GLUSprogram g_program;

static GLUSprogram g_programShadowVolume;

static GLUSprogram g_programShadowPlane;

//

static GLint g_projectionMatrixLocation;

static GLint g_viewMatrixLocation;

static GLint g_modelMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_colorLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

//

static GLint g_projectionMatrixShadowVolumeLocation;

static GLint g_viewMatrixShadowVolumeLocation;

static GLint g_modelMatrixShadowVolumeLocation;

static GLint g_lightDirectionShadowVolumeLocation;

static GLint g_vertexShadowVolumeLocation;

//

static GLint g_vertexShadowPlaneLocation;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_indicesVBO;

static GLuint g_verticesPlaneVBO;

static GLuint g_normalsPlaneVBO;

static GLuint g_indicesPlaneVBO;

static GLuint g_verticesShadowPlaneVBO;

static GLuint g_indicesShadowPlaneVBO;

static GLuint g_vao;

static GLuint g_vaoShadowVolume;

static GLuint g_vaoPlane;

static GLuint g_vaoShadowPlane;

//

static GLuint g_numberIndices;

static GLuint g_numberIndicesPlane;

static GLuint g_numberIndicesShadowPlane;

OGLDeviceManager g_diviceMana;
uint32 g_colorShader;
uint32 g_shadowShader;
uint32 g_volumeShader;
uint32 g_colorVao;
uint32 g_shadowVao;
uint32 g_volumeVao;
uint32 g_planeVao;
map<string, int> g_colorUniform;
map<string, int> g_shadowUniform;
map<string, int> g_volumeUniform;

class Camera
{
public:
	Matrix4 view;
	Camera();
	~Camera();

	void front(float n)
	{
		view.c[3][2] += n;
	}
	void back(float n)
	{
		view.c[3][2] -= n;
	}
	void left(float n)
	{
		view.c[3][0] -= n;
	}
	void right(float n)
	{
		view.c[3][0] += n;
	}
	void pitch(float n)
	{
		view.rotate(0.0f, n, 0.0f);
	}
	void yaw(float n)
	{
		view.rotate(0.0f, 0.0f, n);
	}

};

Camera::Camera()
{
	
}

Camera::~Camera()
{
}

Camera g_camera;

GLUSboolean init(GLUSvoid)
{
	glusMatrix4x4Identityf((float*)g_camera.view.c);
	GLUSshape shadowPlane;

	GLUSshape plane;

	GLUSshape torus, torusWithAdjacency;

	GLUStextfile vertexSource;
	GLUStextfile geometrySource;
	GLUStextfile fragmentSource;

	GLfloat viewMatrix[16];

	GLfloat lightDirection[3];

	lightDirection[0] = g_lightDirection[0];
	lightDirection[1] = g_lightDirection[1];
	lightDirection[2] = g_lightDirection[2];

	glusVector3Normalizef(lightDirection);

	//

	glusFileLoadText("shader/color.vert.glsl", &vertexSource);
	glusFileLoadText("shader/color.frag.glsl", &fragmentSource);
	//glusProgramBuildFromSource(&g_program, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);
	g_colorShader = g_diviceMana.shaderManaRef()->createShader(vertexSource.text, 0, 0, 0, fragmentSource.text);
	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//
	
	g_projectionMatrixLocation = g_diviceMana.shaderManaRef()->getShaderUniformLoc(g_colorShader, "u_projectionMatrix");
	g_viewMatrixLocation = g_diviceMana.shaderManaRef()->getShaderUniformLoc(g_colorShader, "u_viewMatrix");
	g_modelMatrixLocation = g_diviceMana.shaderManaRef()->getShaderUniformLoc(g_colorShader, "u_modelMatrix");
	g_normalMatrixLocation = g_diviceMana.shaderManaRef()->getShaderUniformLoc(g_colorShader, "u_normalMatrix");
	g_colorLocation = g_diviceMana.shaderManaRef()->getShaderUniformLoc(g_colorShader, "u_shapeColor");
	g_lightDirectionLocation = g_diviceMana.shaderManaRef()->getShaderUniformLoc(g_colorShader, "u_lightDirection");
	
	VertexAttribLayoutList attriblist1;
	attriblist1.numAttribs = 2;
	attriblist1.attribs[0].locationName = "a_vertex";
	attriblist1.attribs[1].locationName = "a_normal";
	//attriblist.attribs[0].unitSize = 4;
	//attriblist.attribs[1].unitSize = 3;
	//attriblist.attribs[0].vboHandle = g_verticesVBO;
	//attriblist.attribs[1].vboHandle = g_normalsVBO;
	//attriblist.attribs[0].vboType = GL_ARRAY_BUFFER;
	//attriblist.attribs[1].vboType = GL_ARRAY_BUFFER;
	//attriblist.attribs[2].vboType = GL_ELEMENT_ARRAY_BUFFER;
	//attriblist.attribs[2].vboHandle = g_indicesVBO;
	g_diviceMana.shaderManaRef()->setPragramAttribLoc(g_colorShader, attriblist1);

	/*	g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
		g_viewMatrixLocation = glGetUniformLocation(g_program.program, "u_viewMatrix");
		g_modelMatrixLocation = glGetUniformLocation(g_program.program, "u_modelMatrix");
		g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
		g_colorLocation = glGetUniformLocation(g_program.program, "u_shapeColor");
		g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");*/

	//g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
	//g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

	//

	glusFileLoadText("shader/shadowvolume.vert.glsl", &vertexSource);
	glusFileLoadText("shader/shadowvolume.geom.glsl", &geometrySource);
	glusFileLoadText("shader/shadowvolume.frag.glsl", &fragmentSource);
	//glusProgramBuildFromSource(&g_programShadowVolume, (const GLUSchar**)&vertexSource.text, 0, 0, (const GLUSchar**)&geometrySource.text, (const GLUSchar**)&fragmentSource.text);
	g_volumeShader = g_diviceMana.shaderManaRef()->createShader(vertexSource.text, 0,0,geometrySource.text, fragmentSource.text);
	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&geometrySource);
	glusFileDestroyText(&fragmentSource);

	//
	g_projectionMatrixShadowVolumeLocation = g_diviceMana.shaderManaRef()->getShaderUniformLoc(g_volumeShader, "u_projectionMatrix");
	g_viewMatrixShadowVolumeLocation = g_diviceMana.shaderManaRef()->getShaderUniformLoc(g_volumeShader, "u_viewMatrix");
	g_modelMatrixShadowVolumeLocation = g_diviceMana.shaderManaRef()->getShaderUniformLoc(g_volumeShader, "u_modelMatrix");
	g_lightDirectionShadowVolumeLocation = g_diviceMana.shaderManaRef()->getShaderUniformLoc(g_volumeShader, "u_lightDirection");
	//g_projectionMatrixShadowVolumeLocation = glGetUniformLocation(g_programShadowVolume.program, "u_projectionMatrix");
	//g_viewMatrixShadowVolumeLocation = glGetUniformLocation(g_programShadowVolume.program, "u_viewMatrix");
	//g_modelMatrixShadowVolumeLocation = glGetUniformLocation(g_programShadowVolume.program, "u_modelMatrix");
	//g_lightDirectionShadowVolumeLocation = glGetUniformLocation(g_programShadowVolume.program, "u_lightDirection");
	VertexAttribLayoutList attriblist2;
	attriblist2.numAttribs = 1;
	attriblist2.attribs[0].locationName = "a_vertex";
	//attriblist2.attribs[0].unitSize = 4;
	//attriblist2.attribs[0].vboHandle = g_verticesVBO;
	//attriblist2.attribs[0].vboType = GL_ARRAY_BUFFER;
	//attriblist2.attribs[1].vboHandle = g_indicesVBO;
	//attriblist2.attribs[1].vboType = GL_ELEMENT_ARRAY_BUFFER;
	g_diviceMana.shaderManaRef()->setPragramAttribLoc(g_volumeShader, attriblist2);
	//g_vertexShadowVolumeLocation = glGetAttribLocation(g_programShadowVolume.program, "a_vertex");

	//

	glusFileLoadText("shader/shadow.vert.glsl", &vertexSource);
	glusFileLoadText("shader/shadow.frag.glsl", &fragmentSource);
	//glusProgramBuildFromSource(&g_programShadowPlane, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);
	g_shadowShader = g_diviceMana.shaderManaRef()->createShader(vertexSource.text, 0, 0, 0, fragmentSource.text);
	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//
	VertexAttribLayoutList attriblist3;
	attriblist3.numAttribs = 1;
	attriblist3.attribs[0].locationName = "a_vertex";
	//attriblist2.attribs[0].unitSize = 4;
	//attriblist2.attribs[0].vboHandle = g_verticesVBO;
	//attriblist2.attribs[0].vboType = GL_ARRAY_BUFFER;
	//attriblist2.attribs[1].vboHandle = g_indicesVBO;
	//attriblist2.attribs[1].vboType = GL_ELEMENT_ARRAY_BUFFER;
	g_diviceMana.shaderManaRef()->setPragramAttribLoc(g_shadowShader, attriblist3);
	//g_vertexShadowPlaneLocation = glGetAttribLocation(g_programShadowPlane.program, "a_vertex");

	//

	glusShapeCreateTorusf(&torus, 0.5f, 1.0f, 32, 32);
	glusShapeCreateAdjacencyIndicesf(&torusWithAdjacency, &torus);
	glusShapeDestroyf(&torus);

	g_numberIndices = torusWithAdjacency.numberIndices;

	g_verticesVBO = g_diviceMana.buffManaRef()->createVertexBuffer(torusWithAdjacency.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)torusWithAdjacency.vertices, GL_STATIC_DRAW);
	g_normalsVBO = g_diviceMana.buffManaRef()->createVertexBuffer(torusWithAdjacency.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)torusWithAdjacency.normals, GL_STATIC_DRAW);
	g_indicesVBO = g_diviceMana.buffManaRef()->createIndexBuffer(torusWithAdjacency.numberIndices * sizeof(GLuint), (GLuint*)torusWithAdjacency.indices, GL_STATIC_DRAW);
	//glGenBuffers(1, &g_verticesVBO);
	//glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
	//glBufferData(GL_ARRAY_BUFFER, torusWithAdjacency.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)torusWithAdjacency.vertices, GL_STATIC_DRAW);
	//glGenBuffers(1, &g_normalsVBO);
	//glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
	//glBufferData(GL_ARRAY_BUFFER, torusWithAdjacency.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)torusWithAdjacency.normals, GL_STATIC_DRAW)
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glGenBuffers(1, &g_indicesVBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, torusWithAdjacency.numberIndices * sizeof(GLuint), (GLuint*)torusWithAdjacency.indices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glusShapeDestroyf(&torusWithAdjacency);
	//
	glusShapeCreatePlanef(&plane, 10.0f);
	g_numberIndicesPlane = plane.numberIndices;
	g_verticesPlaneVBO = g_diviceMana.buffManaRef()->createVertexBuffer(plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)plane.vertices, GL_STATIC_DRAW);
	g_normalsPlaneVBO = g_diviceMana.buffManaRef()->createVertexBuffer(plane.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)plane.normals, GL_STATIC_DRAW);
	g_indicesPlaneVBO = g_diviceMana.buffManaRef()->createIndexBuffer(plane.numberIndices * sizeof(GLuint), (GLuint*)plane.indices, GL_STATIC_DRAW);
	//glGenBuffers(1, &g_verticesPlaneVBO);
	//glBindBuffer(GL_ARRAY_BUFFER, g_verticesPlaneVBO);
	//glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)plane.vertices, GL_STATIC_DRAW);
	//glGenBuffers(1, &g_normalsPlaneVBO);
	//glBindBuffer(GL_ARRAY_BUFFER, g_normalsPlaneVBO);
	//glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)plane.normals, GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glGenBuffers(1, &g_indicesPlaneVBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesPlaneVBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, plane.numberIndices * sizeof(GLuint), (GLuint*)plane.indices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glusShapeDestroyf(&plane);
	//
	// The plane extends from -1.0 to 1.0 for both sides. So when rendering in NDC, the plane is always fullscreen.
	glusShapeCreatePlanef(&shadowPlane, 1.0f);
	g_numberIndicesShadowPlane = shadowPlane.numberIndices;
	g_verticesShadowPlaneVBO = g_diviceMana.buffManaRef()->createVertexBuffer(shadowPlane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)shadowPlane.vertices, GL_STATIC_DRAW);
	g_indicesShadowPlaneVBO = g_diviceMana.buffManaRef()->createIndexBuffer(shadowPlane.numberIndices * sizeof(GLuint), (GLuint*)shadowPlane.indices, GL_STATIC_DRAW);
	//glGenBuffers(1, &g_verticesShadowPlaneVBO);
	//glBindBuffer(GL_ARRAY_BUFFER, g_verticesShadowPlaneVBO);
	//glBufferData(GL_ARRAY_BUFFER, shadowPlane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)shadowPlane.vertices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glGenBuffers(1, &g_indicesShadowPlaneVBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesShadowPlaneVBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, shadowPlane.numberIndices * sizeof(GLuint), (GLuint*)shadowPlane.indices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glusShapeDestroyf(&shadowPlane);

	//


	glusMatrix4x4LookAtf(viewMatrix, g_cameraPosition[0], g_cameraPosition[1], g_cameraPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	//将光源转换到相机空间
	glusMatrix4x4MultiplyVector3f(lightDirection, viewMatrix, lightDirection);

	g_diviceMana.shaderManaRef()->setCurrentShader(g_colorShader);
	g_diviceMana.shaderManaRef()->setShaderUniform(g_lightDirectionLocation, UNIFORM_FLOAT3, lightDirection);

	g_diviceMana.shaderManaRef()->setCurrentShader(g_volumeShader);
	g_diviceMana.shaderManaRef()->setShaderUniform(g_lightDirectionShadowVolumeLocation, UNIFORM_FLOAT3, lightDirection);

	//glUseProgram(g_program.program);
	//glUniform3fv(g_lightDirectionLocation, 1, lightDirection);
	//glUseProgram(g_programShadowVolume.program);
	//glUniform3fv(g_lightDirectionShadowVolumeLocation, 1, lightDirection);

	++attriblist1.numAttribs;
	attriblist1.attribs[0].unitSize = 4;
	attriblist1.attribs[1].unitSize = 3;
	attriblist1.attribs[0].vboHandle = g_verticesVBO;
	attriblist1.attribs[1].vboHandle = g_normalsVBO;
	attriblist1.attribs[0].vboType = GL_ARRAY_BUFFER;
	attriblist1.attribs[1].vboType = GL_ARRAY_BUFFER;
	attriblist1.attribs[2].vboType = GL_ELEMENT_ARRAY_BUFFER;
	attriblist1.attribs[2].vboHandle = g_indicesVBO;
	g_colorVao = g_diviceMana.buffManaRef()->createVAO(attriblist1);
	//glGenVertexArrays(1, &g_vao);
	//glBindVertexArray(g_vao);
	//glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
	//glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(g_vertexLocation);
	//glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
	//glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(g_normalLocation);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

	// Shadow Volume
	++attriblist2.numAttribs;
	attriblist2.attribs[0].unitSize = 4;
	attriblist2.attribs[0].vboHandle = g_verticesVBO;
	attriblist2.attribs[0].vboType = GL_ARRAY_BUFFER;
	attriblist2.attribs[1].vboHandle = g_indicesVBO;
	attriblist2.attribs[1].vboType = GL_ELEMENT_ARRAY_BUFFER;
	g_volumeVao = g_diviceMana.buffManaRef()->createVAO(attriblist2);
	//glGenVertexArrays(1, &g_vaoShadowVolume);
	//glBindVertexArray(g_vaoShadowVolume);
	//glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
	//glVertexAttribPointer(g_vertexShadowVolumeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(g_vertexShadowVolumeLocation);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

	// Plane

	
	attriblist1.attribs[0].unitSize = 4;
	attriblist1.attribs[0].vboHandle = g_verticesPlaneVBO;
	attriblist1.attribs[0].vboType = GL_ARRAY_BUFFER;
	attriblist1.attribs[1].unitSize = 3;
	attriblist1.attribs[1].vboHandle = g_normalsPlaneVBO;
	attriblist1.attribs[1].vboType = GL_ARRAY_BUFFER;
	attriblist1.attribs[2].vboHandle = g_indicesPlaneVBO;
	attriblist1.attribs[2].vboType = GL_ELEMENT_ARRAY_BUFFER;
	g_planeVao =  g_diviceMana.buffManaRef()->createVAO(attriblist1);


	//glGenVertexArrays(1, &g_vaoPlane);
	//glBindVertexArray(g_vaoPlane);
	//glBindBuffer(GL_ARRAY_BUFFER, g_verticesPlaneVBO);
	//glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(g_vertexLocation);
	//glBindBuffer(GL_ARRAY_BUFFER, g_normalsPlaneVBO);
	//glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(g_normalLocation);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesPlaneVBO);

	// Shadow Plane

	++attriblist3.numAttribs;
	attriblist3.attribs[0].unitSize = 4;
	attriblist3.attribs[0].vboHandle = g_verticesShadowPlaneVBO;
	attriblist3.attribs[0].vboType = GL_ARRAY_BUFFER;
	attriblist3.attribs[1].vboHandle = g_indicesShadowPlaneVBO;
	attriblist3.attribs[1].vboType = GL_ELEMENT_ARRAY_BUFFER;
	g_shadowVao = g_diviceMana.buffManaRef()->createVAO(attriblist3);
	//glGenVertexArrays(1, &g_vaoShadowPlane);
	//glBindVertexArray(g_vaoShadowPlane);
	//glBindBuffer(GL_ARRAY_BUFFER, g_verticesShadowPlaneVBO);
	//glVertexAttribPointer(g_vertexShadowPlaneLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(g_vertexShadowPlaneLocation);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesShadowPlaneVBO);

	//
	
	g_diviceMana.clearColor(0, 0, 0, 0);
	g_diviceMana.clearDepth(1.0f);
	g_diviceMana.clearStencil(0);
	g_diviceMana.setBlendMode(true, BS_BLEND_SRC_ALPHA, BS_BLEND_INV_SRC_ALPHA);
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClearDepth(1.0f);
	//glClearStencil(0);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonOffset(0.0f, 100.0f);

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	GLfloat projectionMatrix[16];

	//glViewport(0, 0, width, height);
	g_diviceMana.setViewport(0, 0, width, height);

	glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat)width / (GLfloat)height, 1.0f, 100.0f);

	g_diviceMana.shaderManaRef()->setCurrentShader(g_colorShader);
	g_diviceMana.shaderManaRef()->setShaderUniform(g_projectionMatrixLocation, UNIFORM_FLOAT44, projectionMatrix);

	g_diviceMana.shaderManaRef()->setCurrentShader(g_volumeShader);
	g_diviceMana.shaderManaRef()->setShaderUniform(g_projectionMatrixShadowVolumeLocation, UNIFORM_FLOAT44, projectionMatrix);
	//glUseProgram(g_program.program);
	//glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);
	//glUseProgram(g_programShadowVolume.program);
	//glUniformMatrix4fv(g_projectionMatrixShadowVolumeLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
	static GLfloat angle = 0.0f;

	GLfloat modelViewMatrix[16];
	GLfloat viewMatrix[16];
	GLfloat modelMatrix[16];
	GLfloat normalMatrix[9];

	g_diviceMana.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	g_diviceMana.shaderManaRef()->setCurrentShader(g_colorShader);
	glusMatrix4x4LookAtf(viewMatrix, g_cameraPosition[0], g_cameraPosition[1], g_cameraPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	/////******************camra Test*******
	
	g_camera.view.c[0][0] = 0.0f;
	g_camera.view.c[1][0] = 1.0f;
	g_camera.view.c[2][0] = 0.0f;
	g_camera.view.translate(5.0f, 5.0f, 10.0f);
	////////********************************
	g_diviceMana.shaderManaRef()->setShaderUniform(g_viewMatrixLocation, UNIFORM_FLOAT44, g_camera.view.c);

	//glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	g_diviceMana.setDepthTest(true);

	glusMatrix4x4Identityf(modelMatrix);
	glusMatrix4x4Translatef(modelMatrix, 0.0f, 0.0f, -5.0f);
	glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
	glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

	g_diviceMana.shaderManaRef()->setShaderUniform(g_modelMatrixLocation, UNIFORM_FLOAT44, modelMatrix);
	g_diviceMana.shaderManaRef()->setShaderUniform(g_normalMatrixLocation, UNIFORM_FLOAT33, normalMatrix);
	float color[] = { 0.0f, 0.5f, 0.0f, 1.0f };
	g_diviceMana.shaderManaRef()->setShaderUniform(g_colorLocation, UNIFORM_FLOAT4, color);
//	g_diviceMana.buffManaRef()->setCurrentVAO(g_planeVao);
	g_diviceMana.setVAO(g_planeVao);
	g_diviceMana.drawIndexed(GL_TRIANGLES, g_numberIndicesPlane, GL_UNSIGNED_INT);

	glusMatrix4x4Identityf(modelMatrix);
	glusMatrix4x4RotateRzRxRyf(modelMatrix, 0.0f, 0.0f, angle);
	glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
	glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

	g_diviceMana.shaderManaRef()->setShaderUniform(g_modelMatrixLocation, UNIFORM_FLOAT44, modelMatrix);
	g_diviceMana.shaderManaRef()->setShaderUniform(g_normalMatrixLocation, UNIFORM_FLOAT33, normalMatrix);
	color[0] = 0.33f;
	color[1] = 0.0f;
	color[2] = 0.5f;
	color[3] = 1.0f;
	g_diviceMana.shaderManaRef()->setShaderUniform(g_colorLocation, UNIFORM_FLOAT4, color);
	//g_diviceMana.buffManaRef()->setCurrentVAO(g_colorVao);
	g_diviceMana.setVAO(g_colorVao);
	g_diviceMana.drawIndexed(GL_TRIANGLES_ADJACENCY, g_numberIndices, GL_UNSIGNED_INT);

	g_diviceMana.setStencilTest(true);
	//glEnable(GL_STENCIL_TEST);

	g_diviceMana.shaderManaRef()->setCurrentShader(g_volumeShader);
	g_diviceMana.shaderManaRef()->setShaderUniform(g_viewMatrixShadowVolumeLocation, UNIFORM_FLOAT44, viewMatrix);
	g_diviceMana.shaderManaRef()->setShaderUniform(g_modelMatrixShadowVolumeLocation, UNIFORM_FLOAT44, modelMatrix);

	// 只渲染模板缓冲
	g_diviceMana.setColorWriteMask(false);
	g_diviceMana.setDepthMask(false);
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glDepthMask(GL_FALSE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_DEPTH_CLAMP);
	//g_diviceMana.buffManaRef()->setCurrentVAO(g_volumeVao);
	g_diviceMana.setVAO(g_volumeVao);
	g_diviceMana.setCullMode(RS_CULL_FRONT);
	g_diviceMana.setStencilFunc(true, SS_ALWAYS, 0x0, 0xff);
	g_diviceMana.setStencilOp(true, SOP_KEEP, SOP_INCR, SOP_KEEP);
	//glCullFace(GL_FRONT);
	//glStencilFunc(GL_ALWAYS, 0x0, 0xff);
	//glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
	g_diviceMana.drawIndexed(GL_TRIANGLES_ADJACENCY, g_numberIndices, GL_UNSIGNED_INT);

	g_diviceMana.setCullMode(RS_CULL_BACK);
	g_diviceMana.setStencilFunc(true, SS_ALWAYS, 0x0, 0xff);
	g_diviceMana.setStencilOp(true, SOP_KEEP, SOP_DECR, SOP_KEEP);
	//glCullFace(GL_BACK);
	//glStencilFunc(GL_ALWAYS, 0x0, 0xff);
	//glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
	g_diviceMana.drawIndexed(GL_TRIANGLES_ADJACENCY, g_numberIndices, GL_UNSIGNED_INT);


	g_diviceMana.setColorWriteMask(true);
	g_diviceMana.setDepthMask(true);
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//glDepthMask(GL_TRUE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_DEPTH_CLAMP);

	g_diviceMana.shaderManaRef()->setCurrentShader(g_shadowShader);

	g_diviceMana.setDepthTest(false);
	g_diviceMana.setBlendMode(true,BS_BLEND_SRC_ALPHA, BS_BLEND_INV_SRC_ALPHA);
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	g_diviceMana.setStencilFunc(true, SS_NOTEQUAL, 0x0, 0xff);
	g_diviceMana.setStencilOp(true, SOP_REPLACE, SOP_REPLACE, SOP_REPLACE);
	glStencilFunc(GL_NOTEQUAL, 0x0, 0xff);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	//g_diviceMana.buffManaRef()->setCurrentVAO(g_shadowVao);
	g_diviceMana.setVAO(g_shadowVao);
	g_diviceMana.drawIndexed(GL_TRIANGLES, g_numberIndicesShadowPlane, GL_UNSIGNED_INT);
	g_diviceMana.setBlendMode(false);
	//glDisable(GL_BLEND);
	//glDisable(GL_STENCIL_TEST);
	g_diviceMana.setStencilTest(false);

	//

	angle += 20.0f * time;

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
	g_diviceMana.init();

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