#pragma once 
#include "LeanPrerequisites.h"
#include "LeanPrimitives.h"
#include "utMath.h"
#include <vector>
#include <string>
#include <algorithm>

namespace Lean3D
{
	//对象列表，可以减少对象频繁释放，申请的消耗
	template<class T> class ReferenceObjList
	{
	public:
		uint32 add(const T &obj)
		{
			if (!_freeList.empty())
			{
				uint32 index = _freeList.back();
				_freeList.pop_back();
				_objectList[index] = obj;
				return index + 1;
			}
			else
			{
				_objectList.push_back(obj);
				return (uint32)_objectList.size();
			}
		}

		void remove(uint32 handle)
		{
			ASSERT(handle > 0 && handle <= _objectList.size());
			_objectList[handle - 1] = T();
			_freeList.push_back(handle - 1);

		}

		void clearAll()
		{
			_freeList.clear();
			_objectList.clear();
		}

		T &getRef(uint32 handle)
		{
			ASSERT(handle > 0 && handle <= _objectList.size());
			return _objectList[handle - 1];
		}

	private:
		std::vector<T> _objectList;
		std::vector<uint32> _freeList;
	};

	//引用计数智能指针
	template<class T> class ReferenceCountPtr
	{
	public:
		ReferenceCountPtr(T *ptr = 0x0) : _ptr(ptr) { addRef(); }
		ReferenceCountPtr(const ReferenceCountPtr &rcp) : _ptr(rcp._ptr) { addRef(); }
		~ReferenceCountPtr() { subRef(); }

		T &operator*() const { return *_ptr; }
		T *operator->() const { return _ptr; }
		operator T*() const  { return _ptr; }
		operator const T*() const { return _ptr; }
		operator bool() const { return _ptr != 0x0; }
		T *getPtr() { return _ptr; }
		ReferenceCountPtr &operator= (const ReferenceCountPtr &rcp) { return *this = rcp._ptr; }
		ReferenceCountPtr &operator= (T *ptr)
		{
			subRef(); _ptr = ptr; addRef();
			return *this;
		}

	private:
		void addRef() { if (_ptr != 0x0) _ptr->addRef(); }
		void subRef() { if (_ptr != 0x0) _ptr->subRef(); }
	private:
		T *_ptr;
	};

	//词法解析
	class Tokenizer
	{
	public:
		Tokenizer(const char *data) : _p(data), _line(1) { getNextToken(); }
		~Tokenizer() 
		{
			_p = 0x0;
			_line = 0;
		}
		int getLine() { return _line; }
		bool hasToken() { return _token[0] != '\0'; }

		//检查词符是否匹配
		bool checkToken(const char *token, bool peekOnly = false)
		{
			if (_stricmp(_token, token) == 0)
			{
				if (!peekOnly) getNextToken();
				return true;
			}
			return false;
		}

		const char *getToken(const char *charset)
		{
			if (charset)
			{
				const char *p = _token;
				while (*p)
				{
					if (strchr(charset, *p++) == 0x0)
					{
						_preToken[0] = '\0';
						return _preToken;
					}
				}
			}
			memcpy(_preToken, _token, tokenSize);
			getNextToken();
			return _preToken;
		}
		
		bool seekToken(const char *token)
		{
			while (_stricmp(getToken(0x0), token) != 0)
			{
				if (!hasToken()) return false;
			}
			return true;
		}

	protected:
		void checkLineChange()
		{
			if (*_p == '\r' && *(_p + 1) == '\n')
			{
				++_p;
				++_line;
			}
			else if (*_p == '\r' || *(_p + 1) == '\n')
			{
				++_line;
			}
		}

		void ship(const char *chars)
		{
			while (*_p)
			{
				if (!strchr(chars, *_p)) break;	

				checkLineChange();
				++_p;
			}
		}
		
		bool seekChar(const char *chars)
		{
			while (*_p)
			{
				if (strchr(chars, *_p)) break;
				checkLineChange();
				++_p;
			}
			return *_p != '\0';
		}
		
		void getNextToken()
		{
			//跳过空白
			ship(" \t\n\r");
			//解析词符
			const char *p0 = _p, *p1 = _p;
			
			if (*_p == '"')
			{
				++_p; ++p0;
				if (seekChar("\"\n\r")) p1 = _p++;
			}
			else
			{
				seekChar(" \t\n\r{}()<>=,;");
				if (_p == p0 && *_p != '\0') ++_p;
				p1 = _p;
			}

			memcpy(_token, p0, std::min((int)(p1 - p0), tokenSize - 1));
			_token[std::min((int)(p1 - p0), tokenSize - 1)] = '\0';
		}

	private:
		static const int tokenSize = 128;
		char _token[tokenSize], _preToken[tokenSize];
		const char *_p;
		int _line;
	};

	//纹理
	struct TextureType
	{
		enum List
		{
			Tex2D = GL_TEXTURE_2D,
			Tex3D = GL_TEXTURE_3D,
			TexCube = GL_TEXTURE_CUBE_MAP
		};
	};

	struct TextureFormats
	{
		enum List
		{
			Unknown,
			RGBA8,
			DXT1,
			DXT3,
			DXT5,
			RGBA16F,
			RGBA32F,
			DEPTH,
			RGBA
		};
	};

	struct TextureBuffer
	{
		uint32                texRef;
		uint32                glFmt;
		int                   type;
		TextureFormats::List		  format;
		int                   width, height, depth;
		int                   memSize;
		uint32                samplerState;
		bool                  sRGB;
		bool                  hasMips, genMips;
	};


