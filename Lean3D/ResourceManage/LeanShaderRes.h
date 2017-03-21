#pragma once

#include "LeanUtil.h"
#include "LeanResource.h"
#include "LeanTextureRes.h"
#include "Lean3DRoot.h"
#include "OGLDeviceManager.h"
#include <set>
#include <list>

namespace Lean3D
{
	class CodeResource : public Resource
	{
	public:
		CodeResource(const std::string &name, int flags);
		~CodeResource();
		Resource *clone();
		void initDefault();
		void release();

		static Resource *factoryFunc(const std::string &name, int flags)
		{
			return new CodeResource(name, flags);
		}

		bool load(const char *data, int size);
		const std::string &getCode() { return _code; }

	private:
		void includeCodeReplace(std::string &out, std::string &codename, std::string &code);
		void updateCode();

	private:
		uint32			_flagMask;
		std::string		_code;
		std::vector<std::pair<std::string, std::string>>  _include;
	};

	struct ShaderElemType
	{
		enum List
		{
			ContextElem = 600,
			SamplerElem,
			UniformElem,
			ContNameStr,
			SampNameStr,
			SampDefTexResI,
			UnifNameStr,
			UnifSizeI,
			UnifDefValueF4
		};
	};

	//渲染状态
	struct BlendModes
	{
		enum List
		{
			Replace,
			Blend,
			Add,
			AddBlended,
			Mult
		};
	};

	struct TestModes
	{
		enum List
		{
			LessEqual,
			Less,
			Equal,
			Greater,
			GreaterEqual,
			Always
		};
	};

	struct CullModes
	{
		enum List
		{
			Back,
			Front,
			None
		};
	};


	
	//struct ShaderCombination
	//{
	//	uint32              combMask;

	//	uint32              shaderHandle;
	//	uint32              lastUpdateStamp;
	//	//用户自定义一致变量Location
	//	std::vector< int >  samplers;
	//	std::vector< int >  uniforms;


	//	ShaderCombination() :
	//		combMask(0), shaderHandle(0), lastUpdateStamp(0)
	//	{
	//	}
	//};

	enum class ShaderVariableType
	{
		NONE,
		INT,
		FLOAT,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		FLOAT3x3,
		FLOAT4x4
	};

	struct ShaderAttribute
	{
		friend class ShaderPass;
		friend class ShaderResource;
	public:
		const char* getName() const;
		const ShaderVariableType getType() const;
		ShaderPass* getShaderPass() const;
		void setTypeByGLenum(GLenum type);



		ShaderAttribute();
		//ShaderAttribute(const ShaderAttribute& copy){}
		~ShaderAttribute() {}

		//ShaderAttribute& operator=(const ShaderAttribute&){}

		std::string					_name;
		GLint						_location;
		ShaderVariableType			_type;
		unsigned int				_index;
		ShaderPass*					_pass;
	};

	struct ShaderSampler
	{
		friend class ShaderPass;
		friend class ShaderResource;

		void setTypeByGLenum(GLenum type);

		unsigned int							_index;
		std::string								_name;
		TextureType::List						_type;
		ReferenceCountPtr<TextureResource>      _defTex;
		int										_texUnit;
		uint32									_sampState;
		GLint									_location;
		ShaderPass*								_pass;

		ShaderSampler() :
			_texUnit(-1), _sampState(0), _pass(0x0)
		{
		}
	};

	struct ShaderUniform
	{
		friend class ShaderPass;
		friend class ShaderResource;
	public:
		const char* getName() const;
		const ShaderVariableType getType() const;
		ShaderPass* getShaderPass() const;
		void setTypeByGLenum(GLenum type);

		//private:

		ShaderUniform();
		//ShaderUniform(const ShaderUniform& copy){}
		~ShaderUniform() {}

		//ShaderUniform& operator=(const ShaderUniform&){}

		std::string					_name;
		GLint						_location;
		ShaderVariableType			_type;
		unsigned int				_index;
		float						_defValue[16]; // maybe a litter bit memory waste
		ShaderPass*					_pass;
	};

	struct ShaderPass
	{
		std::string                       id;
		uint32                            flagMask;

		// 渲染状态
		BlendModes::List                  blendMode;
		TestModes::List                   depthFunc;
		CullModes::List                   cullMode;
		bool                              depthTest;
		bool                              writeDepth;
		bool                              alphaToCoverage;

		// Shaders
		//std::vector< ShaderCombination >  shaderCombs;
		uint32							  shaderHandle;
		std::vector<ShaderUniform>		  uniforms;
		std::vector<ShaderSampler>		  samplers;
		std::vector<ShaderAttribute>	  attributes;
		int                               vertCodeIdx, fragCodeIdx;
		int								  geoCodeIdx, TescontCodeIdx, TesevalCodeIdx;
		bool                              compiled;

		ShaderPass() :
			blendMode(BlendModes::Replace), depthFunc(TestModes::LessEqual),
			cullMode(CullModes::Back), depthTest(true), writeDepth(true), alphaToCoverage(false),
			vertCodeIdx(-1), fragCodeIdx(-1), geoCodeIdx(-1), TescontCodeIdx(-1), TesevalCodeIdx(-1)
			, compiled(false)
		{
		}
	};



	class ShaderResource : public Resource
	{
	public:
		ShaderResource(const std::string &name, int flags);
		~ShaderResource();

		void initDefault();
		void release();
		Resource* clone();
		bool load(const char *data, int size);
		

		static Resource *factoryFunc(const std::string &name, int flags)
		{
			return new ShaderResource(name, flags);
		}
		static void setPreambles(const std::string & vertPreamble, const std::string &fragPreamble)
		{
			_vertPreamble = vertPreamble;
			_fragPreamble = fragPreamble;
		}
		static uint32 calcCombMask(const std::vector<std::string> &flags);

		
				
		void setPrefixMacros(std::vector<std::string> &macros);
		//void bindComUniform(uint32 shaderHandle,ShaderCombination &sc);

		int getElemCount(int elem);
		int getElemParamI(int elem, int elemIdx, int param);
		float getElemParamF(int elem, int elemIdx, int param, int compIdx);
		void setElemParamF(int elem, int elemIdx, int param, int compIdx, float value);
		const char *getElemParamStr(int elem, int elemIdx, int param);
		bool setUniform(const std::string &name, float a, float b, float c, float d);

		ShaderPass *findPass(const std::string &name);

		std::vector< ShaderPass > &getPasses() { return _passes; }
		CodeResource *getCode(uint32 index) 
		{ 
			return &_codeSections[index]; 
		}

		void compilePasses();
	protected:
		bool parseFXSection(char *fxCode);

		//创建编译shader,获取一致变量location
		void compilePassCode(ShaderPass &context);

	private:
		static std::string				_vertPreamble, _fragPreamble;
		static std::string				_geoPreamble, _tescontPreamble, _tesevalPreamble;
		static std::string				_tmpCode0, _tmpCode1, _tmpCode2, _tmpCode3, _tmpCode4;

		std::string						_macrosPrefix;  // index:0-vertex 1-pixel 2-geo 3-tesscontrol 4-tesseva
		std::vector< ShaderPass >		_passes;
		std::vector< ShaderSampler >	_samplers;
		std::vector< ShaderUniform >	_uniforms;
		std::vector< CodeResource >		_codeSections;
		std::set< uint32 >				_preLoadList;
	};
}