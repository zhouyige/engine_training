#include "LeanTextureRes.h"
#include "OGLDeviceManager.h"
#include "Lean3DRoot.h"
#include "utImage.h"
#include <algorithm>


namespace Lean3D
{

#define FOURCC( c0, c1, c2, c3 ) ((c0) | (c1<<8) | (c2<<16) | (c3<<24))
#define DDSD_MIPMAPCOUNT      0x00020000

#define DDPF_ALPHAPIXELS      0x00000001
#define DDPF_FOURCC           0x00000004
#define DDPF_RGB              0x00000040

#define DDSCAPS2_CUBEMAP      0x00000200
#define DDSCAPS2_CM_COMPLETE  (0x00000400 | 0x00000800 | 0x00001000 | 0x00002000 | 0x00004000 | 0x00008000)
#define DDSCAPS2_VOLUME       0x00200000

#define D3DFMT_A16B16G16R16F  113
#define D3DFMT_A32B32G32R32F  116
	struct DDSHeader
	{
		uint32  dwMagic;
		uint32  dwSize;
		uint32  dwFlags;
		uint32  dwHeight, dwWidth;
		uint32  dwPitchOrLinearSize;
		uint32  dwDepth;
		uint32  dwMipMapCount;
		uint32  dwReserved1[11];

		struct {
			uint32  dwSize;
			uint32  dwFlags;
			uint32  dwFourCC;
			uint32  dwRGBBitCount;
			uint32  dwRBitMask, dwGBitMask, dwBBitMask, dwABitMask;
		} pixFormat;

		struct {
			uint32  dwCaps, dwCaps2, dwCaps3, dwCaps4;
		} caps;

		uint32  dwReserved2;
	} ddsHeader;
	
	uint32 TextureResource::defTex2DHandle = 0;
	uint32 TextureResource::defTex3DHandle = 0;
	uint32 TextureResource::defTexCubeHandle = 0;
	unsigned char *TextureResource::mappedData = 0x0;
	int TextureResource::mappedWriteImage = -1;

	TextureResource::TextureResource(const std::string &name, int flags) 
					: Resource( ResourceTypes::Texture, name, flags)
	{
		_texType = TextureType::Tex2D;
		initDefault();
	}

	TextureResource::TextureResource(const std::string &name, uint32 width, uint32 height, uint32 depth
					, TextureFormats::List fmt, int flags) : Resource(ResourceTypes::Texture, name, flags)
					, _width(width), _height(height), _depth(depth), _rendbufHandle(0)
	{
		_texFormat = fmt;

		if (flags & ResourceFlags::TexRenderable)
		{
			_flags &= ~ResourceFlags::TexCubemap;
			_flags &= ~ResourceFlags::TexSRGB;
			_flags |= ResourceFlags::NoTexCompression;
			_flags |= ResourceFlags::NoTexMipmaps;

			_texType = TextureType::Tex2D;
			_sRGB = false;
			_hasMipMaps = false;
			_rendbufHandle = g_OGLDiv->rendbufManaRef()->createRenderBuffer(width, height, fmt, false, 1, 0);
			_texHandle = g_OGLDiv->rendbufManaRef()->getRenderBufferTex(_rendbufHandle, 0);
		}
		else
		{
			uint32 size = g_OGLDiv->texManaRef()->calcTextureSize(_texFormat, width, height, depth);
			unsigned char *pixels = new unsigned char[size];
			memset(pixels, 0, size);

			_texType = flags & ResourceFlags::TexCubemap ? TextureType::TexCube : TextureType::Tex2D;
			if (depth > 1) _texType = TextureType::Tex3D;
			_sRGB = (_flags & ResourceFlags::TexSRGB) != 0;
			_hasMipMaps = !(_flags & ResourceFlags::NoTexMipmaps);
			_texHandle = g_OGLDiv->texManaRef()->createTexture(_texType, _width, _height, _depth, _texFormat
															, _hasMipMaps, _hasMipMaps, false, _sRGB);
			g_OGLDiv->texManaRef()->uploadTextureData(_texHandle, 0, 0, pixels);

			delete[] pixels;
			pixels = 0x0;
			if (_texHandle == 0)
				initDefault();
		}
		if (_texHandle != 0)
			_glTexRef = g_OGLDiv->texManaRef()->getOGLTexRef(_texHandle);
		else
			_glTexRef = 0;
	}

