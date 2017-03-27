#pragma once
#include "LeanUtil.h"

namespace Lean3D
{
	struct ResourceTypes
	{
		enum List
		{
			Undefined = 0,
			SceneGraph,
			Geometry,
			Animation,
			Material,
			Code,
			Shader,
			Texture,
			ParticleEffect,
			Pipeline
		};
	};

	struct ResourceFlags
	{
		enum Flags
		{
			NoQuery = 1,
			NoTexCompression = 2,
			NoTexMipmaps = 4,
			TexCubemap = 8,
			TexDynamic = 16,
			TexRenderable = 32,
			TexSRGB = 64
		};
	};

	class Resource
	{
	public:
		Resource( int type , const std::string &name, int flag );
		virtual ~Resource();

		virtual Resource *clone();

		//this two function should not be virtual
		//design problem
		virtual void initDefault();
		virtual void release();

		virtual bool load(const char *data, int size);
		void unload();

		int findElem(int elem, int param, const char *value);
		virtual int getElemCount(int elem);
		virtual int getElemParamI(int elem, int elemIdx, int param);
		virtual void setElemParamI(int elem, int elemIdx, int param, int value);
		virtual float getElemParamF(int elem, int elemIdx, int param, int compIdx);
		virtual void setElemParamF(int elem, int elemIdx, int param, int compIdx, float value);
		virtual const char *getElemParamStr(int elem, int elemIdx, int param);
		virtual void setElemParamStr(int elem, int elemIdx, int param, const char *value);
		virtual void *mapStream(int elem, int elemIdx, int stream, bool read, bool write);
		virtual void unmapStream();

		inline int &getType() { return _type; }
		inline int getFlags() { return _flags; }
		inline const std::string &getName() { return _name; }
		inline ResHandle getHandle() { return _handle; }
		inline void setHandle(ResHandle handle) { _handle = handle; }
		inline bool isLoaded() { return _loaded; }
		inline void addRef() { ++_refCount; }
		inline void subRef() { --_refCount; }
		inline uint32 getUserRefCount() { return _userRefCount; }
		inline void addUserRefCount() { ++_userRefCount; }
		inline void subUserRefCount() { --_userRefCount; }
		inline bool isNoQuery() { return _noQuery; }
	protected:
			int                  _type;
			std::string          _name;
			ResHandle            _handle;
			int                  _flags;

			uint32               _refCount;  //被其他资源引用的引用计数
			uint32               _userRefCount;  // handle数

			bool                 _loaded;
			bool                 _noQuery;

	};

}