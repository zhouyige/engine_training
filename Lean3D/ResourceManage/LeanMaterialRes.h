#pragma once

#include "LeanUtil.h"
#include "LeanResource.h"
#include "LeanTextureRes.h"
#include "LeanGeometryRes.h"
#include "LeanAnimationRes.h"
#include "LeanShaderRes.h"
#include <string>

namespace Lean3D
{
	struct MaterialElemType
	{
		enum List
		{
			MaterialElem = 1300,
			SamplerElem,
			UniformElem,
			MatClassStr,
			MatLinkI,
			MatShaderI,
			SampNameStr,
			SampTexResI,
			UnifNameStr,
			UnifValueF4
		};

	};

	struct MatSampler
	{
		std::string		name;
		ReferenceCountPtr<TextureResource> texRes;
	};

	struct MatUniform
	{
		std::string		name;
		float			values[4];

		MatUniform()
		{
			values[0] = 0;
			values[1] = 0;
			values[2] = 0;
			values[3] = 0;
		}
	};

	class MaterialResource : public Resource
	{
	public:
		static Resource *factoryFunc(const std::string &name, int flags)
		{
			return new MaterialResource(name, flags);
		}

		MaterialResource(const std::string &name, int flags);
		~MaterialResource();
		Resource *clone();

		void initDefault();
		void release();
		bool load(const char *data, int size);
		bool setUniform(const std::string &name, float a, float b, float c, float d);
		ShaderResource *getShaderRes() { return _shaderRes; }
		//
		bool isOfClass(const std::string &theClass);
		
		//查询各种元素数量
		int getElemCount(int elem);			
		//查询 设置 linkmat，shader，sampler
		int getElemParamI(int elem, int elemIdx, int param);		
		void setElemParamI(int elem, int elemIdx, int param, int value);
		//查询 设置 uniform
		float getElemParamF(int elem, int elemIdx, int param, int compIdx);
		void setElemParamF(int elem, int elemIdx, int param, int compIdx, float value);
		//查询 材质class samplerName uniformName
		const char *getElemParamStr(int elem, int elemIdx, int param);
		//设置材质class
		void setElemParamStr(int elem, int elemIdx, int param, const char *value);

	private:
		ReferenceCountPtr<ShaderResource>     _shaderRes;
		//uint32							  _combMask;
		std::string							  _class;
		std::vector< ShaderSampler >		  _samplers;
		std::vector< ShaderUniform >		  _uniforms;
		std::vector< std::string >			  _shaderFlags;
		ReferenceCountPtr<MaterialResource>   _matLink;

		friend class ResourceManager;
		friend class Renderer;
		friend class MeshNode;
	};
}