#pragma once

#include "LeanPrerequisites.h"
#include <map>

namespace Lean3D
{
	class StatManager;
	class SceneManager;
	class ResourceManager;
	class OGLDeviceManager;

	extern OGLDeviceManager *g_OGLDiv;

	class LeanRoot
	{
	public:
		static bool init();
		static void release();

		static OGLDeviceManager &oglDMRef() { return *_oglDM; }
		static ResourceManager &resMana() { return *_resourceManager; }
		static SceneManager	   &sceneMana() { return *_sceneManager; }

		//
		//2016/05/19 add by zhou
		static std::string cleanPath(std::string path);
		static void setResourcePath(int type, const char *path);
		static bool loadResourceFromPath(const char *contentPath);
		static bool loadResource(ResHandle handle, const char *data, int size);
		static int getResourceType(ResHandle handle);
		static const char *getResourceName(ResHandle handle);
		//2016/05/25 add by zhou
		static const char *getCurContPath() { return _curContentPath; }

		//2016/05/23 add by zhou
		static bool setMatrialUniform(ResHandle handle, const char *name, float a, float b
									, float c, float d);
		static bool setMatrialUniform(ResHandle handle, const char *name, float a[4]);
	private:
		static OGLDeviceManager           *_oglDM;
		static ResourceManager			  *_resourceManager;
		static SceneManager				  *_sceneManager;
		//
		//2016/05/19 add by zhou
		static std::map< int, std::string >  _resourcePaths;
		//2016/05/25 add by zhou
		static const char *_curContentPath;
	};

	
}