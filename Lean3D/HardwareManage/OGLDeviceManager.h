#pragma once
#include "glus.h"
#include "utMath.h"
#include <string>
#include <vector>
#include "LeanUtil.h"
#include "OGLBufferManager.h"
#include "OGLOcclusionQuery.h"
#include "OGLRenderBufferManager.h"
#include "OGLShaderManager.h"
#include "OGLTextureManager.h"


namespace Lean3D
{
	enum OGLFillMode
	{
		RS_FILL_SOLID = 0,
		RS_FILL_WIREFRAME = 1
	};

	enum OGLCullMode
	{
		RS_CULL_BACK = 0,
		RS_CULL_FRONT,
		RS_CULL_NONE,
	};

	struct OGLRasterState
	{
		union
		{
			uint32  hash;
			struct
			{
				uint32  fillMode : 1;  
				uint32  cullMode : 2;  
				uint32  scissorEnable : 1;
				uint32  multisampleEnable : 1;
				uint32  renderTargetWriteMask : 1;
			};
		};
	};

	enum OGLBlendFunc
	{
		BS_BLEND_ZERO = 0,
		BS_BLEND_ONE,
		BS_BLEND_SRC_ALPHA,
		BS_BLEND_INV_SRC_ALPHA,	// 1 - sAlpha
		BS_BLEND_DEST_COLOR
	};

	struct OGLBlendState
	{
		union
		{
			uint32  hash;
			struct
			{
				uint32  alphaToCoverageEnable : 1;
				uint32  blendEnable : 1;
				uint32  srcBlendFunc : 4;
				uint32  destBlendFunc : 4;
			};
		};
	};

	enum OGLDepthFunc
	{
		DSS_DEPTHFUNC_LESS_EQUAL = 0,
		DSS_DEPTHFUNC_LESS,
		DSS_DEPTHFUNC_EQUAL,
		DSS_DEPTHFUNC_GREATER,
		DSS_DEPTHFUNC_GREATER_EQUAL,
		DSS_DEPTHFUNC_ALWAYS
	};

	struct OGLDepthStencilState
	{
		union
		{
			uint32  hash;
			struct
			{
				uint32  depthWriteMask : 1;
				uint32  depthEnable : 1;
				uint32  depthFunc : 4;  // OGLDepthFunc
			};
		};
	};

	enum OGLStencilFunc
	{
		SS_NEVER = 0,			// 总是失败
		SS_LESS = 1,			// <
		SS_LEQUAL = 2,			// <=
		SS_GREATER = 3,			// >
		SS_GEQUAL = 4,			// >=
		SS_EQUAL = 5,			// ==
		SS_NOTEQUAL = 6,		// !=
		SS_ALWAYS = 7			// 总是通过
	};

	enum OGLStencilOp
	{
		SOP_KEEP = 0,				//保持当前值
		SOP_ZERO = 1,				//模板缓冲去值设为0
		SOP_REPLACE = 2,			//设置为stencilfunc指定的ref值
		SOP_INCR = 3,				//增加当前模板缓冲区的值，截取到最大无符号值
		SOP_INCR_WRAP = 4,			//增加当前模板缓冲区的值，增加最大值时环绕到0
		SOP_DECR = 5,				//增加当前缓冲区值，截取到0
		SOP_DECR_WRAP = 6,			//增加当前模板缓冲区的值，增加0时环绕到最大无符号值
		SOP_INVERT = 7				//模板缓冲区的值按位取反
	};

	struct OGLStencilState
	{
		union
		{
			uint32  hash;
			struct
			{
				uint32  MaskEnable : 1;
				uint32  StencilEnable : 1;
				uint32	StencilOp : 1;
				uint32  FuncEnable : 1;
				OGLStencilFunc  StencilFunc : 4;
				OGLStencilOp	StencilFail : 4;
				OGLStencilOp	DepthFail : 4;
				OGLStencilOp  DepthPass : 4;
				GLint   FuncRef;
				uint32  FuncMask;
				uint32    StencilMask;
			};
		};
	};

	struct OGLDeviceCaps
	{
		bool  texFloat;
		bool  texNPOT;//非2次方纹理
		bool  rtMultisampling;
	};

	class OGLDeviceManager
	{
	public:
		OGLDeviceManager();
		~OGLDeviceManager();

		bool init();	//环境初始化
		void initStates();//设置像素对齐格式
		bool commitStates(uint32 filter = 0xFFFFFFFF);
		void resetStates();
		void applyRenderStates();

