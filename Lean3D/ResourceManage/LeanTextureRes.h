#pragma once

#include "LeanUtil.h"
#include "LeanResource.h"

namespace Lean3D
{
	struct TextureElemType
	{
		enum List
		{
			TextureElem = 1200,
			ImageElem,
			TexFormatI,
			TexSliceCountI,
			ImgWidthI,
			ImgHeightI,
			ImgPixelStream,
			TexGlRefI
		};
	};

	class TextureResource : public Resource
	{
	public:
		TextureResource(const std::string &name, int flags);
		TextureResource(const std::string &name, uint32 width, uint32 height, uint32 depth,
						TextureFormats::List fmt, int flags);
		~TextureResource();
		TextureResource *clone();

		void initDefault();
		void release();
		bool load(const char *data, int size);

		static void initFunc();
		static void releaseFunc();
		static Resource *factoryFunc(const std::string &name, int flags)
		{
			return new TextureResource(name, flags);
		}

		TextureType::List getTexType() { return _texType; }
		TextureFormats::List getTexFormat() { return _texFormat; }
		uint32 getWidth() const { return _width; }
		uint32 getHeight() const { return _height; }
		uint32 getDepth() const { return _depth; }
		uint32 getTexHandle() { return _texHandle; }
		uint32 getRendBufHandle() { return _rendbufHandle; }
		bool hasMipMaps() {	return _hasMipMaps; }

		int getElemCount(int elem);
		int getElemParamI(int elem, int elemIdx, int param);
		//getGPU数据到内存
		void *mapStream(int elem, int elemIdx, int stream, bool read, bool write);
		//更新内存数据到GPU
		void unmapStream();

	protected:
		bool isDDS(const char *data, int size);
		bool loadDDS(const char *data, int size);
		bool loadSTBI(const char *data, int size);
		int  getMipCount();

	public:
		static uint32 defTex2DHandle;
		static uint32 defTex3DHandle;
		static uint32 defTexCubeHandle;
		static unsigned char *mappedData;
		static int mappedWriteImage;
	private:
		TextureType::List		_texType;
		TextureFormats::List	_texFormat;
		int						_width, _height, _depth;
		uint32					_texHandle;
		uint32					_rendbufHandle;
		bool					_sRGB;
		bool					_hasMipMaps;
		uint32					_glTexRef;
	};
}