	TextureResource::~TextureResource()
	{

	}

	TextureResource * TextureResource::clone()
	{
		return 0x0;
	}

	void TextureResource::initDefault()
	{
		_rendbufHandle = 0;
		_hasMipMaps = false;
		_sRGB = false;
		_glTexRef = 0;
		_texFormat = TextureFormats::RGBA8;
		_width = 0;
		_height = 0;
		_depth = 0;

		if (_texType == TextureType::TexCube)
			_texHandle = defTexCubeHandle;
		else if (_texType == TextureType::Tex3D)
			_texHandle = defTex3DHandle;
		else 
			_texHandle = defTex2DHandle;
	}

	void TextureResource::release()
	{
		if (_rendbufHandle != 0)
		{
			g_OGLDiv->rendbufManaRef()->destroyRenderBuffer(_rendbufHandle);
		}
		
		if (_texHandle != 0 && _texHandle != defTex2DHandle && _texHandle != defTexCubeHandle)
		{
			g_OGLDiv->texManaRef()->destroyTexture(_texHandle);
		}
		_texHandle = 0;
		_rendbufHandle = 0;
		_glTexRef = 0;
	}

	void TextureResource::initFunc()
	{
		unsigned char texData[] = {
			128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
			128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
			128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
			128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255 };
		//上传默认纹理
		defTex2DHandle = g_OGLDiv->texManaRef()->createTexture(TextureType::Tex2D, 4, 4, 1
										, TextureFormats::RGBA8, true, true, false, false);
		g_OGLDiv->texManaRef()->uploadTextureData(defTex2DHandle, 0, 0, texData);

		defTexCubeHandle = g_OGLDiv->texManaRef()->createTexture(TextureType::TexCube, 4, 4, 1
										, TextureFormats::RGBA8, true, true, false, false);

		for (uint32 i = 0; i < 6; ++i)
		{
			g_OGLDiv->texManaRef()->uploadTextureData(defTexCubeHandle, i, 0, texData);
		}

		unsigned char *tex3DData = new unsigned char[256];
		memcpy(tex3DData, texData, 64);
		memcpy(tex3DData + 64, texData, 64);
		memcpy(tex3DData + 128, texData, 64);
		memcpy(tex3DData + 192, texData, 64);

		defTex3DHandle = g_OGLDiv->texManaRef()->createTexture(TextureType::Tex3D, 4, 4, 4
									, TextureFormats::RGBA8, true, true, false, false);
		g_OGLDiv->texManaRef()->uploadTextureData(defTex3DHandle, 0, 0, tex3DData);
		delete[] tex3DData;
		tex3DData = 0x0;
	}

	void TextureResource::releaseFunc()
	{
		g_OGLDiv->texManaRef()->destroyTexture(defTex2DHandle);
		g_OGLDiv->texManaRef()->destroyTexture(defTex3DHandle);
		g_OGLDiv->texManaRef()->destroyTexture(defTexCubeHandle);
	}

	int TextureResource::getElemCount(int elem)
	{
		switch (elem)
		{
		case TextureElemType::TextureElem:
			return 1;
		case TextureElemType::ImageElem:
			return _texType == TextureType::TexCube ? 6 * (getMipCount() + 1) : getMipCount() + 1;
		default:
			return Resource::getElemCount(elem);
		}
	}

