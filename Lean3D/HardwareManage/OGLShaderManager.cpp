#include "OGLShaderManager.h"
#include <string.h>

namespace Lean3D
{
	OGLShaderManager::OGLShaderManager()
	{
	}

	OGLShaderManager::~OGLShaderManager()
	{
	}

	const char * OGLShaderManager::getDefaultVSCode()
	{
		return "uniform mat4 viewProjMat;\n"
			   "uniform mat4 worldMat;\n"
			   "attribute vec3 vertPos;\n"
			   "void main() {\n"
			   "	gl_Position = viewProjMat * worldMat * vec4( vertPos, 1.0 );\n"
			   "}\n";
	}

	const char * OGLShaderManager::getDefaultFSCode()
	{
		return 	"uniform vec4 color;\n"
				"void main() {\n"
				"	gl_FragColor = color;\n"
				"}\n";
	}	

	uint32 OGLShaderManager::createShader(const char *vertexShaderSrc, const char *tescontShaderSrc
										, const char *tesevalShaderSrc, const char *geomShaderSrc
										, const char *fragmentShaderSrc)
	{
		ComuseShader newShader;
		bool useTesslShader = false, useGeomShader = false;

		if (tescontShaderSrc != 0 && tesevalShaderSrc != 0 && 
			std::string(tescontShaderSrc) != "" && std::string(tesevalShaderSrc) != "")
			useTesslShader = true;
		if (geomShaderSrc != 0 && std::string(geomShaderSrc) != "")
			useGeomShader = true;
		
		
		if (useTesslShader == false && useGeomShader == false)
			glusProgramBuildFromSource(&newShader.program, (const GLchar**)&vertexShaderSrc, 0, 0, 0
									  , (const GLchar**)&fragmentShaderSrc);
		else if (useTesslShader == false && useGeomShader == true)
			glusProgramBuildFromSource(&newShader.program, (const GLchar**)&vertexShaderSrc, 0, 0
									  , (const GLchar**)&geomShaderSrc, (const GLchar**)&fragmentShaderSrc);
		else if (useTesslShader == true && useGeomShader == false)
			glusProgramBuildFromSource(&newShader.program, (const GLchar**)&vertexShaderSrc, (const GLchar**)&tescontShaderSrc
									  , (const GLchar**)&tesevalShaderSrc, 0, (const GLchar**)&fragmentShaderSrc);
		else
			glusProgramBuildFromSource(&newShader.program, (const GLchar**)&vertexShaderSrc, (const GLchar**)&tescontShaderSrc
									  , (const GLchar**)&tesevalShaderSrc, (const GLchar**)&geomShaderSrc
									  , (const GLchar**)&fragmentShaderSrc);
		ASSERT(newShader.program.program >= 0);
		if (newShader.program.program == 0) return 0;
		
	
		
		return _comuseShadersReflist.add(newShader);
	}

	void OGLShaderManager::setPragramAttribLoc(uint32 shaderHandle, VertexAttribLayoutList &vaolist)
	{
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);

