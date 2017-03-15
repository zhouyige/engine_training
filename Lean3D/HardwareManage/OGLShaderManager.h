#pragma once
#include "LeanUtil.h"

namespace Lean3D
{
	//着色器
	enum UniformType
	{
		UNIFORM_FLOAT,
		UNIFORM_FLOAT2,
		UNIFORM_FLOAT3,
		UNIFORM_FLOAT33,
		UNIFORM_FLOAT4,
		UNIFORM_FLOAT44
	};
	struct ComuseShader	//常用shader包括顶点，细分，几何，片元着色器
	{
		GLUSprogram program;
	};
	struct ComputeShader//计算着色器
	{
		GLUSprogram program;
	};

	class OGLShaderManager///computeShader还未完善
	{
	public:
		OGLShaderManager();
		~OGLShaderManager();

		uint32 createShader(const char *vertexShaderSrc, const char *tescontShaderSrc
							, const char *tesevalShaderSrc, const char *geomShaderSrc
							, const char *fragmentShaderSrc);
		uint32 createComputeShader(const char *computeShaderSrc);
		void destroyShader(uint32 shaderHandle);
		void destroyComputeShader(uint32 shaderHandle);

		void setCurrentShader(uint32 shaderHandle);
		void setCurrentComputeShader(uint32 shaderHandle);

		//这里这的uniform指不包括sampler的一致变量
		void setShaderUniform(int loc, UniformType type, void *values, uint32 count = 1);
		void setShaderSampler(int loc, uint32 texUnit);
		
		int getShaderUniformLoc(uint32 shaderHandle, const char *name);
		int getShaderSamplerLoc(uint32 shaderHandle, const char *name);
			
		int getComputeShaderUniformLoc(uint32 shaderHandle, const char *name);
		int getComputeShaderSamplerLoc(uint32 shaderHandle, const char *name);

		const char *getDefaultVSCode();
		const char *getDefaultFSCode();

		void setPragramAttribLoc(uint32 shaderHandle, VertexAttribLayoutList &vaolist);

		int getActiveAttributeNum(uint32 shaderHandle);
		int getActiveUniformNum(uint32 shaderHandle);
		int getActiveAttributeMaxLength(uint32 shaderHandle);
		int getActiveUniformMaxLength(uint32 shaderHandle);
		void getActiveAttrib(uint32 shaderHandle, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
		void getActiveUniform(uint32 shaderHandle, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
		int getShaderAttributeLoc(uint32 shaderHandle, const char *name);

		std::string &getShaderLog() { return _shaderLog; }
		
	private:
		ReferenceObjList< ComuseShader >        _comuseShadersReflist;
		ReferenceObjList< ComputeShader>  _computeShaderReflist;
		std::string _shaderLog;

	};
	
}