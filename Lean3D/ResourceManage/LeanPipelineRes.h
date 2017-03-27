#pragma once

#include "LeanUtil.h"
#include "LeanResource.h"
#include "LeanTextureRes.h"
#include "LeanShaderRes.h"
#include "LeanMaterialRes.h"
#include "Lean3DRoot.h"
#include "OGLDeviceManager.h"
#include <set>
#include <list>


namespace Lean3D {

	class XMLNode;


	// =================================================================================================
	// Pipeline Resource
	// =================================================================================================

	struct PipelineResData
	{
		enum List
		{
			StageElem = 900,
			StageNameStr,
			StageActivationI
		};
	};

	// =================================================================================================

	struct PipelineCommands
	{
		enum List
		{
			SwitchTarget,
			BindBuffer,
			UnbindBuffers,
			ClearTarget,
			DrawGeometry,
			DrawOverlays,
			DrawQuad,
			DoForwardLightLoop,
			DoDeferredLightLoop,
			SetUniform
		};
	};

	class PipeCmdParam
	{
	public:
		PipeCmdParam() : _string(0x0) { _basic.ptr = 0x0; }
		PipeCmdParam(const PipeCmdParam &copy)
			: _basic(copy._basic), _string(0x0), _resource(copy._resource)
		{
			if (copy._string) setString(copy._string->c_str());
		}
		~PipeCmdParam() { delete _string; }

		float getFloat() { return _basic.f; }
		int getInt() { return _basic.i; }
		bool getBool() { return _basic.b; }
		void *getPtr() { return _basic.ptr; }
		const char *getString() { return _string ? _string->c_str() : 0x0; }
		Resource *getResource() { return _resource.getPtr(); }

		void setFloat(float f) { _basic.f = f; }
		void setInt(int i) { _basic.i = i; }
		void setBool(bool b) { _basic.b = b; }
		void setPtr(void *ptr) { _basic.ptr = ptr; }
		void setString(const char *str) { _string = new std::string(str); }
		void setResource(Resource *resource) { _resource = resource; }

	protected:
		union BasicType
		{
			float  f;
			int    i;
			bool   b;
			void   *ptr;
		};

		BasicType						_basic;
		std::string						*_string;
		ReferenceCountPtr<Resource>     _resource;
	};


	struct PipelineCommand
	{
		PipelineCommands::List       command;
		std::vector< PipeCmdParam >  params;

		PipelineCommand(PipelineCommands::List	command)
		{
			this->command = command;
		}
	};


	struct PipelineStage
	{
		std::string									name;
		ReferenceCountPtr<MaterialResource>         matLink;
		std::vector< PipelineCommand >				commands;
		bool										enabled;

		PipelineStage() : matLink(0x0) {}
	};


	struct RenderTarget
	{
		std::string           id;
		uint32                numColBufs;
		TextureFormats::List  format;
		uint32                width, height;
		uint32                samples;
		float                 scale;  // Scale factor for FB width and height
		bool                  hasDepthBuf;
		uint32                rendBuf;

		RenderTarget()
		{
			hasDepthBuf = false;
			numColBufs = 0;
			rendBuf = 0;
		}
	};

	// =================================================================================================

	class PipelineResource : public Resource
	{
	public:
		static Resource *factoryFunc(const std::string &name, int flags)
		{
			return new PipelineResource(name, flags);
		}

		PipelineResource(const std::string &name, int flags);
		~PipelineResource();

		void initDefault();
		void release();
		bool load(const char *data, int size);
		void resize(uint32 width, uint32 height);

		int getElemCount(int elem);
		int getElemParamI(int elem, int elemIdx, int param);
		void setElemParamI(int elem, int elemIdx, int param, int value);
		const char *getElemParamStr(int elem, int elemIdx, int param);

		bool getRenderTargetData(const std::string &target, int bufIndex, int *width, int *height,
			int *compCount, void *dataBuffer, int bufferSize);

	private:
		bool raiseError(const std::string &msg, int line = -1);
		const std::string parseStage(XMLNode &node, PipelineStage &stage);

		void addRenderTarget(const std::string &id, bool depthBuffer, uint32 numBuffers,
			TextureFormats::List format, uint32 samples,
			uint32 width, uint32 height, float scale);
		RenderTarget *findRenderTarget(const std::string &id);
		bool createRenderTargets();
		void releaseRenderTargets();

	private:
		std::vector< RenderTarget >   _renderTargets;
		std::vector< PipelineStage >  _stages;
		uint32                        _baseWidth, _baseHeight;

		friend class ResourceManager;
		friend class Renderer;
	};

	typedef ReferenceCountPtr< PipelineResource > PPipelineResource;

}