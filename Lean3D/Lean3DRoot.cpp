#include "Lean3DRoot.h"
#include "OGLDeviceManager.h"
#include "LeanResourceManager.h"
#include "LeanAnimationRes.h"
#include "LeanGeometryRes.h"
#include "LeanMaterialRes.h"
#include "LeanShaderRes.h"
#include "LeanTextureRes.h"
#include "LeanSceneGraphRes.h"

#include <vector>
#include <fstream>
#include <ios>

namespace Lean3D
{

	OGLDeviceManager *g_OGLDiv = 0x0;
	OGLDeviceManager *LeanRoot::_oglDM = 0x0;
	ResourceManager  *LeanRoot::_resourceManager = 0x0;
	std::map<int, std::string> LeanRoot::_resourcePaths;

	bool LeanRoot::init()
	{
	
		if (_oglDM == 0x0) _oglDM = new OGLDeviceManager();
		g_OGLDiv = _oglDM;

		if (_resourceManager == 0x0) _resourceManager = new ResourceManager();

		if (!_oglDM->init()) return false;

		_resourceManager->registerResType(ResourceTypes::SceneGraph, "SceneGraph", 0x0, 0x0
			, SceneGraphResource::factoryFunc);
		_resourceManager->registerResType(ResourceTypes::Geometry, "Geometry", GeometryResource::initFunc
										, GeometryResource::releaseFunc, GeometryResource::factoryFunc);
		_resourceManager->registerResType(ResourceTypes::Animation, "Animation",0x0, 0x0
										, AnimationResource::factoryFunc);
		_resourceManager->registerResType(ResourceTypes::Code, "Code", 0x0, 0x0, CodeResource::factoryFunc);
		_resourceManager->registerResType(ResourceTypes::Shader, "Shader", 0x0, 0x0
										, ShaderResource::factoryFunc);
		_resourceManager->registerResType(ResourceTypes::Texture, "Texture", TextureResource::initFunc
										, TextureResource::releaseFunc, TextureResource::factoryFunc);
		_resourceManager->registerResType(ResourceTypes::Material, "Material", 0x0, 0x0
										, MaterialResource::factoryFunc);

		//创建默认的资源
		TextureResource *tex2DRes = new TextureResource("$Tex2D", 32, 32, 1
										, TextureFormats::RGBA8, ResourceFlags::NoTexMipmaps);
		void *image = tex2DRes->mapStream(TextureElemType::ImageElem, 0, TextureElemType::ImgPixelStream
								, false, true);
		ASSERT(image != 0x0);
		for (uint32 i = 0; i < 32 * 32; ++i)
			((uint32*)image)[i] = 0xffffffff;
		tex2DRes->unmapStream();
		tex2DRes->addRef();
		_resourceManager->addNonExistingResource(*tex2DRes, false);

		TextureResource *texCubeRes = new TextureResource("$TexCube", 32, 32, 1
											, TextureFormats::RGBA8
											, ResourceFlags::TexCubemap 
											| ResourceFlags::NoTexMipmaps);
		for (uint32 i = 0; i < 6; ++i)
		{
			image = texCubeRes->mapStream(TextureElemType::ImageElem, i
								, TextureElemType::ImgPixelStream, false, true);
			ASSERT(image != 0x0);
			for (uint32 j = 0; j < 32 * 32; ++j) ((uint32*)image)[j] = 0x0ff000000;
			texCubeRes->unmapStream();
		}
		
		texCubeRes->addRef();
		_resourceManager->addNonExistingResource(*texCubeRes, false);

		TextureResource *tex3DRes = new TextureResource("$Tex3D", 16, 16, 4, TextureFormats::RGBA8
										, ResourceFlags::NoTexMipmaps);
		image = tex3DRes->mapStream(TextureElemType::ImageElem, 0, TextureElemType::ImgPixelStream
									, false, true);
		ASSERT(image != 0x0);
		for (uint32 i = 0; i < 16 * 16 * 4; ++i) ((uint32 *)image)[i] = 0xffffffff;
		tex3DRes->unmapStream();
		tex3DRes->addRef();
		_resourceManager->addNonExistingResource(*tex3DRes, false);
		
		//
		_resourcePaths.clear();
		return false;
	}


	void LeanRoot::release()
	{
		delete _oglDM; g_OGLDiv = 0x0; _oglDM = 0x0;

		delete _resourceManager; _resourceManager = 0x0;
	}


