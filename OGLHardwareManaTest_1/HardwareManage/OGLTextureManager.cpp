#include "OGLTextureManager.h"
#include "utMath.h"

namespace Lean3D
{
	OGLTextureManager::OGLTextureManager()
	{
	}

	OGLTextureManager::~OGLTextureManager()
	{
	}

	uint32 OGLTextureManager::calcTextureSize(TextureFormats::List format, int width, int height, int depth)
	{
		switch (format)
		{
		case TextureFormats::RGBA8:
			return width * height * depth * 4;
		case TextureFormats::DXT1:
			return static_cast<int>(maxf(width / 4, 1)) * static_cast<int>(maxf(height / 4, 1)) * depth * 8;
		case TextureFormats::DXT3:
			return static_cast<int>(maxf(width / 4, 1)) * static_cast<int>(maxf(height / 4, 1)) * depth * 16;
		case TextureFormats::DXT5:
			return static_cast<int>(maxf(width / 4, 1)) * static_cast<int>(maxf(height / 4, 1)) * depth * 16;
		case TextureFormats::RGBA16F:
			return width * height * depth * 8;
		case TextureFormats::RGBA32F:
			return width * height * depth * 16;
		default:
			return 0;
		}
	}

	uint32 OGLTextureManager::createTexture(TextureType::List type, int width, int height, int depth
											, TextureFormats::List format, bool hasMips, bool genMips
											, bool compress, bool sRGB)
	{
		ASSERT(depth > 0);
		TextureBuffer tex;
		tex.type = type;
		tex.format = format;
		tex.width = width;
		tex.height = height;
		tex.depth = depth;
		tex.sRGB = sRGB;
		tex.genMips = genMips;
		tex.hasMips = hasMips;

		switch (format)
		{
		case TextureFormats::RGBA8:
			tex.glFmt = tex.sRGB ? GL_SRGB8_ALPHA8_EXT : GL_RGBA8;
			break;
		case TextureFormats::DXT1:
			tex.glFmt = tex.sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			break;
		case TextureFormats::DXT3:
			tex.glFmt = tex.sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;
		case TextureFormats::DXT5:
			tex.glFmt = tex.sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		case TextureFormats::RGBA16F:
			tex.glFmt = GL_RGBA16F_ARB;
			break;
		case TextureFormats::RGBA32F:
			tex.glFmt = GL_RGBA32F_ARB;
			break;
		case TextureFormats::DEPTH:
			tex.glFmt = GL_DEPTH_COMPONENT24; //有些老ati显卡支持16位深度FBO
			break;
		case TextureFormats::Unknown:
			tex.glFmt = 0;
			break;
		case TextureFormats::RGBA:
			tex.glFmt = GL_RGBA;
			break;
		default:
			ASSERT(0);
			break;
		}

		glGenTextures(1, &tex.texRef);
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(tex.type, tex.texRef);

		GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		tex.samplerState = 0;
		setSamplerState(tex);

		glBindTexture(tex.type, 0);

		///??????????????????????????????

		tex.memSize = calcTextureSize(format, width, height, depth);
		if (hasMips || genMips) tex.memSize += ftoi_r(tex.memSize * 1.0f / 3.0f);
		if (type == TextureType::List::TexCube) tex.memSize *= 6;
		
		_textureTotalMem += tex.memSize;

		return _texturesReflist.add(tex);
	}