	int TextureResource::getElemParamI(int elem, int elemIdx, int param)
	{
		switch (elem)
		{
		case TextureElemType::TextureElem:
			switch (param)
			{
			case TextureElemType::TexFormatI:
				return _texFormat;
			case TextureElemType::TexSliceCountI:
				return _texType == TextureType::TexCube ? 6 : 1;
			case TextureElemType::TexGlRefI:
				return _glTexRef;
			}
			break;
		case TextureElemType::ImageElem:
			switch (param)
			{
			case TextureElemType::ImgWidthI:
				if (elemIdx < getElemCount(elem))
				{
					int mipLevel = elemIdx % (getMipCount() + 1);
					return std::max(1, _width >> mipLevel);
				}
				break;
			case TextureElemType::ImgHeightI:
				if (elemIdx < getElemCount(elem))
				{
					int mipLevel = elemIdx % (getMipCount() + 1);
					return std::max(1, _height >> mipLevel);
				}
				break;
			}
			break;
		}

		return Resource::getElemParamI(elem, elemIdx, param);
	}

	void * TextureResource::mapStream(int elem, int elemIdx, int stream, bool read, bool write)
	{
		if ((read || write) && mappedData == 0x0)
		{
			if (elem == TextureElemType::ImageElem && stream == TextureElemType::ImgPixelStream
				 && elemIdx < getElemCount(elem))
			{
				mappedData = g_OGLDiv->useScrathBuf(g_OGLDiv->texManaRef()->calcTextureSize(
												_texFormat, _width, _height, _depth));
				if (read)
				{
					int slice = elemIdx / (getMipCount() + 1);
					int mipLevel = elemIdx % (getMipCount() + 1);
					g_OGLDiv->texManaRef()->getTextureData(_texHandle, slice, mipLevel, mappedData);
				}
				if (write)
					mappedWriteImage = elemIdx;
				else
					mappedWriteImage = -1;
				return mappedData;
			}
		}
		return Resource::mapStream(elem, elemIdx, stream, read, write);
	}

	void TextureResource::unmapStream()
	{
		if (mappedData != 0x0)
		{
			if (mappedWriteImage >= 0)
			{
				int slice = mappedWriteImage / (getMipCount() + 1);
				int mipLevel = mappedWriteImage % (getMipCount() + 1);
				g_OGLDiv->texManaRef()->updateTextureData(_texHandle, slice, mipLevel, mappedData);
				mappedWriteImage = -1;
			}

			mappedData = 0x0;
			return;
		}

		Resource::unmapStream();
	}

	bool TextureResource::isDDS(const char *data, int size)
	{
		return size > 128 && *((uint32*)data) == FOURCC('D', 'D', 'S', ' ');
	}