	std::string LeanRoot::cleanPath(std::string path)
	{
		//去掉路径前面的空格
		int count = 0;
		for (int i = 0; i < (int)path.length(); ++i)
		{
			if (path[i] != ' ')break;
			else ++count;
		}
		if (count > 0) path.erase(0, count);
		//去掉字符串末尾的斜杠和反斜杠还有空格
		count = 0;
		for (int i = (int)path.length() - 1; i >= 0; --i)
		{
			if (path[i] != '/' && path[i] != '\\' && path[i] != ' ')
				break;
			else
				++count;
		}
		if (count > 0)
			path.erase(path.length() - count, count);

		return path;
	}

	//设置不同资源类型的存放文件夹路径
	void LeanRoot::setResourcePath(int type, const char *path)
	{
		std::string s = path != 0x0 ? path : "";

		_resourcePaths[type] = cleanPath(s);
	}

	bool LeanRoot::loadResourceFromPath(const char *contentPath)
	{
		bool result = true;  //返回值，所有文件读取成功标志

		std::string dir;
		std::vector<std::string> dirs;
		//分割路径字符串
		char *c = (char*)contentPath;
		do 
		{
			if (*c != '|' && *c != '\0')
				dir += *c;
			else
			{
				dir = cleanPath(dir);
				if (dir != "") dir += '/';
				dirs.push_back(dir);
				dir = "";
			}
		} while (*c++ != '\0');

		//找到第一个需要被读取的资源
		int resHandle = _resourceManager->queryUnloadedResource(0);
		char *dataBuf = 0;
		int bufSize = 0;

		while (resHandle != 0)
		{
			std::ifstream inf;

			//遍历全局路径找到要打开的文件
			for (unsigned int i = 0; i < dirs.size(); ++i)
			{
				std::string fileName = dirs[i] + _resourcePaths[getResourceType(resHandle)]
					+ "/" + getResourceName(resHandle);
				inf.clear();
				inf.open(fileName.c_str(), std::ios::binary);
				if (inf.good()) break;
			}

			//打开资源文件
			if (inf.good())
			{
				//获取文件大小
				inf.seekg(0, std::ios::end);
				int fileSize = inf.tellg();
				if (bufSize < fileSize)
				{
					delete[] dataBuf;
					dataBuf = new char[fileSize+1];
					if (!dataBuf)
					{
						bufSize = 0;
						continue;
					}
					bufSize = fileSize;
				}
				if (fileSize == 0) continue;
				//复制文件数据输入流到内存
				inf.seekg(0);
				inf.read(dataBuf, fileSize);
				inf.close();
				//调用资源的load函数
				result &= loadResource(resHandle, dataBuf, fileSize);
			}
			else
			{
				//若未找到文件，则传入空数据指针
				loadResource(resHandle, 0x0, 0);
				result = false;
			}
			//获取下一个未读取资源
			resHandle = _resourceManager->queryUnloadedResource(0);
		}
		delete[] dataBuf;

		return result;
	}
	
	//调用资源的load函数
	bool LeanRoot::loadResource(ResHandle handle, const char *data, int size)
	{
		Resource *res = _resourceManager->resolveResHandle(handle);
		if (res == 0x0)
		{
			LEAN_DEGUG_LOG("error load获取资源失败！", 0);
			ASSERT(0);
			return false;
		}
		
		LEAN_DEGUG_LOG("hint 读取资源：%d.", res->getName().c_str());
		
		return res->load(data, size);
	}

	int LeanRoot::getResourceType(ResHandle handle)
	{
		Resource *res = _resourceManager->resolveResHandle(handle);

		if (res == 0x0)
		{
			LEAN_DEGUG_LOG("error 获取资源类型失败！", 0);
			ASSERT(0);
			return false;
		}

		return res->getType();
	}

	const char * LeanRoot::getResourceName(ResHandle handle)
	{
		Resource *res = _resourceManager->resolveResHandle(handle);

		if (res == 0x0)
		{
			LEAN_DEGUG_LOG("error 获取资源Name失败！", 0);
			ASSERT(0);
			return false;
		}

		return res->getName().c_str();
	}

	bool LeanRoot::setMatrialUniform(ResHandle handle, const char *name, float a
									, float b, float c, float d)
	{
		MaterialResource* res = (MaterialResource*)_resourceManager->resolveResHandle(handle);
		if (res == 0x0)
		{
			LEAN_DEGUG_LOG("error 获取Material资源失败！", 0);
			return false;
		}
		return res->setUniform(name, a, b, c, d);

	}

	bool LeanRoot::setMatrialUniform(ResHandle handle, const char *name, float a[4])
	{
		MaterialResource* res = (MaterialResource*)_resourceManager->resolveResHandle(handle);
		if (res == 0x0)
		{
			LEAN_DEGUG_LOG("error 获取Material资源失败！", 0);
			return false;
		}
		return res->setUniform(name, a[0], a[1], a[2], a[3]);
	}

}