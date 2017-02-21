#include "OGLDeviceManager.h"


namespace Lean3D
{

	OGLDeviceManager::~OGLDeviceManager()
	{
		_texManager = NULL;
		_shaderManager = NULL;
		_rendbufManager = NULL;
		_oclManager = NULL;
		_bufManager = NULL;

		init();
	}
	OGLDeviceManager::OGLDeviceManager()
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//像素对齐为1的倍数
	}

	bool OGLDeviceManager::init()
	{
		_bufManager = new OGLBufferManager();
		_oclManager = new OGLOcclusionQuery();
		_texManager = new OGLTextureManager();
		_rendbufManager = new OGLRenderBufferManager(_texManager);
		_shaderManager = new OGLShaderManager();

		//此处应该检查本机对OPENGL的支持状态
		//现在只是大概初始化下 
		_caps.texFloat = true;
		_caps.texNPOT = true;
		_caps.rtMultisampling = true;

		//2016-5-18
		_scratchBuf = 0x0;
		_scratchBufSize = 0;
		useScrathBuf(4 * 1024 * 1024);

		initStates();
		return true;
	}

	void OGLDeviceManager::initStates()
	{

	}

	bool OGLDeviceManager::commitStates(uint32 filter /*= 0xFFFFFFFF*/)
	{
		if (_pendingMask & filter)
		{
			uint32 mask = _pendingMask & filter;

			// 设置视口
			if (mask & PM_VIEWPORT)
			{
				glViewport(_vpX, _vpY, _vpWidth, _vpHeight);
				_pendingMask &= ~PM_VIEWPORT;
			}

			if (mask & PM_VAOBUFF)
			{
				if (_preVaoHandle != _newVaoHandle)
				{
					_bufManager->setCurrentVAO(_newVaoHandle);
				}
				_preVaoHandle = _newVaoHandle;
			}
				
			if (mask & PM_RENDERSTATES)
			{
				applyRenderStates();
				_pendingMask &= ~PM_RENDERSTATES;
			}

			// 设置剪裁矩形
			if (mask & PM_SCISSOR)
			{
				glScissor(_scX, _scY, _scWidth, _scHeight);
				_pendingMask &= ~PM_SCISSOR;
			}

			//// Bind index buffer
			//if (mask & PM_INDEXBUF)
			//{
			//	if (_newIndexBuf != _curIndexBuf)
			//	{
			//		if (_newIndexBuf != 0)
			//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers.getRef(_newIndexBuf).glObj);
			//		else
			//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			//		_curIndexBuf = _newIndexBuf;
			//		_pendingMask &= ~PM_INDEXBUF;
			//	}
			//}

			// 绑定纹理到活动单元并绑定采样状态
			if (mask & PM_TEXTURES)
			{
				_texManager->bindAllActiveTex();
				_pendingMask &= ~PM_TEXTURES;
			}

			//// Bind vertex buffers
			//if (mask & PM_VERTLAYOUT)
			//{
			//	//if( _newVertLayout != _curVertLayout || _curShader != _prevShader )
			//	{
			//		if (!applyVertexLayout())
			//		return false;
			//		_curVertLayout = _newVertLayout;
			//		_prevShaderId = _curShaderId;
			//		_pendingMask &= ~PM_VERTLAYOUT;
			//	}
			//}
		}

		return true;
	}

	void OGLDeviceManager::resetStates()
	{
		//_curIndexBuf = 1; _newIndexBuf = 0;
		//_curVertLayout = 1; _newVertLayout = 0;
		_preRasterState.hash = 0xFFFFFFFF; _newRasterState.hash = 0;
		_preBlendState.hash = 0xFFFFFFFF; _newBlendState.hash = 0;
		_preDepthStencilState.hash = 0xFFFFFFFF; _newDepthStencilState.hash = 0;
		_preStencilState.hash = 0xFFFFFFFF; _newStencilState.hash = 0;
		for (uint32 i = 0; i < 16; ++i)
			setTexture(i, 0, 0);

		setColorWriteMask(true);
		_pendingMask = 0xFFFFFFFF;
		commitStates();

		_bufManager->setCurrentVertexBuffer(0);
		_rendbufManager->setCurRenderBuffer(0);
	}

	void OGLDeviceManager::draw(uint32 primType, uint32 firstVert, uint32 numVerts)
	{
		if (commitStates())
		{
			glDrawArrays(primType, firstVert, numVerts);
		}
	}

	void OGLDeviceManager::drawIndexed(uint32 primType, uint32 count, uint32 datatype)
	{
		if (commitStates())
		{
			glDrawElements(primType, count, datatype, 0);
		}
	}

	void OGLDeviceManager::applyRenderStates()
	{
		// 光栅状态
		if (_newRasterState.hash != _preRasterState.hash)
		{
			if (_newRasterState.fillMode == RS_FILL_SOLID) 
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			else 
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			if (_newRasterState.cullMode == RS_CULL_BACK)
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
			}
			else if (_newRasterState.cullMode == RS_CULL_FRONT)
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
			}
			else
			{
				glDisable(GL_CULL_FACE);
			}

			if (!_newRasterState.scissorEnable) 
				glDisable(GL_SCISSOR_TEST);
			else 
				glEnable(GL_SCISSOR_TEST);

			if (_newRasterState.renderTargetWriteMask) 
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			else 
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			_preRasterState.hash = _newRasterState.hash;
		}

		// 混合设置
		if (_newBlendState.hash != _preBlendState.hash)
		{
			if (!_newBlendState.alphaToCoverageEnable) 
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			else 
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

			if (!_newBlendState.blendEnable)
			{
				glDisable(GL_BLEND);
			}
			else
			{
				uint32 oglBlendFuncs[8] = { GL_ZERO, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_COLOR, GL_ZERO, GL_ZERO };

				glEnable(GL_BLEND);
				glBlendFunc(oglBlendFuncs[_newBlendState.srcBlendFunc], oglBlendFuncs[_newBlendState.destBlendFunc]);
			}

			_preBlendState.hash = _newBlendState.hash;
		}

		// 深度模板
		if (_newDepthStencilState.hash != _preDepthStencilState.hash)
		{
			if (_newDepthStencilState.depthWriteMask) 
				glDepthMask(GL_TRUE);
			else 
				glDepthMask(GL_FALSE);

			if (_newDepthStencilState.depthEnable)
			{
				uint32 oglDepthFuncs[8] = { GL_LEQUAL, GL_LESS, GL_EQUAL, GL_GREATER, GL_GEQUAL, GL_ALWAYS, GL_ALWAYS, GL_ALWAYS };

				glEnable(GL_DEPTH_TEST);
				glDepthFunc(oglDepthFuncs[_newDepthStencilState.depthFunc]);
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
			}

			_preDepthStencilState.hash = _newDepthStencilState.hash;
		}

		//模板缓冲
		if (_preDepthStencilState.hash != _newStencilState.hash)
		{
			if (_newStencilState.StencilEnable)
			{
				glEnable(GL_STENCIL_TEST);
				if (_newStencilState.MaskEnable) 
					glStencilMask(_newStencilState.StencilMask);

				if (_newStencilState.FuncEnable)
				{
					uint32 stencilfunc[] = {GL_NEVER, GL_LESS, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_EQUAL, GL_NOTEQUAL, GL_ALWAYS};
					glStencilFunc(stencilfunc[_newStencilState.StencilFunc], _newStencilState.FuncRef, _newStencilState.FuncMask);
				}
					
				if (_newStencilState.StencilOp)
				{
					uint32 stenciloptype[] = {GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, GL_INVERT};
					glStencilOp(stenciloptype[_newStencilState.StencilFail]
								, stenciloptype[_newStencilState.DepthFail]
								, stenciloptype[_newStencilState.DepthPass]);
				}
			}
			else
			{
				glDisable(GL_STENCIL_TEST);
			}
		}
	}

	void OGLDeviceManager::clear(GLbitfield mask)
	{
		glClear(mask);
	}

	//2016/05/18 add by zhou
	unsigned char * OGLDeviceManager::useScrathBuf(uint32 minSize)
	{
		if (_scratchBufSize < minSize)
		{
			delete[] _scratchBuf;
			_scratchBuf = new unsigned char[minSize + 15];
			_scratchBufSize = minSize;
		}

		return _scratchBuf + (size_t)_scratchBuf % 16;//16字节对齐
	}

	void OGLDeviceManager::clearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
	{
		glClearColor(red, green, blue, alpha);
	}

	void OGLDeviceManager::clearDepth(GLclampd depth)
	{
		glClearDepth(depth);
	}

	void OGLDeviceManager::clearStencil(GLint s)
	{
		glClearStencil(s);
	}





}