	bool TextureResource::loadDDS(const char *data, int size)
	{
		ASSERT_STATIC(sizeof(DDSHeader) == 128);

		memcpy(&ddsHeader, data, 128);

		if (ddsHeader.dwSize != 124)
		{
			LEAN_DEGUG_LOG(" DDS 文件头错误！", 0);
			return false;
		}

		//保存属性
		_width = ddsHeader.dwWidth;
		_height = ddsHeader.dwHeight;
		_depth = 1;
		_texFormat = TextureFormats::Unknown;
		_texHandle = 0;
		_sRGB = (_flags & ResourceFlags::TexSRGB) != 0;
		int mipCount = ddsHeader.dwFlags & DDSD_MIPMAPCOUNT ? ddsHeader.dwMipMapCount : 1;
		_hasMipMaps = mipCount > 1 ? true : false;

		//获取纹理类型
		if (ddsHeader.caps.dwCaps2 == 0)
		{
			_texType = TextureType::Tex2D;
		}
		else if (ddsHeader.caps.dwCaps2 & DDSCAPS2_CUBEMAP)
		{
			if ((ddsHeader.caps.dwCaps2 & DDSCAPS2_CM_COMPLETE) != DDSCAPS2_CM_COMPLETE)
			{
				LEAN_DEGUG_LOG("CubeMap 不包含所有切面！", 0);
				//return false;
			}
			_texType = TextureType::TexCube;
		}
		else if (ddsHeader.caps.dwCaps2 & DDSCAPS2_VOLUME)
		{
			_depth = ddsHeader.dwDepth;
			_texType = TextureType::Tex3D;
		}
		else
		{
			LEAN_DEGUG_LOG("不支持的DDS类型！", 0);
			return false;
		}

		//获取像素格式
		int blockSize = 1, bytesPerBlock = 4;
		enum { pfBGRA, pfBGR, pfBGRX, pfRGB, pfRGBX, pfRGBA } pixFmt = pfBGRA;

		if (ddsHeader.pixFormat.dwFlags & DDPF_FOURCC)
		{
			switch (ddsHeader.pixFormat.dwFourCC)
			{
			case FOURCC('D', 'X', 'T', '1'):
				_texFormat = TextureFormats::DXT1;
				blockSize = 4;
				bytesPerBlock = 8;
				break;
			case FOURCC('D', 'X', 'T', '3'):
				_texFormat = TextureFormats::DXT3;
				blockSize = 4;
				bytesPerBlock = 16;
				break;
			case FOURCC('D', 'X', 'T', '5'):
				_texFormat = TextureFormats::DXT5;
				blockSize = 4;
				bytesPerBlock = 16;
				break;
			case D3DFMT_A16B16G16R16F:
				_texFormat = TextureFormats::RGBA16F;
				bytesPerBlock = 8;
				break;
			case D3DFMT_A32B32G32R32F:
				_texFormat = TextureFormats::RGBA32F;
				bytesPerBlock = 16;
				break;
			default:
				break;
			}
		}
		else if (ddsHeader.pixFormat.dwFlags & DDPF_RGB)
		{
			bytesPerBlock = ddsHeader.pixFormat.dwRGBBitCount / 8;

			if (ddsHeader.pixFormat.dwRBitMask == 0x00ff0000 && 
				ddsHeader.pixFormat.dwGBitMask == 0x0000ff00 &&
				ddsHeader.pixFormat.dwBBitMask == 0x000000ff ) 
			{
				pixFmt = pfBGR;
			}
			else if (ddsHeader.pixFormat.dwRBitMask == 0x000000ff &&
					ddsHeader.pixFormat.dwGBitMask == 0x0000ff00 &&
					ddsHeader.pixFormat.dwBBitMask == 0x00ff0000)

			{
				pixFmt = pfRGB;
			}

			if (pixFmt == pfBGR || pixFmt == pfRGB)
			{
				if (ddsHeader.pixFormat.dwRGBBitCount == 24)
				{
					_texFormat = TextureFormats::RGBA8;
				}
				else if (ddsHeader.pixFormat.dwRGBBitCount == 32)
				{
					if (!(ddsHeader.pixFormat.dwFlags & DDPF_ALPHAPIXELS) ||
						ddsHeader.pixFormat.dwABitMask == 0x00000000)
					{
						_texFormat = TextureFormats::RGBA8;
						pixFmt = pixFmt == pfBGR ? pfBGRX : pfRGBX;
					}
					else
					{
						_texFormat = TextureFormats::RGBA8;
						pixFmt = pixFmt == pfBGR ? pfBGRA : pfRGBA;
					}
				}
			}
		}
		if (_texFormat == TextureFormats::Unknown)
		{
			LEAN_DEGUG_LOG("不支持的dds像素格式！", 0);
			return false;
		}

		_texHandle = g_OGLDiv->texManaRef()->createTexture(_texType, _width, _height, _depth
												, _texFormat, mipCount > 1, false, false, _sRGB);

		int  slicesCount = _texType == TextureType::TexCube ? 6 : 1;
		unsigned char *pixels = (unsigned char*)(data + 128);

		for (int i = 0; i < slicesCount; ++i)
		{
			int width = _width;
			int height = _height;
			int depth = _depth;
			uint32 *dstBuf = 0x0;

			for (int j = 0; j < mipCount; ++j)
			{
				size_t mipSize = std::max(width / blockSize, 1) * std::max(height / blockSize, 1)
									* depth * bytesPerBlock;
				if (pixels + mipSize > (unsigned char*)data + size)
				{
					LEAN_DEGUG_LOG("错误dds文件！", 0);
					return false;
				}

				if (_texFormat == TextureFormats::RGBA8 && pixFmt != pfBGRA)
				{
					//转换8位DDS格式到BGRA
					uint32 pixCount = width * height * depth;
					if (dstBuf == 0x0)  dstBuf = new uint32[pixCount * 4];
					uint32 *p = dstBuf;
					
					if (pixFmt == pfBGR)
					for (uint32 k = 0; k < pixCount * 3; k += 3)
						*p++ = pixels[k + 0] | pixels[k + 1] << 8 | pixels[k + 2] << 16 | 0xFF000000;
					else if (pixFmt == pfBGRX)
					for (uint32 k = 0; k < pixCount * 4; k += 4)
						*p++ = pixels[k + 0] | pixels[k + 1] << 8 | pixels[k + 2] << 16 | 0xFF000000;
					else if (pixFmt == pfRGB)
					for (uint32 k = 0; k < pixCount * 3; k += 3)
						*p++ = pixels[k + 2] | pixels[k + 1] << 8 | pixels[k + 0] << 16 | 0xFF000000;
					else if (pixFmt == pfRGBX)
					for (uint32 k = 0; k < pixCount * 4; k += 4)
						*p++ = pixels[k + 2] | pixels[k + 1] << 8 | pixels[k + 0] << 16 | 0xFF000000;
					else if (pixFmt == pfRGBA)
					for (uint32 k = 0; k < pixCount * 4; k += 4)
						*p++ = pixels[k + 2] | pixels[k + 1] << 8 | pixels[k + 0] << 16 | pixels[k + 3] << 24;
					
					g_OGLDiv->texManaRef()->uploadTextureData(_texHandle, i, j, dstBuf);
				}
				else
				{
					// 直接导入DDS数据到GPU
					g_OGLDiv->texManaRef()->uploadTextureData(_texHandle, i, j, pixels);
				}

				pixels += mipSize;
				if (width > 1) width >>= 1;
				if (height > 1) height >>= 1;
				if (depth > 1) depth >>= 1;
			}

			if (dstBuf != 0x0) delete[] dstBuf;
			dstBuf = 0x0;
		}
		
		ASSERT(pixels == (unsigned char *)data + size);
		
		return true;
	}

