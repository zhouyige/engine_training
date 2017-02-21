#pragma  once
#include "LeanUtil.h"
#include "OGLTextureManager.h"
#include "glus.h"
#include <vector>

namespace Lean3D
{
	class OGLRenderBufferManager
	{
	public:
		OGLRenderBufferManager(OGLTextureManager *texManager);
		~OGLRenderBufferManager();

		void initDefaultFBO();

		uint32 createRenderBuffer(uint32 width, uint32 height, TextureFormats::List format,
			bool depth, uint32 numColBufs, uint32 samples);
		void destroyRenderBuffer(uint32 rbHandle);
		uint32 getRenderBufferTex(uint32 rbHandle, uint32 bufIndex);
		void resolveRenderBuffer(uint32 rbHandle);
		void setCurRenderBuffer(uint32 rbHandle);
		bool getRenderBufferData(uint32 rbHandle, int bufIndex, int *width, int *height,
			int *compCount, void *dataBuffer, int bufferSize);
		
		void setCurRenderTexture(uint32 texHandle);

		int getCurFboWidth();
		int getCurFboHeight();		
		void setCurFboAspect(int width, int height);
		
	private:
		int _defaultFBO;
		int _curFBOwidth, _curFBOheight;
		int _stereoScopicContext;
		uint32 _preRenderBufHandle, _curRenderBufHandle;
		ReferenceObjList< RenderBuffer >  _rendBufsRefList;
		OGLTextureManager *_texManager;

	};


}