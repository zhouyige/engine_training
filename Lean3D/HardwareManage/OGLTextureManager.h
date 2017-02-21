#pragma once
#include "LeanUtil.h"

namespace Lean3D
{
	
	class OGLTextureManager
	{
	public:
		OGLTextureManager();
		~OGLTextureManager();

		uint32 createTexture(TextureType::List type, int width, int height, int depth,TextureFormats::List format
			, bool hasMips, bool genMips, bool compress, bool sRGB);
		void uploadTextureData(uint32 texHandle, int slice, int mipLevel, const void *pixelsData);
		void updateTextureData(uint32 texHandle, int slice, int mipLevel, const void *pixelsData);
		void destroyTexture(uint32 texHandle);
		void setCurrentTexture(uint32 texHandle);
		bool getTextureData(uint32 texHandle, int slice, int mipLevel, void *buffer);
		uint32 getTextureTotalMem() { return _textureTotalMem; }

		//���ð�����Active����Ԫ
		void addActiveTexture(uint32 activeUnit, uint32 texHandle, uint16 samplerState);
		//�󶨲���������״̬
		void bindAllActiveTex();

		//��ȡ������opengl�е�����ֵ
		uint32 getOGLTexRef(uint32 handle);
		
		uint32 calcTextureSize(TextureFormats::List format, int width, int height, int depth);
		friend class OGLRenderBufferManager;
	protected:
		
		void setSamplerState(TextureBuffer &tex);

	protected:
		ActiveTexureList _actTexlist;
		uint32        _textureTotalMem; //����ռ���ܿռ� 
		ReferenceObjList< TextureBuffer >       _texturesReflist;
	};


}