		inline void setViewport(int x, int y, int width, int height)
		{
			_vpX = x; _vpY = y; _vpWidth = width; _vpHeight = height; _pendingMask |= PM_VIEWPORT;
		}
		inline void setScissorRect(int x, int y, int width, int height)
		{
			_scX = x; _scY = y; _scWidth = width; _scHeight = height; _pendingMask |= PM_SCISSOR;
		}
		//void setIndexBuffer(uint32 bufObj, RDIIndexFormat idxFmt)
		//{
		//	_indexFormat = (uint32)idxFmt; _newIndexBuf = bufObj; _pendingMask |= PM_INDEXBUF;
		//}
		//void setVertexBuffer(uint32 slot, uint32 vbObj, uint32 offset, uint32 stride)
		//{
		//	ASSERT(slot < 16); _vertBufSlots[slot] = RDIVertBufSlot(vbObj, offset, stride);
		//	_pendingMask |= PM_VERTLAYOUT;
		//}
		/*	void setVertexLayout(uint32 vlObj)
			{
			_newVertLayout = vlObj;
			}*/
		inline  void setVAO(uint32 vaoHandle) const
		{ _bufManager->setCurrentVAO(vaoHandle); }
		inline void setTexture(uint32 activeUnit, uint32 texHandle, uint16 samplerState)
		{
			_texManager->addActiveTexture(activeUnit, texHandle, samplerState);
			_pendingMask |= PM_TEXTURES;
		}
		//OGL渲染状态
		inline void setColorWriteMask(bool enabled)//如果光栅状态hash变化了默认情况下Color是false
		{
			_newRasterState.renderTargetWriteMask = enabled; _pendingMask |= PM_RENDERSTATES;
		}
		inline void getColorWriteMask(bool &enabled) const
		{
			enabled = _newRasterState.renderTargetWriteMask;
		}
		inline void setFillMode(OGLFillMode fillMode)
		{
			_newRasterState.fillMode = fillMode; _pendingMask |= PM_RENDERSTATES;
		}
		inline void getFillMode(OGLFillMode &fillMode) const
		{
			fillMode = static_cast<OGLFillMode>(_newRasterState.fillMode);
		}
		inline void setCullMode(OGLCullMode cullMode)
		{
			_newRasterState.cullMode = cullMode; _pendingMask |= PM_RENDERSTATES;
		}
		inline void getCullMode(OGLCullMode &cullMode) const
		{
			cullMode = static_cast<OGLCullMode>(_newRasterState.cullMode);
		}
		inline void setScissorTest(bool enabled)
		{
			_newRasterState.scissorEnable = enabled; _pendingMask |= PM_RENDERSTATES;
		}
		inline void getScissorTest(bool &enabled) const
		{
			enabled = _newRasterState.scissorEnable;
		}
		inline void setMulisampling(bool enabled)
		{
			_newRasterState.multisampleEnable = enabled; _pendingMask |= PM_RENDERSTATES;
		}
		inline void getMulisampling(bool &enabled) const
		{
			enabled = _newRasterState.multisampleEnable;
		}
		inline void setAlphaToCoverage(bool enabled)
		{
			_newBlendState.alphaToCoverageEnable = enabled; _pendingMask |= PM_RENDERSTATES;
		}
		inline void getAlphaToCoverage(bool &enabled) const
		{
			enabled = _newBlendState.alphaToCoverageEnable;
		}
		inline void setBlendMode(bool enabled, OGLBlendFunc srcBlendFunc = BS_BLEND_ZERO, OGLBlendFunc destBlendFunc = BS_BLEND_ZERO)
		{
			_newBlendState.blendEnable = enabled; _newBlendState.srcBlendFunc = srcBlendFunc;
			_newBlendState.destBlendFunc = destBlendFunc; _pendingMask |= PM_RENDERSTATES;
		}
		inline void getBlendMode(bool &enabled, OGLBlendFunc &srcBlendFunc, OGLBlendFunc &destBlendFunc) const
		{
			enabled = _newBlendState.blendEnable; srcBlendFunc = static_cast<OGLBlendFunc>(_newBlendState.srcBlendFunc);
			destBlendFunc = static_cast<OGLBlendFunc>(_newBlendState.destBlendFunc);
		}
		inline void setDepthMask(bool enabled)
		{
			_newDepthStencilState.depthWriteMask = enabled; _pendingMask |= PM_RENDERSTATES;
		}
		inline void getDepthMask(bool &enabled) const
		{
			enabled = _newDepthStencilState.depthWriteMask;
		}
		inline void setDepthTest(bool enabled)
		{
			_newDepthStencilState.depthEnable = enabled; _pendingMask |= PM_RENDERSTATES;
		}
		inline void getDepthTest(bool &enabled) const
		{
			enabled = _newDepthStencilState.depthEnable;
		}
		inline void setDepthFunc(OGLDepthFunc depthFunc)
		{
			_newDepthStencilState.depthFunc = depthFunc; _pendingMask |= PM_RENDERSTATES;
		}
		inline void getDepthFunc(OGLDepthFunc &depthFunc) const
		{
			depthFunc = static_cast<OGLDepthFunc>(_newDepthStencilState.depthFunc);
		}
		inline void setStencilTest(bool enabled)
		{
			_newStencilState.StencilEnable = enabled;
			_pendingMask |= PM_RENDERSTATES;
		}
		inline void getStencilTest(bool &enabled) const
		{
			enabled = _newStencilState.StencilEnable;
		}
		inline void setStencilMask(bool enabled, uint32 mask)
		{
			_newStencilState.MaskEnable = enabled;
			_newStencilState.StencilMask = mask;
			_pendingMask |= PM_RENDERSTATES;
		}
		inline void getStencilMask(bool &enabled, uint32 &mask) const
		{
			enabled = _newStencilState.MaskEnable;
			mask = _newStencilState.StencilMask;
		}
		inline void setStencilFunc(bool enabled, OGLStencilFunc func, int refvalue, uint32 mask)
		{
			_newStencilState.FuncEnable = enabled;
			_newStencilState.StencilFunc = func;
			_newStencilState.FuncRef = refvalue;
			_newStencilState.FuncMask = mask;
			_pendingMask |= PM_RENDERSTATES;
		}
		inline void getStencilFunc(bool &enabled, OGLStencilFunc &func, int &refvalue, uint32 &mask) const
		{
			enabled = _newStencilState.FuncEnable;
			func = _newStencilState.StencilFunc;
			refvalue = _newStencilState.FuncRef;
			mask = _newStencilState.FuncMask;
		}
		inline void setStencilOp(bool enabled, OGLStencilOp sfail, OGLStencilOp dpfial, OGLStencilOp dppass)
		{
			_newStencilState.StencilOp = enabled;
			_newStencilState.StencilFail = sfail;
			_newStencilState.DepthFail = dpfial;
			_newStencilState.DepthPass = dppass;
			_pendingMask |= PM_RENDERSTATES;
		}
		inline void getStencilOp(bool &enabled, OGLStencilOp &sfail, OGLStencilOp &dpfial, OGLStencilOp &dppass) const
		{
			enabled = _newStencilState.StencilOp;
			sfail = _newStencilState.StencilFail;
			dpfial = _newStencilState.DepthFail;
			dppass = _newStencilState.DepthPass;
		}

