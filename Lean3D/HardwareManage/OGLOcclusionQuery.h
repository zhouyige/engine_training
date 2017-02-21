#pragma once
#include "LeanUtil.h"
#include "glus.h"

namespace Lean3D
{
	class OGLOcclusionQuery
	{
	public:
		uint32 createOcclusionQuery();
		void destroyQuery(uint32 queryObj);
		void beginQuery(uint32 queryObj);
		void endQuery(uint32 queryObj);
		uint32 getQueryResult(uint32 queryObj);
	};
}