	enum SamplerState
	{
		SS_FILTER_BILINEAR = 0x0,
		SS_FILTER_TRILINEAR = 0x0001,
		SS_FILTER_POINT = 0x0002,
		SS_ANISO1 = 0x0,
		SS_ANISO2 = 0x0004,
		SS_ANISO4 = 0x0008,
		SS_ANISO8 = 0x0010,
		SS_ANISO16 = 0x0020,
		SS_ADDRU_CLAMP = 0x0,
		SS_ADDRU_WRAP = 0x0040,
		SS_ADDRU_CLAMPCOL = 0x0080,
		SS_ADDRV_CLAMP = 0x0,
		SS_ADDRV_WRAP = 0x0100,
		SS_ADDRV_CLAMPCOL = 0x0200,
		SS_ADDRW_CLAMP = 0x0,
		SS_ADDRW_WRAP = 0x0400,
		SS_ADDRW_CLAMPCOL = 0x0800,
		SS_ADDR_CLAMP = SS_ADDRU_CLAMP | SS_ADDRV_CLAMP | SS_ADDRW_CLAMP,
		SS_ADDR_WRAP = SS_ADDRU_WRAP | SS_ADDRV_WRAP | SS_ADDRW_WRAP,
		SS_ADDR_CLAMPCOL = SS_ADDRU_CLAMPCOL | SS_ADDRV_CLAMPCOL | SS_ADDRW_CLAMPCOL,
		SS_COMP_LEQUAL = 0x1000
	};

	struct ActiveTexureList
	{
		uint32 samplerState[16];
		uint32 texHandle[16];
		ActiveTexureList()
		{
			for (int i = 0; i < 16; ++i)
			{
				samplerState[i] = 0;
				texHandle[i] = 0;
			}
		}
	};

	const uint32 SS_FILTER_START = 0;
	const uint32 SS_FILTER_MASK = SS_FILTER_BILINEAR | SS_FILTER_TRILINEAR | SS_FILTER_POINT;
	const uint32 SS_ANISO_START = 2;
	const uint32 SS_ANISO_MASK = SS_ANISO1 | SS_ANISO2 | SS_ANISO4 | SS_ANISO8 | SS_ANISO16;
	const uint32 SS_ADDRU_START = 6;
	const uint32 SS_ADDRU_MASK = SS_ADDRU_CLAMP | SS_ADDRU_WRAP | SS_ADDRU_CLAMPCOL;
	const uint32 SS_ADDRV_START = 8;
	const uint32 SS_ADDRV_MASK = SS_ADDRV_CLAMP | SS_ADDRV_WRAP | SS_ADDRV_CLAMPCOL;
	const uint32 SS_ADDRW_START = 10;
	const uint32 SS_ADDRW_MASK = SS_ADDRW_CLAMP | SS_ADDRW_WRAP | SS_ADDRW_CLAMPCOL;
	const uint32 SS_ADDR_START = 12;
	const uint32 SS_ADDR_MASK = SS_ADDR_CLAMP | SS_ADDR_WRAP | SS_ADDR_CLAMPCOL;

	//VAO相关数据
	struct VertexAttribLayout
	{
		std::string  locationName;
		uint32 attribLoc;
		uint32 unitSize; //一个单位为float
		uint32 vboType;
		uint32 vboHandle;
		uint32 offset;
		uint32 stride;
		VertexAttribLayout() :stride(0), offset(0), vboHandle(0), vboType(0), unitSize(0)
		{}
	};

	struct VertexAttribLayoutList
	{
		uint32 numAttribs;
		uint32 vaoHandle;
		VertexAttribLayout attribs[16];//一个shader最多16个attrib变量
		void clear()
		{
			numAttribs = 0;
			vaoHandle = 0;
			for (int i = 0; i < 16; ++i)
			{
				attribs[i].attribLoc = 0;
				attribs[i].locationName = "";
				attribs[i].offset = 0;
				attribs[i].stride = 0;
				attribs[i].unitSize = 0;
				attribs[i].vboHandle = 0;
				attribs[i].vboType = 0;
			}
		}
	};

	//顶点和索引缓存
	struct VIBuffer
	{
		uint32 bufRef;
		uint32 bufferSize;
		uint32 bufferType;
		uint32 drawType;
	};

	//渲染缓存
	struct RenderBuffer
	{
		static const uint32 MaxColorAttachmentCount = 4;

		uint32  fbo, fboMS;  // fboMS: 当samples大于0用于fbo混合
		uint32  width, height;
		uint32  samples;

		uint32  depthTexHandle, colTexsHandle[MaxColorAttachmentCount];
		uint32  depthBuf, colBufs[MaxColorAttachmentCount];  // 用于多重纹理

		RenderBuffer() : fbo(0), fboMS(0), width(0), height(0), depthTexHandle(0), depthBuf(0)
		{
			for (uint32 i = 0; i < MaxColorAttachmentCount; ++i) colTexsHandle[i] = colBufs[i] = 0;
		}
	};


	// scene

	const int ROOTNODE = 1;

	enum class SceneNodeType
	{
		Undefined = 0,
		Group,
		Model,
		Mesh,
		Joint,
		Light,
		Camera,
		Emitter
	};

	struct SceneNodeFlags
	{
		enum List
		{
			NoDraw = 0x1,
			NoCastShadow = 0x2,
			NoRayQuery = 0x4,
			Inactive = 0x7  // NoDraw | NoCastShadow | NoRayQuery
		};
	};

	enum class RenderingOrder
	{
		None = 0,
		FrontToBack,
		BackToFront,
		StateChanges
	};
}