		//drawcall
		void draw(uint32 primType, uint32 firstVert, uint32 numVerts);
		void drawIndexed(uint32 primType, uint32 count, uint32 datatype);

		void clearColor(GLclampf R, GLclampf G, GLclampf B, GLclampf alpha);
		void clearDepth(GLclampd depth);
		void clearStencil(GLint s);
		void clear(GLbitfield mask);

		inline OGLBufferManager *buffManaRef() const
		{ return _bufManager; }
		inline OGLTextureManager *texManaRef() const
		{ return _texManager; }
		inline OGLRenderBufferManager *rendbufManaRef() const
		{ return _rendbufManager; }
		inline OGLShaderManager *shaderManaRef() const
		{ return _shaderManager; }
		inline OGLOcclusionQuery *oclQueryRef() const
		{ return _oclManager; }

		//纹理临时内存一次创建，避免临时纹理内存还要申请删除内存
		unsigned char *useScrathBuf(uint32 minSize);


	protected:

		enum OGLPendingMask
		{
			PM_VIEWPORT = 0x00000001,
			PM_VAOBUFF = 0x00000002,
			PM_TEXTURES = 0x00000004,
			PM_SCISSOR = 0x0000008,
			PM_RENDERSTATES = 0x00000010,
		};

		OGLDeviceCaps    _caps;
		int           _vpX, _vpY, _vpWidth, _vpHeight;
		int           _scX, _scY, _scWidth, _scHeight;
		int           _fbWidth, _fbHeight;
		std::string   _shaderLog;
		uint32        _curRendBuf;
		int           _outputBufferIndex;  // 用于左右眼，立体画面渲染
		int                            _defaultFBO;
		OGLRasterState        _preRasterState, _newRasterState;
		OGLBlendState         _preBlendState, _newBlendState;
		OGLDepthStencilState  _preDepthStencilState, _newDepthStencilState;
		OGLStencilState		  _preStencilState, _newStencilState;
		uint32                _preShaderHandle, _curShaderHandle;
		uint32				  _preVaoHandle, _newVaoHandle;
		//uint32                _curVertLayout, _newVertLayout;
		//uint32                _curIndexBuf, _newIndexBuf;
		//uint32                _indexFormat;
		//uint32                _activeVertexAttribsMask;
		uint32                _pendingMask;

		OGLBufferManager		*_bufManager;
		OGLOcclusionQuery		*_oclManager;
		OGLRenderBufferManager  *_rendbufManager;
		OGLShaderManager		*_shaderManager;               
		OGLTextureManager		*_texManager;

		//2016-5-18
		unsigned char			*_scratchBuf;
		uint32					_scratchBufSize;
	};

}


