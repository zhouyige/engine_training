#pragma once

#include "LeanUtil.h"
#include "LeanResource.h"
#include "LeanTextureRes.h"
#include "Lean3DRoot.h"
#include "OGLDeviceManager.h"
#include <set>

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

		bool hasDependency(CodeResource *codeRes);
		bool getCodeLinkFlags(uint32 *flagMask);
		std::string assembleCode();

		bool isLoaded() { return _loaded; }
		const std::string &getCode() { return _code; }

	private:
		void updateShaders();

	private:
		uint32			_flagMask;
		std::string		_code;
		std::vector<std::pair<ReferenceCountPtr<CodeResource>, size_t>>  _include;
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

	struct ShaderCombination
	{
		uint32              combMask;

		uint32              shaderHandle;
		uint32              lastUpdateStamp;

		// y引擎一致变量Location
		int                 uni_frameBufSize;
		int                 uni_viewMat, uni_viewMatInv, uni_projMat, uni_viewProjMat, uni_viewProjMatInv, uni_viewerPos;
		int                 uni_worldMat, uni_worldNormalMat, uni_nodeId, uni_customInstData;
		int                 uni_skinMatRows;
		int                 uni_lightPos, uni_lightDir, uni_lightColor;
		int                 uni_shadowSplitDists, uni_shadowMats, uni_shadowMapSize, uni_shadowBias;
		int                 uni_parPosArray, uni_parSizeAndRotArray, uni_parColorArray;
		int                 uni_olayColor;
		//用户自定义一致变量Location
		std::vector< int >  customSamplers;
		std::vector< int >  customUniforms;


		ShaderCombination() :
			combMask(0), shaderHandle(0), lastUpdateStamp(0)
		{
		}
	};

	struct ShaderContext
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
		std::vector< ShaderCombination >  shaderCombs;
		int                               vertCodeIdx, fragCodeIdx;
		int								  geoCodeIdx, TescontCodeIdx, TesevalCodeIdx;
		bool                              compiled;


		ShaderContext() :
			blendMode(BlendModes::Replace), depthFunc(TestModes::LessEqual),
			cullMode(CullModes::Back), depthTest(true), writeDepth(true), alphaToCoverage(false),
			vertCodeIdx(-1), fragCodeIdx(-1), geoCodeIdx(-1), TescontCodeIdx(-1), TesevalCodeIdx(-1)
			, compiled(false)
		{
		}
	};

	struct ShaderSampler
	{
		std::string								id;
		TextureType::List						type;
		ReferenceCountPtr<TextureResource>      defTex;
		int										texUnit;
		uint32									sampState;


		ShaderSampler() :
			texUnit(-1), sampState(0)
		{
		}
	};

	struct ShaderUniform
	{
		std::string    id;
		float          defValues[4];
		unsigned char  size;
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

		
				
		void preLoadCombination(uint32 combMask);
		ShaderCombination *getCombination(ShaderContext &context, uint32 combMask);
		void bindComUniform(uint32 shaderHandle,ShaderCombination &sc);

		int getElemCount(int elem);
		int getElemParamI(int elem, int elemIdx, int param);
		float getElemParamF(int elem, int elemIdx, int param, int compIdx);
		void setElemParamF(int elem, int elemIdx, int param, int compIdx, float value);
		const char *getElemParamStr(int elem, int elemIdx, int param);
		bool setUniform(const std::string &name, float a, float b, float c, float d);

		ShaderContext *findContext(const std::string &name)
		{
			for (uint32 i = 0; i < _contexts.size(); ++i)
				if (_contexts[i].id == name) return &_contexts[i];

			return 0x0;
		}
		
		std::vector< ShaderContext > &getContexts() { return _contexts; }
		CodeResource *getCode(uint32 index) 
		{ 
			return &_codeSections[index]; 
		}

		void compileContexts();
	protected:
		bool parseFXSection(char *fxCode);

		//创建编译shader,获取一致变量location
		void compileCombination(ShaderContext &context, ShaderCombination &sc);

	private:
		static std::string				_vertPreamble, _fragPreamble;
		static std::string				_geoPreamble, _tescontPreamble, _tesevalPreamble;
		static std::string				_tmpCode0, _tmpCode1, _tmpCode2, _tmpCode3, _tmpCode4;

		std::vector< ShaderContext >  _contexts;
		std::vector< ShaderSampler >  _samplers;
		std::vector< ShaderUniform >  _uniforms;
		std::vector< CodeResource >   _codeSections;
		std::set< uint32 >            _preLoadList;
	};
}