	bool TextureResource::loadSTBI(const char *data, int size)
	{

		bool hdr = false;

		if (stbi_is_hdr_from_memory((unsigned char*)data, size) > 0) hdr = true;
		
		int comps;
		void *pixels = 0x0;
		if (hdr)
			pixels = stbi_loadf_from_memory((unsigned char*)data, size, &_width, &_height, &comps, 4);
		else
			pixels = stbi_load_from_memory((unsigned char*)data, size, &_width, &_height, &comps, 4);
	
		if (pixels == 0x0)
		{
			LEAN_DEGUG_LOG("未定义Image格式！", 0);
			return false;
		}

		uint32 *ptr = (uint32 *)pixels;
		for (uint32 i = 0, si = _width * _height; i < si; ++i)
		{
			uint32 col = *ptr;
			*ptr++ = (col & 0xFF00FF00) | ((col & 0x000000FF) << 16) | ((col & 0x00FF0000) >> 16);
		}
		_depth = 1;
		_texType = TextureType::Tex2D;
		_texFormat = hdr ? TextureFormats::RGBA16F : TextureFormats::RGBA8;  //其实数据是BGRA
		_sRGB = (_flags & ResourceFlags::TexSRGB) != 0;
		_hasMipMaps = !(_flags & ResourceFlags::NoTexMipmaps);

		_texHandle = g_OGLDiv->texManaRef()->createTexture(_texType, _width, _height, _depth, _texFormat,
							_hasMipMaps, _hasMipMaps, !(_flags & ResourceFlags::NoTexCompression), _sRGB);
		g_OGLDiv->texManaRef()->uploadTextureData(_texHandle, 0, 0, pixels);

		stbi_image_free(pixels);

		return true;
	}

	int TextureResource::getMipCount()
	{
		if (_hasMipMaps)
		{
			return ftoi_t(log((float)std::max(_width, _height)) / log(2.0f));
		}
		else
			return 0;
	}

	bool TextureResource::load(const char *data, int size)
	{
		if (!Resource::load(data, size)) return false;

		if (isDDS(data, size))
		{
			return loadDDS(data, size);
		}
		else
		{
			return loadSTBI(data, size);
		}
	}

}