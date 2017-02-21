#include "OGLRenderBufferManager.h"

namespace Lean3D
{
	OGLRenderBufferManager::OGLRenderBufferManager(OGLTextureManager *texManager)
	{
		_defaultFBO = 0;
		_preRenderBufHandle = 0;
		_curRenderBufHandle = 0;
		_stereoScopicContext = 0;
		_curFBOheight = 0;
		_curFBOwidth = 0;
		_texManager = texManager;
		//initDefaultFBO();
	}

	OGLRenderBufferManager::~OGLRenderBufferManager()
	{
	}

	uint32 OGLRenderBufferManager::createRenderBuffer(uint32 width, uint32 height, TextureFormats::List format, bool depth, uint32 numColBufs, uint32 samples)
	{
		if ((format == TextureFormats::List::RGBA16F || format == TextureFormats::List::RGBA32F))
		{
			return 0;
		}

		if (numColBufs > RenderBuffer::MaxColorAttachmentCount) return 0;

		uint32 maxSamples = 0;
		GLint value;
		glGetIntegerv(GL_MAX_SAMPLES_EXT, &value);
		maxSamples = (uint32)value;

		if (samples > maxSamples)
		{
			samples = maxSamples;
			//Modules::log().writeWarning("GPU 不支持采样数");
		}

		RenderBuffer renderbuf;
		renderbuf.width = width;
		renderbuf.height = height;
		renderbuf.samples = samples;

		// 创建帧缓存
		glGenFramebuffersEXT(1, &renderbuf.fbo);
		if (samples > 0) glGenFramebuffersEXT(1, &renderbuf.fboMS);

		if (numColBufs > 0)
		{
			// 绑定颜色缓存
			for (uint32 j = 0; j < numColBufs; ++j)
			{
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fbo);
				// 创建纹理
				uint32 texHandle = _texManager->createTexture(TextureType::List::Tex2D, renderbuf.width, renderbuf.height, 1, format, false, false, false, false);
				ASSERT(texHandle != 0);
				_texManager->uploadTextureData(texHandle, 0, 0, 0x0);
				renderbuf.colTexsHandle[j] = texHandle;
				TextureBuffer &tex = _texManager->_texturesReflist.getRef(texHandle);
				//绑定纹理
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + j, GL_TEXTURE_2D, tex.texRef, 0);

				if (samples > 0)
				{
					glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fboMS);
					// 创建多重采样渲染缓存
					glGenRenderbuffersEXT(1, &renderbuf.colBufs[j]);
					glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuf.colBufs[j]);
					glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, renderbuf.samples, tex.glFmt, renderbuf.width, renderbuf.height);
					// 绑定渲染缓存
					glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + j,
						GL_RENDERBUFFER_EXT, renderbuf.colBufs[j]);
				}
			}

			uint32 buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT,
				GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fbo);
			glDrawBuffers(numColBufs, buffers);

			if (samples > 0)
			{
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fboMS);
				glDrawBuffers(numColBufs, buffers);
			}
		}
		else
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fbo);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			if (samples > 0)
			{
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fboMS);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}
		}

		// 绑定深度缓存
		if (depth)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fbo);
			// 创建深度纹理
			uint32 texHandle = _texManager->createTexture(TextureType::List::Tex2D, renderbuf.width, renderbuf.height, 1, TextureFormats::List::DEPTH, false, false, false, false);
			ASSERT(texHandle != 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
			_texManager->uploadTextureData(texHandle, 0, 0, 0x0);
			renderbuf.depthTexHandle = texHandle;
			TextureBuffer &tex = _texManager->_texturesReflist.getRef(texHandle);
			// 绑定纹理到帧缓存
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, tex.texRef, 0);

			if (samples > 0)
			{
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fboMS);
				// 创建多重采样渲染缓存
				glGenRenderbuffersEXT(1, &renderbuf.depthBuf);
				glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuf.depthBuf);
				glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, renderbuf.samples, GL_DEPTH_COMPONENT24, renderbuf.width, renderbuf.height);
				// 绑定渲染缓存
				glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
					GL_RENDERBUFFER_EXT, renderbuf.depthBuf);
			}
		}

		uint32 renderbufHandle = _rendBufsRefList.add(renderbuf);

		// FBO完整性检查
		bool valid = true;
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fbo);
		uint32 status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _defaultFBO);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) valid = false;

		if (samples > 0)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fboMS);
			status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _defaultFBO);
			if (status != GL_FRAMEBUFFER_COMPLETE_EXT) valid = false;
		}

		if (!valid)
		{
			destroyRenderBuffer(renderbufHandle);
			return 0;
		}

		return renderbufHandle;
	}

	void OGLRenderBufferManager::destroyRenderBuffer(uint32 rbHandle)
	{
		RenderBuffer &renderbuf = _rendBufsRefList.getRef(rbHandle);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _defaultFBO);
		
		//删除DepthBuf
		if (renderbuf.depthTexHandle != 0) _texManager->destroyTexture(renderbuf.depthTexHandle);
		if (renderbuf.depthBuf != 0) glDeleteRenderbuffersEXT(1, &renderbuf.depthBuf);
		renderbuf.depthTexHandle = renderbuf.depthBuf = 0;
		
		//删除Colorbuf
		for (uint32 i = 0; i < RenderBuffer::MaxColorAttachmentCount; ++i)
		{
			if (renderbuf.colTexsHandle[i] != 0) _texManager->destroyTexture(renderbuf.colTexsHandle[i]);
			if (renderbuf.colBufs[i] != 0) glDeleteRenderbuffersEXT(1, &renderbuf.colBufs[i]);
			renderbuf.colTexsHandle[i] = renderbuf.colBufs[i] = 0;
		}

		//删除FBO
		if (renderbuf.fbo != 0) glDeleteFramebuffersEXT(1, &renderbuf.fbo);
		if (renderbuf.fboMS != 0) glDeleteFramebuffersEXT(1, &renderbuf.fboMS);
		renderbuf.fbo = renderbuf.fboMS = 0;

		_rendBufsRefList.remove(rbHandle);
	}

	uint32 OGLRenderBufferManager::getRenderBufferTex(uint32 rbHandle, uint32 bufIndex)
	{
		RenderBuffer &renderbuf = _rendBufsRefList.getRef(rbHandle);

		if (bufIndex < RenderBuffer::MaxColorAttachmentCount)
			return renderbuf.colTexsHandle[bufIndex];
		else if (bufIndex == 32)
			return renderbuf.depthTexHandle;
		else
			return 0;


	}

	void OGLRenderBufferManager::setCurRenderBuffer(uint32 rbHandle)
	{
		if (_curRenderBufHandle != 0) resolveRenderBuffer(_curRenderBufHandle);
		
		_curRenderBufHandle = rbHandle;

		if (_curRenderBufHandle == 0)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _defaultFBO);
			//_stereoScopicContext为0只写入后台左缓存用于单视角
			if (_defaultFBO == 0) glDrawBuffer(_stereoScopicContext == 1 ? GL_BACK_RIGHT : GL_BACK_LEFT);
			glDisable(GL_MULTISAMPLE);
		}
		else
		{	
			//取消所有纹理绑定确保没有FBO绑定
			for (uint32 numTex = 0; numTex < 16; ++numTex)
			{
				glActiveTexture(GL_TEXTURE0 + numTex);
				
				if (_texManager->_actTexlist.texHandle[numTex] != 0)
				{
					TextureBuffer &tex = _texManager->_texturesReflist.getRef(_texManager->_actTexlist.texHandle[numTex]);
					glBindTexture(tex.type, tex.texRef);
					if (tex.samplerState != _texManager->_actTexlist.samplerState[numTex])
					{
						tex.samplerState = _texManager->_actTexlist.samplerState[numTex];
						_texManager->setSamplerState(tex);
					}
				}
				else
				{
					glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
					glBindTexture(GL_TEXTURE_3D, 0);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}

			RenderBuffer &renderbuf = _rendBufsRefList.getRef(_curRenderBufHandle);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fboMS != 0 ? renderbuf.fboMS : renderbuf.fbo);
			ASSERT(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);
			_curFBOwidth = renderbuf.width;
			_curFBOheight = renderbuf.height;

			if (renderbuf.fboMS != 0)
				glEnable(GL_MULTISAMPLE);
			else
				glDisable(GL_MULTISAMPLE);

		}
	}

	bool OGLRenderBufferManager::getRenderBufferData(uint32 rbHandle, int bufIndex, int *width, int *height, int *perPixSize, void *dataBuffer, int bufferSize)
	{
		int format = GL_RGBA;
		int type = GL_FLOAT;
		int x, y, w, h;
		x = y = w = h = 0;

		initDefaultFBO();
		//glPixelStorei(GL_PACK_ALIGNMENT, 4); 先注释 默认对齐为1

		if (rbHandle == 0)
		{
			if (bufIndex != 32 && bufIndex != 0) return false;
			if (width != 0x0) *width = _curFBOwidth;
			if (height != 0x0) *height = _curFBOheight;
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _defaultFBO);
			if (bufIndex != 32) glReadBuffer(GL_BACK_LEFT);
		}
		else
		{
			resolveRenderBuffer(rbHandle);

			RenderBuffer &renderbuf = _rendBufsRefList.getRef(rbHandle);
			if (bufIndex == 32 && renderbuf.depthTexHandle == 0) return false;
			if (bufIndex != 32)
			{
				if ((unsigned)bufIndex >= RenderBuffer::MaxColorAttachmentCount 
					|| renderbuf.colTexsHandle == 0)
				{
					return false;
				}
			}
			if (width != 0x0) *width = renderbuf.width;
			if (height != 0x0) *height = renderbuf.height;

			x = 0; y = 0; w = renderbuf.width; h = renderbuf.height;
			
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderbuf.fbo);
			if (bufIndex != 32) glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + bufIndex);
		}

		if ( bufIndex == 32)
		{
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
		}

		int pixSize = (bufIndex == 32 ? 1 : 4);
		if (perPixSize != 0x0) *perPixSize = pixSize;

		bool readflag = false;
		if (dataBuffer != 0x0 && bufferSize >= w * h * pixSize * 4)
		{
			glFinish();//等待命令队列执行完成
			glReadPixels(x, y, w, h, format, type, dataBuffer);
			readflag = true;
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _defaultFBO);

		return readflag;
	}

	void OGLRenderBufferManager::initDefaultFBO()
	{
		glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &_defaultFBO);
		//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _defaultFBO);
	}

	void OGLRenderBufferManager::resolveRenderBuffer(uint32 rbHandle)
	{
		RenderBuffer &renderbuf = _rendBufsRefList.getRef(rbHandle);

		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, renderbuf.fboMS);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, renderbuf.fbo);

		if (renderbuf.fboMS == 0) return;

		bool depthResolved = false;
		for (uint32 CAnum = 0; CAnum < RenderBuffer::MaxColorAttachmentCount; ++CAnum)
		{
			if (renderbuf.colBufs[CAnum] != 0)
			{
				glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + CAnum);
				glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + CAnum);

				int mask = GL_COLOR_BUFFER_BIT;
				if (!depthResolved && renderbuf.depthBuf != 0)
				{
					mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
					depthResolved = true;
				}
				glBlitFramebufferEXT(0, 0, renderbuf.width, renderbuf.height, 0, 0, renderbuf.width, renderbuf.height, mask, GL_NEAREST);
			}
		}

		if ( !depthResolved && renderbuf.depthBuf != 0)
		{
			glReadBuffer(GL_NONE);
			glDrawBuffer(GL_NONE);
			glBlitFramebufferEXT(0, 0, renderbuf.width, renderbuf.height, 0, 0, renderbuf.width, renderbuf.height
								, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
		}

		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, _defaultFBO);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, _defaultFBO);
	}

	void OGLRenderBufferManager::setCurFboAspect(int width, int height)
	{
		_curFBOheight = height;
		_curFBOwidth = width;
	}

	int OGLRenderBufferManager::getCurFboWidth()
	{
		return _curFBOwidth;
	}

	int OGLRenderBufferManager::getCurFboHeight()
	{
		return _curFBOheight;
	}

	void OGLRenderBufferManager::setCurRenderTexture(uint32 texHandle)
	{
		_texManager->setCurrentTexture(texHandle);
	}

}