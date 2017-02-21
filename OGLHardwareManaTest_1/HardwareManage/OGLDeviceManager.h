//#pragma once
//#include "glus.h"
//#include "utMath.h"
//#include <string>
//#include <vector>
//#include "LeanUtil.h"
//
//namespace Lean3D
//{
//	
//
//
//
//
//
//
//	enum FillMode
//	{
//		RS_FILL_SOLID = 0,
//		RS_FILL_WIREFRAME = 1
//	};
//
//	enum CullMode
//	{
//		RS_CULL_BACK = 0,
//		RS_CULL_FRONT,
//		RS_CULL_NONE,
//	};
//
//	struct RasterState
//	{
//		union
//		{
//			uint32  hash;
//			struct
//			{
//				uint32  fillMode : 1;  
//				uint32  cullMode : 2;  
//				uint32  scissorEnable : 1;
//				uint32  multisampleEnable : 1;
//				uint32  renderTargetWriteMask : 1;
//			};
//		};
//	};
//
//	enum BlendFunc
//	{
//		BS_BLEND_ZERO = 0,
//		BS_BLEND_ONE,
//		BS_BLEND_SRC_ALPHA,
//		BS_BLEND_INV_SRC_ALPHA,
//		BS_BLEND_DEST_COLOR
//	};
//
//	struct BlendState
//	{
//		union
//		{
//			uint32  hash;
//			struct
//			{
//				uint32  alphaToCoverageEnable : 1;
//				uint32  blendEnable : 1;
//				uint32  srcBlendFunc : 4;
//				uint32  destBlendFunc : 4;
//			};
//		};
//	};
//
//	enum DepthFunc
//	{
//		DSS_DEPTHFUNC_LESS_EQUAL = 0,
//		DSS_DEPTHFUNC_LESS,
//		DSS_DEPTHFUNC_EQUAL,
//		DSS_DEPTHFUNC_GREATER,
//		DSS_DEPTHFUNC_GREATER_EQUAL,
//		DSS_DEPTHFUNC_ALWAYS
//	};
//
//	struct DepthStencilState
//	{
//		union
//		{
//			uint32  hash;
//			struct
//			{
//				uint32  depthWriteMask : 1;
//				uint32  depthEnable : 1;
//				uint32  depthFunc : 4;  // RDIDepthFunc
//			};
//		};
//	};
//
//	struct DeviceCaps
//	{
//		bool  texFloat;
//		bool  texNPOT;//非2次方纹理
//		bool  rtMultisampling;
//	};
//
//	class OGLDeviceManager
//	{
//	public:
//		OGLDeviceManager();
//		~OGLDeviceManager();
//
//		bool init();	//环境初始化
//		void initStates();//设置像素对齐格式
//		bool commitStates(uint32 filter = 0xFFFFFFFF);
//		void resetStates();
//
//
//
//		
//
//	protected:
//
//		DeviceCaps    _caps;
//
//		uint32        _depthFormat;
//		int           _vpX, _vpY, _vpWidth, _vpHeight;
//		int           _scX, _scY, _scWidth, _scHeight;
//		int           _fbWidth, _fbHeight;
//		std::string   _shaderLog;
//		uint32        _curRendBuf;
//		int           _outputBufferIndex;  // 用于左右眼，立体画面渲染
//		
//
//		int                            _defaultFBO;
//		uint32                         _numVertexLayouts;
//
//		/*RDIVertBufSlot        _vertBufSlots[16];
//		RDITexSlot            _texSlots[16];*/
//		RasterState        _curRasterState, _newRasterState;
//		BlendState         _curBlendState, _newBlendState;
//		DepthStencilState  _curDepthStencilState, _newDepthStencilState;
//		uint32                _prevShaderId, _curShaderId;
//		uint32                _curVertLayout, _newVertLayout;
//		uint32                _curIndexBuf, _newIndexBuf;
//		uint32                _indexFormat;
//		uint32                _activeVertexAttribsMask;
//		uint32                _pendingMask;
//	};
//
//}
//
//
