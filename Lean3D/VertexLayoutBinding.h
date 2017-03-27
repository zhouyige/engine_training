#pragma once
#include "LeanUtil.h"
#include "LeanShaderRes.h"
#include "LeanGeometryRes.h"

namespace Lean3D
{
	class VertexLayoutBinding
	{
	public:
		VertexLayoutBinding();

		static VertexLayoutBinding* create(GeometryResource *geores, ShaderPass *pass);
	};

}


