//#include "OGLDeviceManager.h"
//
//
//namespace Lean3D
//{
//
//	OGLDeviceManager::~OGLDeviceManager()
//	{
//
//	}
//	OGLDeviceManager::OGLDeviceManager()
//	{
//		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//像素对齐为1的倍数
//	}
//
//	bool OGLDeviceManager::init()
//	{
//		//此处应该检查本机对OPENGL的支持状态
//		//现在只是大概初始化下 
//		_caps.texFloat = true;
//		_caps.texNPOT = true;
//		_caps.rtMultisampling = true;
//		
//		//深度缓存格式  部分老ati显卡只支持16位深度缓存FBO  我将忽略这个
//		_depthFormat = GL_DEPTH_COMPONENT24; 
//		
//		initStates();
//
//	}
//
//
//
//}
//
//
