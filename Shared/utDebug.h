/****************lean3D*******************/

#include "utPlatform.h"


#if defined( PLATFORM_WIN ) && defined( _MSC_VER ) && defined( _DEBUG )

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//内存检测
#define MYDEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
// 编译设置: /Zi /D_DEBUG /MLd

#define new MYDEBUG_NEW

#endif