		for (int i = 0; i < vaolist.numAttribs; ++i)
		{
			if (vaolist.attribs[i].vboType == GL_ELEMENT_ARRAY_BUFFER) return;
			const char *attribName = vaolist.attribs[i].locationName.c_str();
			ASSERT(strcmp(attribName, "") != 0);
			vaolist.attribs[i].attribLoc =  glGetAttribLocation(shader.program.program, attribName);
		}

	}

	int OGLShaderManager::getActiveAttributeNum(uint32 shaderHandle)
	{
		int attribnum = 0;
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);
		glGetProgramiv(shader.program.program, GL_ACTIVE_ATTRIBUTES, &attribnum);
		return attribnum;
	}

	int OGLShaderManager::getActiveUniformNum(uint32 shaderHandle)
	{
		int uniformnum = 0;
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);
		glGetProgramiv(shader.program.program, GL_ACTIVE_UNIFORMS, &uniformnum);
		return uniformnum;
	}

	int OGLShaderManager::getActiveAttributeMaxLength(uint32 shaderHandle)
	{
		int length = 0;
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);
		glGetProgramiv(shader.program.program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length);
		return length;
	}

	int OGLShaderManager::getActiveUniformMaxLength(uint32 shaderHandle)
	{
		int length = 0;
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);
		glGetProgramiv(shader.program.program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &length);
		return length;
	}

	void OGLShaderManager::getActiveAttrib(uint32 shaderHandle, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name)
	{
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);
		glGetActiveAttrib(shader.program.program, index, bufSize, length, size, type, name);
	}

	void OGLShaderManager::getActiveUniform(uint32 shaderHandle, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name)
	{
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);
		glGetActiveUniform(shader.program.program, index, bufSize, length, size, type, name);
	}

	uint32 OGLShaderManager::createComputeShader(const char *computeShaderSrc)
	{
		ComuseShader newShader;
		
		glusProgramBuildComputeFromSource(&newShader.program, (const GLchar**)&computeShaderSrc);
		if (newShader.program.program == 0) return 0;

		return _comuseShadersReflist.add(newShader);
	}

	void OGLShaderManager::destroyShader(uint32 shaderHandle)
	{
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);

		glUseProgram(0);
		glusProgramDestroy(&shader.program);

		_comuseShadersReflist.remove(shaderHandle);
	}

	void OGLShaderManager::destroyComputeShader(uint32 shaderHandle)
	{
		ComputeShader &shader = _computeShaderReflist.getRef(shaderHandle);

		glUseProgram(0);
		glusProgramDestroy(&shader.program);

		_computeShaderReflist.remove(shaderHandle);
	}

	void OGLShaderManager::setCurrentShader(uint32 shaderHandle)
	{
		unsigned int glShaderRef = 0;
		if (shaderHandle != 0)
		{
			ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);
			glShaderRef = shader.program.program;
		}
		
		glUseProgram(glShaderRef);
	}

	void OGLShaderManager::setCurrentComputeShader(uint32 shaderHandle)
	{
		ComputeShader &shader = _computeShaderReflist.getRef(shaderHandle);
		glUseProgram(shader.program.program);
	}

	void OGLShaderManager::setShaderUniform(int loc, UniformType type, void *values, uint32 count /*= 1*/)
	{
		switch (type)
		{
		case Lean3D::UNIFORM_FLOAT:
			glUniform1fv(loc, count, (float*)values);
			break;
		case Lean3D::UNIFORM_FLOAT2:
			glUniform2fv(loc, count, (float *)values);
			break;
		case Lean3D::UNIFORM_FLOAT3:
			glUniform3fv(loc, count, (float *)values);
			break;
		case Lean3D::UNIFORM_FLOAT33:
			glUniformMatrix3fv(loc, count, GL_FALSE, (float *)values);
			break;
		case Lean3D::UNIFORM_FLOAT4:
			glUniform4fv(loc, count, (float *)values);
			break;
		case Lean3D::UNIFORM_FLOAT44:
			glUniformMatrix4fv(loc, count, GL_FALSE, (float *)values);
			break;
		}
	}

	void OGLShaderManager::setShaderSampler(int loc, uint32 texUnit)
	{
		glUniform1i(loc, (int)texUnit);
	}

	int OGLShaderManager::getShaderUniformLoc(uint32 shaderHandle, const char *name)
	{
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);

		return  glGetUniformLocation(shader.program.program, name);
	}

	int OGLShaderManager::getShaderSamplerLoc(uint32 shaderHandle, const char *name)
	{
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);

		return  glGetUniformLocation(shader.program.program, name);
	}

	int OGLShaderManager::getComputeShaderUniformLoc(uint32 shaderHandle, const char *name)
	{
		return 0;
	}

	int OGLShaderManager::getComputeShaderSamplerLoc(uint32 shaderHandle, const char *name)
	{
		return 0;
	}

	int OGLShaderManager::getShaderAttributeLoc(uint32 shaderHandle, const char *name)
	{
		ComuseShader &shader = _comuseShadersReflist.getRef(shaderHandle);

		return  glGetAttribLocation(shader.program.program, name);
	}

}