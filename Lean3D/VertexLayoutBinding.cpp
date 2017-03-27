#include "VertexLayoutBinding.h"


namespace Lean3D
{
	VertexLayoutBinding::VertexLayoutBinding()
	{
	}

	VertexLayoutBinding * VertexLayoutBinding::create(GeometryResource * geores, ShaderPass * pass)
	{
		ASSERT(geores);
		ASSERT(pass);

		VertexLayoutBinding *vlb = nullptr;

		for(int i=0; i < pass->attributes.size(); ++i)
		{
			if (pass->attributes[i]._name == "a_vertPos")
			{
				
			}
		}

		return nullptr;
	}

}