	void OGLTextureManager::setSamplerState(TextureBuffer &tex)
	{
		uint32 state = tex.samplerState;
		uint32 target = tex.type;

		const uint32 magFilters[] = { GL_LINEAR, GL_LINEAR, GL_NEAREST };
		const uint32 minFiltersMips[] = { GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST };
		const uint32 maxAniso[] = { 1, 2, 4, 0, 8, 0, 0, 0, 16 };
		const uint32 wrapModes[] = { GL_CLAMP_TO_EDGE, GL_REPEAT, GL_CLAMP_TO_BORDER };
		
		if (tex.hasMips)
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFiltersMips[(state & SS_FILTER_MASK) >> SS_FILTER_START]);
		else
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, magFilters[(state & SS_FILTER_MASK) >> SS_FILTER_START]);

		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilters[(state & SS_FILTER_MASK) >> SS_FILTER_START]);
		glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso[(state & SS_ANISO_MASK) >> SS_ANISO_START]);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapModes[(state & SS_ADDRU_MASK) >> SS_ADDRU_START]);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapModes[(state & SS_ADDRV_MASK) >> SS_ADDRV_START]);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapModes[(state & SS_ADDRW_MASK) >> SS_ADDRW_START]);

		if (!(state & 0x1000))
		{
			glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		}
	}

	void OGLTextureManager::uploadTextureData(uint32 texHandle, int slice, int mipLevel, const void *pixelsData)
	{
		const  TextureBuffer & tex = _texturesReflist.getRef(texHandle);
		TextureFormats::List format = tex.format;

		glActiveTexture(GL_TEXTURE15);
		glBindTexture(tex.type, tex.texRef);
		
		int inputFormat = GL_RGBA, inputType = GL_UNSIGNED_BYTE;
		bool compressed = (format == TextureFormats::List::DXT1) || (format == TextureFormats::List::DXT3)
														   || (format == TextureFormats::List::DXT5);

		switch (format)
		{
		case TextureFormats::RGBA16F:
			inputFormat = GL_RGBA;
			inputType = GL_FLOAT;
			break;
		case TextureFormats::RGBA32F:
			inputFormat = GL_RGBA;
			inputType = GL_FLOAT;
			break;
		case TextureFormats::DEPTH:
			inputFormat = GL_DEPTH_COMPONENT;
			inputType = GL_FLOAT;
			break;
		}

		int width = fmax(tex.width >> mipLevel, 1), height = fmax(tex.height >> mipLevel, 1);

		if (tex.type == TextureType::List::Tex2D || tex.type == TextureType::List::TexCube)
		{
			int target = (tex.type == TextureType::List::Tex2D) ?
			GL_TEXTURE_2D : (GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice);
			if (compressed)
				glCompressedTexImage2D(target, mipLevel, tex.glFmt, width, height, 0
				, calcTextureSize(format, width, height, 1), pixelsData);
			else
				glTexImage2D(target, mipLevel, tex.glFmt, width, height, 0, inputFormat, inputType, pixelsData);			
		}
		else if (tex.type == TextureType::List::Tex3D)
		{
			int depth = fmax(tex.depth >> mipLevel, 1);
			if (compressed)
				glCompressedTexImage3D(GL_TEXTURE_3D, mipLevel, tex.glFmt, width, height, depth, 0
				, calcTextureSize(format, width, height, depth), pixelsData);
			else
				glTexImage3D(GL_TEXTURE_3D, mipLevel, tex.glFmt, width, height, depth, 0
				, inputFormat, inputType, pixelsData);
		}

		if (tex.genMips && (tex.type != GL_TEXTURE_CUBE_MAP || slice == 5))
		{
			glEnable(tex.type);
			glGenerateMipmapEXT(tex.type);
			glDisable(tex.type);
		}

		glBindTexture(tex.type, 0);

		///???????????????????????????????
	}

	void OGLTextureManager::updateTextureData(uint32 texHandle, int slice, int mipLevel, const void *pixelsData)
	{

	}

	void OGLTextureManager::destroyTexture(uint32 texHandle)
	{
		if (texHandle == 0) return;

		const TextureBuffer &tex = _texturesReflist.getRef(texHandle);
		glDeleteTextures(1, &tex.texRef);

		_textureTotalMem -= tex.memSize;
		_texturesReflist.remove(texHandle);
	}

	void OGLTextureManager::setCurrentTexture(uint32 texHandle)
	{
		const TextureBuffer &tex = _texturesReflist.getRef(texHandle);
		glBindTexture(tex.type, tex.texRef);
	}

	bool OGLTextureManager::getTextureData(uint32 texHandle, int slice, int mipLevel, void *buffer)
	{
		const TextureBuffer &tex = _texturesReflist.getRef(texHandle);

		int target = tex.type == TextureType::List::TexCube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
		if (target == GL_TEXTURE_CUBE_MAP) target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;

		int fmt, type, compressed = 0;
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(tex.type, tex.texRef);

		switch (tex.format)
		{
		case TextureFormats::List::RGBA8:
			fmt = GL_BGRA;
			type = GL_UNSIGNED_BYTE;
			break;
		case TextureFormats::List::DXT1:
		case TextureFormats::List::DXT3:
		case TextureFormats::List::DXT5:
			compressed = 1;
			break;
		case TextureFormats::List::RGBA16F:
		case TextureFormats::List::RGBA32F:
			fmt = GL_RGBA;
			type = GL_FLOAT;
			break;
		default:
			return false;
		};

		if (compressed)
			glGetCompressedTexImage(target, mipLevel, buffer);
		else
			glGetTexImage(target, mipLevel, fmt, type, buffer);

		glBindTexture(tex.type, 0);
		
		///???????????????????????????????????????

		return true;
	}

	void OGLTextureManager::addActiveTexture(uint32 activeUnit, uint32 texHandle, uint16 samplerState)
	{
		ASSERT(activeUnit >= 0 && activeUnit < 16);
		_actTexlist.texHandle[activeUnit] = texHandle;
		_actTexlist.samplerState[activeUnit] = samplerState;
	}

	void OGLTextureManager::bindAllActiveTex()
	{
		for (uint32 i = 0; i < 16; ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			if (_actTexlist.texHandle[i] != 0)
			{
				TextureBuffer &tex = _texturesReflist.getRef(_actTexlist.texHandle[i]);
				glBindTexture(tex.type, tex.texRef);

				//设置纹理状态参数
				if (tex.samplerState != _actTexlist.samplerState[i])
				{
					tex.samplerState = _actTexlist.samplerState[i];
					setSamplerState(tex);
				}
			}
			else
			{
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				glBindTexture(GL_TEXTURE_3D, 0);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
	}

}